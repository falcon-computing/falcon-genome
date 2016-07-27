#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh
source $DIR/stage-worker/common.sh

stage_name=concatVCF
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
  -i|--input_dir)
    input_dir="$2"
    shift # past argument
    ;;
  -o|--output)
    output="$2"
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

# Check the input 
chr_list="$(seq 1 22) X Y MT";
for chr in $chr_list; do
  if [ -z "`find $input_dir -name *.chr${chr}.gvcf`" ]; then
    log_error "ERROR: cannot find VCF for chr$chr";
    exit 1;
  fi;
done;

#check_output $output

# Infer the basename
output_base=${output%%.*}

# Start concating the vcf files

$BCFTOOLS concat $input_dir/*.chr*.gvcf -o ${output_base}.gvcf
$BGZIP -c ${output_base}.gvcf > $output
$TABIX -p vcf $output
