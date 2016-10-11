#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

stage_name=compressVCF

input_gvcf_file=$1

$BGZIP -c $input_gvcf_file > ${input_gvcf_file}.gz
$TABIX -p vcf ${input_gvcf_file}.gz
