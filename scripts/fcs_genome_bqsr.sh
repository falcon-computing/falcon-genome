################################################################################
## This script generates sorted bam file from fastq using bwa-flow
################################################################################
#!/bin/bash

# Import global variables
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

declare -A knownSites
ks_index=0
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -r|--ref)
    ref_fasta="$2"
    shift # past argument
    ;;
    -i|--input)
    input="$2"
    shift # past argument
    ;;
    -ks|-knownSites)
    knownSites["$ks_index"]="$2"
    echo "knownSites["$ks_index"] is ${knownSites[$ks_index]}"
    ks_index=$[$ks_index+1]
    shift # past argument
    ;;
    -o|--output)
    output_rpt="$2"
    shift # past argument
    ;;
    -v|--verbose)
    verbose="$2"
    shift
    ;;
    -f|--force)
    force_flag=YES
    ;;
    -h|--help)
    help_req=YES
    ;;
    *)
            # unknown option
    ;;
esac
shift # past argument or value
done

# Check the command
if [ ! -z $help_req ];then
  echo " USAGE: fcs_genome bqsr -r <ref.fasta> -i <input> -knownSites <xx.vcf> -o <output.rpt> " 
  echo " The input argument is the markduped bam file"
  echo " The output_dir argument specifies the directory where the output pf bqsr should be stored"
  echo  " The <verbose> argument is the verbose level of the run, verbose=0 means quiet \
and no output, verbose=1 means output errors, verbose=2 means detailed information. By default it is set to 1"
  exit 1;
fi

if [ -z $ref_fasta ];then
  ref_fasta=$ref_genome
  echo "The reference fasta is not specified, by default we would use $ref_fasta"
  echo "You should use -r <ref.fasta> to specify the reference"
fi

if [ -z $input ];then
  echo "The input bam is not specified, please check the command"
  echo "You should use -i <input.bam> to specify the markduped bam file"
  exit 1;
fi

if [ -z ${knownSites[0]} ];then
  knownSites_string="-knownSites $g1000_indels -knownSites $g1000_gold_standard_indels -knownSites $db138_SNPs "
  echo "The knownSites are not specified, by default $g1000_indels $g1000_gold_standard_indels \
and $db138_SNPs is used"
  echo "If you want to specify them, please use the -ks|-knownSites option"
fi

if [ -z $output_rpt ];then
  rpt_dir=$output_dir/rpt
  create_dir $rpt_dir
  output_rpt=$rpt_dir/$(basename $input).recalibration_report.grp
  echo "The output rpt is not specified, by default the result is put to $output_rpt"
fi

if [ -z $verbose ];then
  verbose=1;
  echo "In default verbose is 1"
  echo "If you want to set it, use the -v <verbose> option "
fi

for ks in ${knownSites[@]}; do
   knownSites_string="$knownSites_string -knownSites $ks"  
done
echo "$knownSites_string"

bqsr_log_dir=$log_dir/bqsr
bqsr_manager_dir=$log_dir/bqsr/manager
create_dir $log_dir
create_dir $bqsr_log_dir
create_dir $bqsr_manager_dir

check_input $input
if [ ! -z $force_flag ];then
   echo "Force option is used"
   check_output_force $output_rpt
  else
   check_output $output_rpt
fi

#Clear the done files to recover
rm $rpt_dir/.${output_rpt}*> /dev/null 
rm $rpt_dir/${output_rpt}.*> /dev/null 
rm .queue -rf> /dev/null

# Table storing all the pids for tasks within one stage
declare -A pid_table

# Start the index option
echo "Start the samtools index"
start_ts=$(date +%s)
$SAMTOOLS index $input 
end_ts=$(date +%s)
echo "Samtools index for $(basename $input) finishes in $((end_ts - start_ts))s"
echo "The output file is ${input}.bai"

# Start manager
start_ts=$(date +%s)

echo "Starting manager to post jobs"
if [ -e "host_file" ]; then
  host_file=host_file
else
  host_file=$DIR/host_file
fi
echo "$DIR/manager/manager --v=1 --log_dir=$bqsr_manager_dir $host_file"
$DIR/manager/manager --v=1 --log_dir=$bqsr_manager_dir $host_file &
manager_pid=$!
sleep 1
if [[ ! $(ps -p "$manager_pid" -o comm=) =~ "manager" ]]; then
  >&2 echo "Cannot start manager, exiting"
  exit -1
fi

echo "manager started"
#kill_process() {
#  echo "Reiceived interrupt, killing the manager process"
#  local process_pid=$1
#  kill $process_pid
#  exit -1
#}
#trap "kill_process $manager_pid" 1 2 3 15

export PATH=$DIR:$PATH


for ks in ${knownSites[@]}; do
   knownSites_string="$knownSites_string -knownSites $ks"  
done
echo "$knownSites_string"

echo "Starting bqsr"
case $verbose in
    0|1|2)
    # Put all the information to log, not displaying
    $JAVA -Djava.io.tmpdir=/tmp -jar ${GATK_QUEUE} \
    -S $DIR/BaseRecalQueue.scala \
    -R $ref_fasta \
    -I $input \
    $knownSites_string \
    -o $output_rpt \
    -jobRunner ParallelShell \
    -maxConcurrentRun 32 \
    -scatterCount 32 \
    -run
    > $bqsr_log_dir/bqsr_run.log 2> $bqsr_log_dir/bqsr_run_err.log 
    ;;
   # TODO(yaoh) add the remain verbose options after figure out how the bqsr print information 
esac
# Stop manager
kill $manager_pid

end_ts=$(date +%s);
echo "bqsr for $(basename $output_rpt) finishes in $((end_ts - start_ts))s"
echo "The output could be found at $output_rpt"
