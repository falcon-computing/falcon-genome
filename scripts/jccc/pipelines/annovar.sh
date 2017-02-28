#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../common.sh
source $DIR/../settings.sh

samtools_vcfcall() {
  local input=$1;
  samtools mpileup \
    -E -uf \
    $ref_genome \
    $input -P Illumina \
    -Q 30 2>/dev/null | bcftools call \
    -cv -Oz -o ${input}.vcf.gz 2>/dev/null ;
}

sample_id=$1
dest_dir=$2

mkdir -p $log_dir
log_file=$log_dir/annovar-${sample_id}.log

# Check if samtools and bcftools are available
which samtools &> /dev/null
if [ "$?" -gt 0 ]; then
  log_error "cannot find 'samtools' in PATH"
  exit -1
fi
which bcftools &> /dev/null
if [ "$?" -gt 0 ]; then
  log_error "cannot find 'bcftools' in PATH"
  exit -1
fi

start_ts=$(date +%s)

declare -A pid_table
for bam in `ls ${tmp_dir}/${sample_id}.recal.bam/*.bam`; do
  samtools_vcfcall "$bam" &
  pid_table["$bam"]=$!
done

for bam in `ls ${tmp_dir}/${sample_id}.recal.bam/*.bam`; do
  pid=${pid_table[$bam]}
  wait "${pid}"
  tabix -p vcf -f ${bam}.vcf.gz
done

bcftools concat -Ov -o \
  $dest_dir/${sample_id}.vcf \
  `ls ${tmp_dir}/${sample_id}.recal.bam/*.vcf.gz`

if [ $? -ne 0 ]; then
  log_error "bcftools concat for ${sample_id}.vcf failed"
  exit -1
fi

$annovar_dir/convert2annovar.pl \
  -format vcf4 \
  $dest_dir/${sample_id}.vcf > $dest_dir/${sample_id}.vcf.avinput \
  2>> $log_file

if [ $? -ne 0 ]; then
  log_error "convert2annovar for ${sample_id}.vcf failed"
  exit -1
fi

$annovar_dir/annotate_variation.pl \
  -buildver hg19 \
  $dest_dir/${sample_id}.vcf.avinput \
  $humandb_dir 2>> $log_file

if [ $? -ne 0 ]; then
  log_error "annotate_variation for ${sample_id}.vcf failed"
  exit -1
fi

end_ts=$(date +%s)
echo "annovar finishes in $((end_ts - start_ts)) seconds"
