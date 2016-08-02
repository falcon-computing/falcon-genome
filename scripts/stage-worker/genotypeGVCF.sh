#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

stage_name=genotypeGVCF
print_help() {
  echo "USAGE:"
  echo "fcs-genome gt|genotypeGVCF \\";
  echo "    -r <ref.fasta> \\"
  echo "    -i <input_dir> \\"
  echo "    -o <output.gvcf> \\"
}

if [[ $# -lt 2 ]]; then
  print_help 
  exit 1
fi

while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
  -r|--ref)
    ref_fasta="$2"
    shift
    ;;
  -i|--input_dir)
    input_dir="$2"
    shift # past argument
    ;;
  -o|--output)
    output="$2"
    shift # past argument
    ;;
  -v|--verbose)
    if [ $2 -eq $2 2> /dev/null ]; then
      # user specified an integer as input
      verbose="$2"
      shift
    else
      verbose=2
      shift
    fi
    ;;
  *) # unknown option
    log_error "failed to parse argument $key"
    ;;
  esac
  shift # past argument or value
done

# Check the args 
check_arg "-i" "input_dir"
check_arg "-o" "output"
check_args
check_arg "-r" "ref_fasta" "$ref_genome"

# Get absolute file path
readlink_check ref_fasta
readlink_check input_dir
readlink_check output
readlink_check log_dir

check_output $output
create_dir $log_dir
check_output_dir $log_dir

# Get a sample list from input dir
#IFS=$'\r\n' GLOBIGNORE='*' command eval 'sample_list=($(ls $input_dir | grep "gvcf"))' &>/dev/null
declare -A sample_list
part_index=1

while true; do
  if [ -f $input_dir/part-${part_index}.gvcf ];then
    sample_list["$part_index"]=part-${part_index}.gvcf
    part_index=$[$part_index+1]
  else
    part_index=$[$part_index-1]
    break
  fi
done 

# check if there are inputs
if [ $part_index -eq 0 ];then
  log_error "No valid gvcf file found in $input_dir"
  exit 1
fi

start_ts=$(date +%s)

declare -A pid_table
start_manager
trap "terminate" 1 2 3 15

echo "Compressing for ${#sample_list[@]} gvcf" >$log_dir/genotype.log

# Start the compress jobs distributely
for sample in "${sample_list[@]}";do
  gvcf_file=$input_dir/$sample
  $DIR/../fcs-sh "$DIR/genotypeGVCF_part.sh $gvcf_file $ref_fasta" &>$log_dir/compress.${sample}.log & 
  pid_table["$sample"]=$! 
done

# Wait for the jobs to finish
is_error=0
output_vcf_list=
for index in $(seq 1 $part_index); do
  sample=${sample_list["$index"]}
  pid=${pid_table[$sample]}
  wait "${pid}"
  if [ "$?" -gt 0 ];then
    is_error=1
    log_error "Failed on sample $sample"
  fi
  # Concat logs
  sample_log=$log_dir/compress.${sample}.log
  cat $sample_log >> $log_dir/genotype.log
  rm -f $sample_log
  # Collect results
  output_vcf_list=$output_vcf_list"$input_dir/${sample}.genotype.vcf "
done

stop_manager
unset pid_table
unset sample_list

if [ "$is_error" -ne 0 ];then
  log_error "Stage failed, please check logs in $log_dir/genotype.log for details"
  exit 1
fi

end_ts=$(date +%s)
log_info "Genotype finishes in $((end_ts - start_ts))s"

# Concat the distributed vcf
$BCFTOOLS concat $output_vcf_list -o $output &>>$log_dir/genotype.log

if [ "$?" -gt 0 ];then
  log_error "Stage failed, please check logs in $log_dir/genotype.log for details"
  exit 1
fi

end_ts_total=$(date +%s)
log_info "Stage finishes in $((end_ts_total - start_ts))s"

