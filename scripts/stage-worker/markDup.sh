################################################################################
## This script generates mark duplicates of the sorted bam file
################################################################################
#!/bin/bash

# Import global variables
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

stage_name=markDup

# Prevent this script to be running alone
if [[ $0 != ${BASH_SOURCE[0]} ]]; then
  # Script is sourced by another shell
  cmd_name=`basename $0 2> /dev/null`
  if [[ "$cmd_name" != "fcs-genome" ]]; then
    log_error "This script should be started by 'fcs-genome'"
    return 1
  fi
else
  # Script is executed directly
  log_error "This script should be started by 'fcs-genome'"
  exit 1
fi

print_help() {
  echo "USAGE:"
  echo "fcs-genome md|markDup \\";
  echo "    -i <input.bam> \\ ";
  echo "    -o <output.bam>";
  echo 
  echo "<input.bam> argument is the sorted bam file";
  echo "<output.bam> argument is the markduped bam file";
}

if [ $# -lt 1 ]; then
  print_help
  exit 1;
fi

# Get the input command 
while [[ $# -gt 0 ]];do
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
    if [ $2 -eq $2 2> /dev/null ]; then
      # user specified an integer as input
      verbose="$2"
      shift
    else
      verbose=2
    fi
    ;;
  -f|--force)
    force_flag=YES
    ;;
  -h|--help)
    help_req=YES
    ;;
  *)
    # unknown option
    log_error "Failed to recongize argument '$1'"
    print_help
    exit 1
    ;;
  esac
  shift # past argument or value
done

# If no argument is given then print help message
if [ ! -z $help_req ];then
  print_help
  exit 0;
fi

# Check the input arguments
check_arg "-i" "input"
check_input $input

fastq_base_withsuffix=$(basename $input)
fastq_base=${fastq_base_withsuffix%.*} 
output_default=${tmp_dir[2]}/${fastq_base}.markdups.bam

check_arg "-o" "output" "$output_default"
check_args

# Get absolute filepath for input/output
readlink_check input
readlink_check output
readlink_check log_dir


# Check input
check_output $output
check_output $output.dup_stats

tmp_dir=${tmp_dir[1]}
create_dir $tmp_dir
check_output_dir $tmp_dir
log_info "The intermediate files of mark duplicate are stored to $tmp_dir as default"

#Create log dir
create_dir $log_dir

log_info "Start stage for input $input"

start_ts=$(date +%s)

# Put all the information to log, not displaying
$JAVA -XX:+UseSerialGC -Xmx160g -jar $PICARD \
    MarkDuplicates \
    TMP_DIR=$tmp_dir COMPRESSION_LEVEL=1 \
    INPUT=$input \
    OUTPUT=$output \
    METRICS_FILE=${output}.dups_stats \
    REMOVE_DUPLICATES=false ASSUME_SORTED=true VALIDATION_STRINGENCY=SILENT \
    &> $log_dir/markDup.log

if [ "$?" -ne 0 ]; then 
  log_error "Stage failed, please check $log_dir/markDup.log for detailed information"
  exit 1
fi

end_ts=$(date +%s)
log_info "Stage finished in $((end_ts - start_ts))s"
