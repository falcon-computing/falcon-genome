#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

stage_name=concatVCF

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
  echo "USAGE: fcs-genome concat|concatGVCF -i <input_dir> -o <output>"
}

if [[ $# -lt 2 ]]; then
  print_help 
  exit 1
fi

while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
  -i|--input_dir)
    input_vcf_dir="$2"
    shift # past argument
    ;;
  -o|--output_dir)
    output_dir="$2"
    shift # past argument
    ;;
  -v|--verbose)
    verbose=2
    ;;
  *) # unknown option
    log_error "failed to parse argument $key"
    ;;
  esac
  shift # past argument or value
done

check_arg "-i" "input_vcf_dir"
check_arg "-o" "output_dir" $(pwd)
check_args

# Get absolute file path
readlink_check input_vcf_dir
readlink_check output_dir
readlink_check log_dir

create_dir $output_dir
create_dir $log_dir
check_output_dir $output_dir
check_output_dir $log_dir

get_sample_id() {
  local input_dir=$1;
  local sample_id=`ls $input_dir/*.gvcf | sed -e 'N;s/^\(.*\).*\n\1.*$/\1\n\1/;D'`;
  local sample_id=$(basename $sample_id);
  #local sample_id=${sample_id::-4};
  local sample_id=${sample_id%%.*}
  echo $sample_id;
}

kill_task_pid() {
  log_info "kill $task_pid"
  kill $task_pid 2> /dev/null
  exit 1
}

trap "kill_task_pid" 1 2 3 9 15

vcf_sample_id=`get_sample_id $input_vcf_dir`

input_vcf_list=
chr_list="$(seq 1 22) X Y MT";
for chr in $chr_list; do
  input_vcf_list="$input_vcf_list ${input_vcf_dir}/${vcf_sample_id}.chr${chr}.gvcf"
done

$BCFTOOLS concat $input_vcf_list -o $output_dir/${vcf_sample_id}.gvcf &>$log_dir/concat.log &
task_pid=$!
wait "$task_pid"
if [ "$?" -ne "0" ]; then
  log_error "bcftools concat failed"
  exit 1;
fi

$BGZIP -c $output_dir/${vcf_sample_id}.gvcf > $output_dir/${vcf_sample_id}.gvcf.gz &>>$log_dir/concat.log &
task_pid=$!
wait "$task_pid"
if [ "$?" -ne "0" ]; then
  log_error "bgzip compression failed"
  exit 1;
fi

# delete uncompressed gvcf file
rm $output_dir/${vcf_sample_id}.gvcf

$TABIX -p vcf $output_dir/${vcf_sample_id}.gvcf.gz &>>$log_dir/concat.log &
task_pid=$!
wait "$task_pid"
if [ "$?" -ne "0" ]; then
  log_error "tabix failed"
  exit 1;
fi 

