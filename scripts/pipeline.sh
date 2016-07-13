#############################################################################################
## This script generates gVCF files from a specified in a input file 
## (c) Di Wu 04/07/2016
#############################################################################################

#!/bin/bash

# Import global variables
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

if [[ $# -lt 1 ]]; then
  echo "USAGE: $0 <sample id> [1..5]"
  exit 1;
fi

sample_id=$1

shift

# dictionary to enable selective stages
declare -A do_stage
if [[ $# -gt "0" ]]; then
  do_stage=( ["1"]="0" ["2"]="0" ["3"]="0" ["4"]="0" ["5"]="0" )
  for i in $@; do 
    do_stage["$i"]=1
  done
else
  do_stage=( ["1"]="1" ["2"]="1" ["3"]="1" ["4"]="1" ["5"]="1" )
fi

# Step 1: BWA alignment
if [[ "${do_stage["1"]}" == "1" ]]; then
  if [ ! -d "$fastq_dir" ]; then
    echo "Cannot find fastq dir: $fastq_dir" 
    exit 1
  fi

  create_dir $bam_dir
  fastq_1=$fastq_dir/${sample_id}_1.fastq
  fastq_2=$fastq_dir/${sample_id}_2.fastq
  if [ ! -f $fastq_1 ]; then
    fastq_1=$fastq_dir/${sample_id}_1.fastq.gz
    fastq_2=$fastq_dir/${sample_id}_2.fastq.gz
  fi
  if [ ! -f $fastq_1 ]; then
    fastq_1=$fastq_dir/${sample_id}_1.fq
    fastq_2=$fastq_dir/${sample_id}_2.fq
  fi
  output=$bam_dir/${sample_id}.bam
  $DIR/align.sh $fastq_1 $fastq_2 $output 2> align.log
fi

# Step 2: Mark Duplicate
# - Input: sorted ${sample_id}.bam
# - Output: duplicates-removed ${sample_id}.markdups.bam
if [[ "${do_stage["2"]}" == "1" ]]; then
  input=$bam_dir/${sample_id}.bam
  output=$bam_dir/${sample_id}.markdups.bam
  $DIR/markDup.sh $input $output 2> markDup.log
fi

# Step 3: GATK BaseRecalibrate
# - Input: ${sample_id}.markdups.bam
# - Output: recalibrated ${sample_id}.markdups.recal.bam
if [[ "${do_stage["3"]}" == "1" ]]; then
  input=$bam_dir/${sample_id}.markdups.bam
  if [ ! -f $input ]; then
    echo "Cannot find $input"
    exit 1
  fi

  start_ts=$(date +%s)
  # Indexing input if it is not indexed
  if [ ! -f ${input}.bai ]; then
    $SAMTOOLS index $input $chr
  fi
  end_ts=$(date +%s)
  echo "Samtools index for $(basename $input) finishes in $((end_ts - start_ts))s"

  create_dir $rpt_dir

  start_ts=$(date +%s)
  output_rpt=$rpt_dir/${sample_id}.recalibration_report.grp
  if [ ! -f $output_rpt ]; then
    $DIR/baseRecal.sh $input $output_rpt 2> baseRecal.log
  else 
    echo "WARNING: $output_rpt already exists, assuming BaseRecalibration already done"
  fi
  end_ts=$(date +%s)
  echo "BaseRecalibrator stage finishes in $((end_ts - start_ts))s"

  start_ts=$(date +%s)
  $SAMTOOLS index $input
  end_ts=$(date +%s)
  echo "Samtools index for $(basename $input) finishes in $((end_ts - start_ts))s"

  # Table storing all the pids for tasks within one stage
  declare -A pid_table

  input_bam=$bam_dir/${sample_id}.markdups.bam
  input_rpt=$output_rpt

  chr_list="$(seq 1 22) X Y MT"
  start_ts=$(date +%s)
  for chr in $chr_list; do
    chr_recal_bam=$bam_dir/${sample_id}.recal.chr${chr}.bam
    $DIR/printReads.sh "$chr" "$input_bam" "$input_rpt" "$chr_recal_bam" 2> printReads_chr${chr}.log &
    pid_table["$chr"]=$!
  done
  # Wait on all the tasks
  for pid in ${pid_table[@]}; do
    wait "${pid}"
  done
  end_ts=$(date +%s)
  echo "PrintReads stage finishes in $((end_ts - start_ts))s"
fi

# Step 4: GATK HaplotypeCaller
# - Input: ${sample_id}.markdups.recal.bam
# - Output: per chromosome varients ${sample_id}_$chr.gvcf
if [[ "${do_stage["4"]}" == "1" ]]; then
  chr_list="$(seq 1 22) X Y MT"
  declare -A pid_table

  create_dir $vcf_dir

  start_ts=$(date +%s)
  for chr in $chr_list; do
    chr_bam=$bam_dir/${sample_id}.recal.chr${chr}.bam
    chr_vcf=$vcf_dir/${sample_id}_chr${chr}.gvcf
    $DIR/haploTC.sh $chr $chr_bam $chr_vcf 2> haplotypeCaller_chr${chr}.log &
    pid_table["$chr"]=$!
  done

  # Wait on all the tasks
  for pid in ${pid_table[@]}; do
    wait "${pid}"
  done
  end_ts=$(date +%s)
  echo "HaplotypeCaller stage finishes in $((end_ts - start_ts))s"
fi
