################################################################################
## This script does the printread stage
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
    -i|--input_base)
    input_base="$2"
    shift # past argument
    ;;
    -c|--chr_dir)
    chr_dir="$2"
    shift # past argument
    ;;
    -r|--rpt_dir)
    rpt_dir="$2"
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
  echo " USAGE: fcs_genome printread -i <input_base> -c <chr_dir> \
-r <rpt_dir> -o <output_dir> -v <verbose> -clean <0|1>"
  echo " The <input_base> argument is the basename of the input bams, shoud not contain suffixes"
  echo " The <chr_dir> argument is the directory to find the input bams"
  echo " The <rpt_dir> argument is the directory to find the reports of bqsr"
  echo " The <verbose> argument is the verbose level of the run, verbose=0 means quiet \
and no output, verbose=1 means output errors, verbose=2 means detailed information. By default it is set to 1"
  echo " The clean option tells if the program should keep the input , if set to 1, we would \
remove the input, by default it's set to 0 "
  exit 1;
fi

if [ -z $input_base ];then
  echo "input_base is not specified, please check the command"
  echo " USAGE: fcs_genome printread -i <input_base> -c <chr_dir> -r <rpt_dir> -o <output_dir> -v <verbose>"
  exit 1
fi

if [ -z $chr_dir ];then
  echo "chr_dir is not specified, please check the command"
  echo " USAGE: fcs_genome printread -i <input_base> -c <chr_dir> -r <rpt_dir> -o <output_dir> -v <verbose>"
  exit 1
fi

if [ -z $rpt_dir ];then
  echo "rpt_dir is not set, please check the command"
  echo " USAGE: fcs_genome printread -i <input_base> -c <chr_dir> -r <rpt_dir> -o <output_dir> -v <verbose>"
fi

if [ -z $verbose ];then
  verbose=1;
  echo "In default verbose is 1"
  echo "If you want to set it, use the -v <verbose> option "
fi

if [ -z $chean_flag ];then
  clean_flag=0;
  echo "In default clean_flag is 0, the input would be kept"
  echo "If you want to set it, use the -clean option "
fi

if [ -z $output ];then
  create_dir ${tmp_dir[2]}
  output=${tmp_dir[2]}
  echo "Output file name is not set, the output file is stored to "$output" as default"
fi

# Check the inputs

chr_list="$(seq 1 22) X Y MT"
for chr in $chr_list; do
  chr_bam=$chr_dir/${input_base}.bam.markdups.chr${chr}.bam
  if [ ! -f $chr_bam ];then 
    >&2 echo "Could not find the $chr_bam"
    exit 1
  fi
done

# Check the outputs
chr_recal_bam=${tmp_dir[2]}/${input_base}.bam.recal.chr1.bam
check_output $chr_recal_bam
rm ${tmp_dir[2]}/${input_base}.bam.recal.*.bam.done

# Create the directorys
pr_log_dir=$log_dir/pr_log_dir
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

echo "manager started"
# Start the jobs
for chr in $chr_list; do
    # The splited bams are in tmp_dir[1], the recalibrated bams should be in [2]
    chr_bam=${tmp_dir[1]}/${input_base}.bam.markdups.chr${chr}.bam
    chr_rpt=$rpt_dir/${input_base}.bam.markdups.bam.recalibration_report.grp
    chr_recal_bam=${tmp_dir[2]}/${input_base}.bam.recal.chr${chr}.bam
    # TODO(yaoh) add the verbose option case here
    $DIR/fcs-sh "$DIR/printReads.sh $chr_bam $chr_rpt $chr_recal_bam $chr" 2> $log_dir/printReads_chr${chr}.log &
    pid_table["$chr"]=$!
done
# Wait on all the tasks
for pid in ${pid_table[@]}; do
  wait "${pid}"
done

for chr in $chr_list; do
  chr_bam=${tmp_dir[1]}/${input_base}.bam.markdups.chr${chr}.bam
  chr_recal_bam=${tmp_dir[2]}/${input_base}.bam.recal.chr${chr}.bam
  if [ ! -e ${chr_recal_bam}.done ]; then
    exit 4;
  fi
  if [ $clean_flag == 1 ]; then
   rm ${chr_recal_bam}.done
   rm $chr_bam &
   rm ${chr_bam}.bai &
  fi
done

# Stop manager
kill $manager_pid
end_ts=$(date +%s)

echo "PrintReads stage finishes in $((end_ts - start_ts))s"


