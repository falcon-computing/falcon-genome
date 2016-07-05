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
    -t|--temp_dir)
    tmp_dir="$2"
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
  echo  " USAGE: fcs_genome markdup -i <input_bam> -o <output_bam>"
  echo  " The <input_bam> argument is necessary for the script to run, should contain the full name"
  echo  " The <output> argument is the file to store the marked bam, could be empty."
  echo  " The <temp_dir> argument is the directory to store the intermediate results, could be empty."
  exit 1;
fi

if [ -z $input ];then
  echo " The <input_bam> argument is necessary for the script to run, should contain the full name"
  echo " You should use -i <input_bam> to specify the input bam file"
  exit 1;
fi 

if [ -z $output ];then
  output=${tmp_dir[2]}/$(basename $input).markdups.bam
  echo "Output file is not set, the output file is stored to "$output" as default"
  echo "If you want to set it, use the -o <output> option "
fi

if [ -z $tmp_dir ];then
  create_dir ${tmp_dir[1]}
  tmp_dir=${tmp_dir[1]}
  echo "Temp directory is not set, the intermediate files are stored to $tmp_dir as default"
  echo "If you want to set it, use the -t <temp_dir> option"
fi


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
$JAVA -XX:+UseSerialGC -Xmx160g -jar $PICARD \
    MarkDuplicates \
    TMP_DIR=$tmp_dir COMPRESSION_LEVEL=1 \
    INPUT=$input \
    OUTPUT=$output \
    METRICS_FILE=${output}.dups_stats \
    REMOVE_DUPLICATES=false ASSUME_SORTED=true VALIDATION_STRINGENCY=SILENT \
    2>$markdup_log_dir/markdup_run.log

if [ "$?" -ne 0 ]; then 
  echo "Mark duplicates failed,please check $markdup_log_dir/markdup_run.log for detailed information"
  exit 1
fi

end_ts=$(date +%s)
echo "Mark duplicates finished in $((end_ts - start_ts))s"

