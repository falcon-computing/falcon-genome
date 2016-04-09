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
  fastq_1=$fastq_dir/${sample_id}_1.fastq
  fastq_2=$fastq_dir/${sample_id}_2.fastq
  output=$sam_dir/${sample_id}.sam
  $DIR/align.sh $fastq_1 $fastq_2 $output 2> align.log
fi

# Step 2: Samtools Sort
if [[ "${do_stage["2"]}" == "1" ]]; then
  input=$sam_dir/${sample_id}.sam
  output=$bam_dir/${sample_id}.bam
  $DIR/sort.sh $input $output 2> sort.log
fi

# Step 3: Mark Duplicate
# - Input: sorted ${sample_id}.bam
# - Output: duplicates-removed ${sample_id}.markdups.bam
if [[ "${do_stage["3"]}" == "1" ]]; then
  input=$bam_dir/${sample_id}.bam
  output=$bam_dir/${sample_id}.markdups.bam
  $DIR/markDup.sh $input $output 2> markDup.log
fi

# Step 4: GATK BaseRecalibrate
# - Input: ${sample_id}.markdups.bam
# - Output: recalibrated ${sample_id}.markdups.recal.bam
if [[ "${do_stage["4"]}" == "1" ]]; then
  input=$bam_dir/${sample_id}.markdups.bam
  if [ ! -f $input ]; then
    echo "Cannot find $input"
    exit 1
  fi

  # Split BAM by chromosome
  chr_list="$(seq 1 22) X Y MT"
  for chr in $chr_list; do
    chr_bam=$bam_dir/${sample_id}.markdups.chr${chr}.bam
    if [ ! -f $chr_bam ]; then
      $SAMTOOLS view -u -b -S $input $chr > $chr_bam
    fi
  done
  
  # Table storing all the pids for tasks within one stage
  declare -A pid_table

  start_ts=$(date +%s)
  for chr in $chr_list; do
    chr_bam=$bam_dir/${sample_id}.markdups.chr${chr}.bam
    chr_rpt=$rpt_dir/${sample_id}.chr${chr}.recalibration_report.grp
    $DIR/baseRecal.sh $chr_bam $chr_rpt 2> baseRecal_chr${chr}.log &
    pid_table["$chr"]=$!
  done
  # Wait on all the tasks
  for pid in ${pid_table[@]}; do
    wait "${pid}"
  done
  end_ts=$(date +%s)
  echo "BaseRecalibrator stage finishes in $((end_ts - start_ts))s"

  start_ts=$(date +%s)
  for chr in $chr_list; do
    chr_bam=$bam_dir/${sample_id}.markdups.chr${chr}.bam
    chr_rpt=$rpt_dir/${sample_id}.chr${chr}.recalibration_report.grp
    chr_recal_bam=$bam_dir/${sample_id}.recal.chr${chr}.bam
    $DIR/printReads.sh $chr_bam $chr_rpt $chr_recal_bam 2> printReads_chr${chr}.log &
    pid_table["$chr"]=$!
  done
  # Wait on all the tasks
  for pid in ${pid_table[@]}; do
    wait "${pid}"
  done
  end_ts=$(date +%s)
  echo "PrintReads stage finishes in $((end_ts - start_ts))s"
fi

# Step 5: GATK HaplotypeCaller
# - Input: ${sample_id}.markdups.recal.bam
# - Output: per chromosome varients ${sample_id}_$chr.gvcf
if [[ "${do_stage["5"]}" == "1" ]]; then
  chr_list="$(seq 1 22) X Y MT"
  declare -A pid_table

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
