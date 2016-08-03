#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

stage_name=compressVCF

input_gvcf_file=$1
ref_fasta=$2
output_vcf=${input_gvcf_file}.genotype.vcf

$BGZIP -c $input_gvcf_file > ${input_gvcf_file}.gz
$TABIX -p vcf ${input_gvcf_file}.gz

$JAVA -d64 -Xmx4g -jar $GATK \
      -T GenotypeGVCFs \
      -R $ref_fasta \
      --variant ${input_gvcf_file}.gz \
      -o $output_vcf 
