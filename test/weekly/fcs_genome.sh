fcs_genome_align () {
  local sample_id=$1
  local log_file=$2

  local fastq1="${db_dir}/${sample}_1.fastq"
  local fastq2="${db_dir}/${sample}_2.fastq"
  if [ ! -f $fastq_1 ] && [ ! -f $fastq_2 ]; then
    log_error "Cannot find input fastq of ${sample}"
    return 1
  fi
  local read_group=${sample_id}
  local library=${sample_id}

  fcs-genome align \
    -1 $fastq1 \
    -2 $fastq2 \
    -o $tmp_dir/$sample_id \
    -r $ref_genome \
    --align-only \
    -S $sample_id \
    -R $read_group \
    -L $library \
    -P Illumina \
    -f 2> $log_file

  if [ $? -ne 0 ]; then
    log_error "alignment failed for ${sample_id}"
    return 1
  fi

  return 0
}

fcs_genome_markdup () {
  local sample_id=$1
  local log_file=$2
  fcs-genome markDup \
    -i ${tmp_dir}/${sample_id} \
    -o ${tmp_dir}/${sample_id}.bam \
    -f 2> $log_file
  if [ $? -ne 0 ]; then
    log_error "markdup failed for ${sample_id}"
    return 1
  fi

  return 0
}

fcs_genome_ir () {
  local sample_id=$1
  local log_file=$2
  fcs-genome ir \
    -r $ref_genome \
    -i ${tmp_dir}/${sample_id}.bam \
    -o ${tmp_dir}/${sample_id}.realign.bam \
    -K $g1000_indels \
    -K $g1000_gold_standard_indels \
    -f 2> $log_file

  if [ $? -ne 0 ]; then
    log_error "ir failed for ${sample_id}"
    return 1
  fi

  return 0
}

fcs_genome_bqsr () {
  local sample_id=$1
  local log_file=$2
  fcs-genome bqsr \
    -r $ref_genome \
    -i ${tmp_dir}/${sample_id}.bam \
    -o ${tmp_dir}/${sample_id}.recal.bam \
    -K $db138_SNPs \
    -K $g1000_indels \
    -K $g1000_gold_standard_indels \
    -f 2> $log_file

  if [ $? -ne 0 ]; then
    log_error "bqsr failed for ${sample_id}"
    return 1
  fi

  return 0
}

fcs_genome_htc () {
  local sample_id=$1
  local log_file=$2
  fcs-genome htc \
    -r $ref_genome \
    -i ${tmp_dir}/${sample_id}.recal.bam \
    -o ${tmp_dir}/${sample_id}.vcf 2> $log_file

  if [ $? -ne 0 ]; then
    log_error "htc failed for ${sample_id}"
    return 1
  fi

  return 0
}
