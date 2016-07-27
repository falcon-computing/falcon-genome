#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh
source $DIR/stage-worker/common.sh

stage_name=genotypeGVCF
print_help() {
  echo "USAGE: $0 -i <input_dir> -o output"
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
IFS=$'\r\n' GLOBIGNORE='*' command eval 'sample_list=($(ls $input_dir | grep "gvcf"))' &>/dev/null
input_gvcfs=
for sample in "${sample_list[@]}";do
  gvcf_file=$input_dir/$sample
  $BGZIP -c $gvcf_file > ${gvcf_file}.gz
  $TABIX -p vcf ${gvcf_file}.gz
  input_gvcfs=$input_gvcfs"--variant ${gvcf_file}.gz "  
done

# Start to genotype the gvcf using GATK
start_ts=$(date +%s)

log_info "$JAVA -d64 -Xmx4g -jar $GATK \
      -T GenotypeGVCFs \
      -R $ref_fasta \
      $input_gvcfs \
      -nt 12 \
      -o $output_gvcf";

$JAVA -d64 -Xmx4g -jar $GATK \
      -T GenotypeGVCFs \
      -R $ref_fasta \
      $input_gvcfs \
      -nt 12 \
      -o $output &> $log_dir/genotype.log

if [ "$?" -ne 0 ]; then 
  log_error "genotypeGVCF failed, please check $log_dir/genotype.log for details"
  exit 1
fi

end_ts=$(date +%s)
log_info "Stage finishes in $((end_ts - start_ts))s"

