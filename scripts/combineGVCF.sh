#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh
source $DIR/stage-worker/common.sh

print_help() {
  echo "USAGE: $0 -i <input_dir> -o <output_dir>"
}

if [[ $# -lt 2 ]]; then
  print_help 
  exit 1
fi

while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
  -i|--input_dir)
    input_dir="$2"
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

check_arg "-i" "input_dir"
check_arg "-o" "output_dir"
check_args

check_output_dir $output_dir

check_input_chr() {
  local input_dir=$1;
  local chr_list="$(seq 1 22) X Y MT";
  for chr in $chr_list; do
    if [ -z "`find $input_dir -name *_chr${chr}.gvcf`" ]; then
      log_error "ERROR: cannot find VCF for chr$chr";
      return 1;
    fi;
  done;
}

gatk_combine_gvcf() {
  local input="$1";
  local output="$2";
  
  log_info "$JAVA -d64 -Xmx4g -jar $GATK \
      -T CombineGVCF \
      -R $ref_genome \
      $input \
      -o $output_file";

  $JAVA -d64 -Xmx4g -jar $GATK \
      -T CombineGVCF \
      -R $ref_genome \
      $input \
      -o $output_file 2> /dev/null;
}

# Get a sample list from input dir
IFS=$'\r\n' GLOBIGNORE='*' command eval 'sample_list=($(ls $input_dir))'

input_samples=()
for sample_id in "${sample_list[@]}"; do
  check_input_chr "$input_dir/$sample_id/vcf"
  if [ "$?" -eq 0 ]; then
    input_samples+=("$sample_id")
  fi
done

if [ ${#input_samples[@]} -lt 1 ]; then
  log_error "Cannot find valid input files"
  exit 1
fi

log_info "Start combining ${#input_samples[@]} samples"

# Start combine gvcf using GATK
chr_list="$(seq 1 22) X Y MT"
for chr in $chr_list; do
  input_gvcfs=
  output_gvcf=$output_dir/combined.chr${chr}.gvcf
  for sample in "${input_samples[@]}"; do
    gvcf_file=$input_dir/$sample/vcf/${sample}_chr${chr}.gvcf
    input_gvcfs=$input_gvcfs"--variant $gvcf_file " 
  done

  log_info "$JAVA -d64 -Xmx4g -jar $GATK \
      -T CombineGVCFs \
      -R $ref_genome \
      $input_gvcfs \
      -o $output_gvcf";

  $JAVA -d64 -Xmx4g -jar $GATK \
      -T CombineGVCFs \
      -R $ref_genome \
      $input \
      $input_gvcfs \
      -o $output_gvcf 2> combineGVCF_chr${chr}.log;
  
  if [ "$?" -ne 0 ]; then
    log_error "CombineVCF fails for chr$chr"
  fi
done
