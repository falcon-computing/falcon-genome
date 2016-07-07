################################################################################
## This script generates sorted bam file from fastq using bwa-flow
################################################################################
#!/bin/bash

# Import global variables
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -i|--input)
    input="$2"
    shift # past argument
    ;;
    -o|--output)
    rpt_dir="$2"
    shift # past argument
    ;;
    -v|--verbose)
    verbose="$2"
    shift
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
  echo " USAGE: fcs_genome bqsr -i <input> -o <output_dir> -v <verbose>" 
  echo " The input argument is the markduped bam file"
  echo " The output_dir argument specifies the directory where the output pf bqsr should be stored"
  echo  " The <verbose> argument is the verbose level of the run, verbose=0 means quiet \
and no output, verbose=1 means output errors, verbose=2 means detailed information. By default it is set to 1"
  exit 1;
fi

if [ -z $input ];then
  echo "The input arg is missing, please check the cmd"
  exit 1;
fi

if [ -z $rpt_dir ];then
  rpt_dir=$output_dir/rpt
  create_dir $rpt_dir
  echo "The output dir is not specified, would put the result to $rpt_dir"
fi

if [ -z $verbose ];then
  verbose=1;
  echo "In default verbose is 1"
  echo "If you want to set it, use the -v <verbose> option "
fi

output=$rpt_dir/$(basename $input).recalibration_report.grp
bqsr_log_dir=$log_dir/bqsr
bqsr_manager_dir=$log_dir/bqsr/manager
create_dir $log_dir
create_dir $bqsr_log_dir
create_dir $bqsr_manager_dir

check_input $input
check_output $output

#Clear the done files to recover
rm $rpt_dir/.*.done> /dev/null 

# Table storing all the pids for tasks within one stage
declare -A pid_table

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
export PATH=$DIR:$PATH

echo "Starting bqsr"
case $verbose in
    0)
    # Put all the information to log, not displaying
    $JAVA -Djava.io.tmpdir=/tmp -jar ${GATK_QUEUE} \
    -S $DIR/BaseRecalQueue.scala \
    -R $ref_genome \
    -I $input \
    -knownSites $g1000_indels \
    -knownSites $g1000_gold_standard_indels \
    -knownSites $db138_SNPs \
    -o $output \
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
echo "Samtools sort for $(basename $output) finishes in $((end_ts - start_ts))s"
