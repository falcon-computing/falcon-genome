################################################################################
## This script does the printreads stage
################################################################################
#!/bin/bash

# Import global variables
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

# Get the input command
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
    -bqsr)
    bqsr_rpt="$2"
    shift # past argument
    ;;
    -o|--output)
    output="$2"
    shift # past argument
    ;;
    -v|--verbose)
    verbose="$2"
    shift
    ;;
    -clean)
    clean_flag="$2"
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
  echo " USAGE: fcs_genome printread -r <ref.fasta> -i <input.bam> \
-bqsr calibrate.rpt -o <output_dir> "
  echo " The <input_base> argument is the basename of the input bams, shoud not contain suffixes"
  echo " The <chr_dir> argument is the directory to find the input bams"
  echo " The <output_dir> argument is the directory to put the output results"
#  echo " The <verbose> argument is the verbose level of the run, verbose=0 means quiet \
#and no output, verbose=1 means output errors, verbose=2 means detailed information. By default it is set to 1"
#  echo " The clean option tells if the program should keep the input , if set to 1, we would \
#remove the input, by default it's set to 0 "
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

if [ -z $bqsr_rpt ];then
  echo "The report file of bqsr is not specified, please check the command"
  echo "You should use -bqsr <calibrate.rpt> to specify it"
  exit 1;
fi

if [ -z $verbose ];then
  verbose=1;
  echo "In default verbose is 1"
  echo "If you want to set it, use the -v <verbose> option "
fi

if [ -z $clean_flag ];then
  clean_flag=0;
  echo "In default clean_flag is 0, the input would be kept"
  echo "If you want to set it, use the -clean option "
fi

if [ -z $output ];then
  create_dir ${tmp_dir[1]}
  output=${tmp_dir[1]}
  echo "Output file name is not set, the output file is stored to "$output" as default"
fi

# Check the input
check_input $input
input_base_withsuffix=$(basename $input)
input_base=${input_base_withsuffix%%.*} 
chr_recal_bam=${output}/${input_base}.recal.chr1.bam
check_output $chr_recal_bam
rm ${output}/${input_base}.recal.*.bam.done>/dev/null

# Create the directorys
pr_log_dir=$log_dir/pr
pr_manager_dir=$pr_log_dir/manager
create_dir $log_dir
create_dir $pr_log_dir
create_dir $pr_manager_dir

# Start the manager
declare -A pid_table

# Start manager
start_ts=$(date +%s)

echo "Starting manager to post jobs"
if [ -e "host_file" ]; then
  host_file=host_file
else
  host_file=$DIR/host_file
fi
echo "$DIR/manager/manager --v=1 --log_dir=$pr_manager_dir/ $host_file"
$DIR/manager/manager --v=1 --log_dir=$pr_manager_dir/ $host_file &
manager_pid=$!
sleep 1
if [[ ! $(ps -p "$manager_pid" -o comm=) =~ "manager" ]]; then
  >&2 echo "Cannot start manager, exiting"
  exit -1
fi

# Catch the interrupt option to stop manager
terminate(){
  echo "Caught interruption, cleaning up"
  # Stop manager
  kill "$manager_pid"
  echo "Stopped manager"
  # Stop stray
  for pid in ${pid_table[@]}; do
    ps -p $pid>/dev/null
    if [ "$?" == 0 ];then 
      kill "${pid}"
      echo "kill $pid"
    fi
  done
  # Check run dir
  for file in ${pr_output_table[@]}; do
   if [ -e ${file}.java.pid ]; then
     pr_java_pid=$(cat ${file}.java.pid)
     kill "$pr_java_pid"
     rm ${file}.java.pid
     echo "Stopped remote java process $pr_java_pid for $file"
   fi
   if [ -e ${file}.pid ]; then
     pr_pid=$(cat ${file}.pid)
     kill "$pr_pid"
     rm ${file}.pid
     echo "Stopped remote printReads process $pr_pid for $file"
   fi
  done
  echo "Exiting"
  exit 1;
}

trap "terminate" 1 2 3 15

echo "manager started"
# Start the jobs
chr_list="$(seq 1 22) X Y MT"
for chr in $chr_list; do
    # The splited bams are in tmp_dir[1], the recalibrated bams should be in [2]
    chr_recal_bam=${output}/${input_base}.recal.chr${chr}.bam
    # TODO(yaoh) add the verbose option case here
    $DIR/fcs-sh "$DIR/printReads.sh $input $bqsr_rpt $chr_recal_bam $chr $ref_fasta " 2> $pr_log_dir/printReads_chr${chr}.log &
    pid_table["$chr"]=$!
    pr_output_table["$chr"]=$chr_recal_bam
done
# Wait on all the tasks
for pid in ${pid_table[@]}; do
  wait "${pid}"
done
echo "all tasks finished"
unset pr_output_table

for chr in $chr_list; do
  chr_recal_bam=${output}/${input_base}.recal.chr${chr}.bam
  if [ ! -e ${chr_recal_bam}.done ]; then
    exit 4;
  fi
  if [ $clean_flag == 1 ]; then
   rm ${chr_recal_bam}.done
   rm $input &
   rm ${input}.bai &
  fi
done

# Stop manager
kill $manager_pid
end_ts=$(date +%s)

echo "PrintReads stage finishes in $((end_ts - start_ts))s"
echo "Please find the results in $output dir"

