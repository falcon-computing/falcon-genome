################################################################################
## This script generates mark duplicates of the sorted bam file
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
    -i|--input)
    input="$2"
    shift # past argument
    ;;
    -o|--output)
    output="$2"
    shift
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
  echo  " USAGE: fcs_genome markdup -i <input.bam> -o <output.bam>"
  echo  " The <input.bam> argument is necessary for the script to run, should contain the full name"
  echo  " The <output.bam> argument is the file to store the markduped bam, could be empty."
  echo  " The <verbose> argument is the verbose level of the run, verbose=0 means quiet \
and no output, verbose=1 means output errors, verbose=2 means detailed information. By default it is set to 1"
  exit 1;
fi

if [ -z $input ];then
  echo " The input bam is not specified, please check the command"
  echo " You should use -i <input.bam> to specify the input bam file"
  exit 1;
fi 

if [ -z $output ];then
  output=${tmp_dir[2]}/$(basename $input).markdups.bam
  echo "Output file is not set, the output file is stored to "$output" as default"
  echo "If you want to set it, use the -o option "
fi

if [ -z $verbose ];then
  verbose=1;
  echo "In default verbose is 1"
  echo "If you want to set it, use the -v <verbose> option "
fi

create_dir ${tmp_dir[1]}
tmp_dir=${tmp_dir[1]}
echo "The intermediate files of mark duplicate are stored to $tmp_dir as default"

# Find the input bam

check_input $input
check_output $output
check_output ${output}.dup_stats
check_output_dir $tmp_dir

markdup_log_dir=$log_dir/markdup
create_dir $log_dir
create_dir $markdup_log_dir

echo "Start mark duplicates"
start_ts=$(date +%s)

case $verbose in
    0)
    # Put all the information to log, not displaying
    $JAVA -XX:+UseSerialGC -Xmx160g -jar $PICARD \
    MarkDuplicates \
    TMP_DIR=$tmp_dir COMPRESSION_LEVEL=1 \
    INPUT=$input \
    OUTPUT=$output \
    METRICS_FILE=${output}.dups_stats \
    REMOVE_DUPLICATES=false ASSUME_SORTED=true VALIDATION_STRINGENCY=SILENT \
    >$markdup_log_dir/markdup_run_err.log 2> $markdup_log_dir/markdup_run.log
    ;;

    1)
    # Put stdout to log, stderr to log and display
    $JAVA -XX:+UseSerialGC -Xmx160g -jar $PICARD \
    MarkDuplicates \
    TMP_DIR=$tmp_dir COMPRESSION_LEVEL=1 \
    INPUT=$input \
    OUTPUT=$output \
    METRICS_FILE=${output}.dups_stats \
    REMOVE_DUPLICATES=false ASSUME_SORTED=true VALIDATION_STRINGENCY=SILENT \
    > >(tee $markdup_log_dir/markdup_run.log) 2> >(tee $markdup_log_dir/markdup_run_err.log >&2)
    ;;

    2)
    # Put all the information to log and display
    $JAVA -XX:+UseSerialGC -Xmx160g -jar $PICARD \
    MarkDuplicates \
    TMP_DIR=$tmp_dir COMPRESSION_LEVEL=1 \
    INPUT=$input \
    OUTPUT=$output \
    METRICS_FILE=${output}.dups_stats \
    REMOVE_DUPLICATES=false ASSUME_SORTED=true VALIDATION_STRINGENCY=SILENT \
    > >(tee $markdup_log_dir/markdup_run.log) 2> >(tee $markdup_log_dir/markdup_run_err.log >&2)
    ;;
esac

if [ "$?" -ne 0 ]; then 
  >&2 echo "Mark duplicates failed, please check $markdup_log_dir/markdup_run_err.log for detailed information"
  exit 1
fi

end_ts=$(date +%s)
echo "Mark duplicates finished in $((end_ts - start_ts))s"
echo "The results can be found at $output "
echo "Log can be found at $markdup_log_dir"
