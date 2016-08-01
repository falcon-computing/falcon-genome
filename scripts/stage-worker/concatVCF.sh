#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

stage_name=concatVCF

input_vcf_dir=$1
output_vcf=$2
output_base=${output_vcf%%.*}
# infer the basename, TODO:find a better way to get the basename
vcf_sample_id=`ls $input_vcf_dir/*.chr*.gvcf | sed -e 'N;s/^\(.*\).*\n\1.*$/\1\n\1/;D'`;
vcf_sample_id=${vcf_sample_id%%.*}
input_vcf_list=
chr_list="$(seq 1 22) X Y MT";

for chr in $chr_list; do
  input_vcf_list="$input_vcf_list ${vcf_sample_id}.chr${chr}.gvcf"
done

$BCFTOOLS concat $input_vcf_list -o ${vcf_sample_id}.gvcf
$BGZIP -c ${vcf_sample_id}.gvcf > $output_vcf
$TABIX -p vcf $output_vcf
