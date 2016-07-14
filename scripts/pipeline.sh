#############################################################################################
## This script generates gVCF files from a specified in a input file 
## (c) Di Wu 04/07/2016
#############################################################################################
#!/bin/bash

# Import global variables
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh
source $DIR/stage-worker/common.sh

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

# Check input data
if [[ $INPUT_DIR != "" ]]; then
  fastq_dir=$INPUT_DIR
fi
if [[ "${do_stage["1"]}" == "1" ]]; then
  basename=$fastq_dir/${sample_id}_1
  if [ -f "${basename}.fastq" -o -f "${basename}.fastq.gz" -o -f "${basename}.fq" ]; then
    echo "Input file found"
  else
    echo "Cannot find input file in $fastq_dir" 
    exit -1
  fi
fi

# Preparation of output directories
create_dir $log_dir
create_dir ${tmp_dir[1]}
create_dir ${tmp_dir[2]}

trap 'echo intrrupted; kill $(jobs -p)' SIGINT SIGTERM

start_ts=$(date +%s)

# Step 1: BWA alignment and sort
if [[ "${do_stage["1"]}" == "1" ]]; then

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

  # Put output in tmp_dir[1]
  output=${tmp_dir[1]}/${sample_id}.bam

  $DIR/fcs-genome align \
    -r $ref_genome \
    -fq1 $fastq_1 \
    -fq2 $fastq_2 \
    -o $output

  if [ "$?" -ne 0 ]; then
    echo "Alignment failed"
    exit 1;
  fi

  # checkpoint
  cp $output $bam_dir &
fi

# Step 2: Mark Duplicate
# - Input: sorted ${sample_id}.bam
# - Output: duplicates-removed ${sample_id}.markdups.bam
if [[ "${do_stage["2"]}" == "1" ]]; then

  input=${tmp_dir[1]}/${sample_id}.bam
  output=${tmp_dir[2]}/${sample_id}.markdups.bam

  $DIR/fcs-genome markDup \
    -i $input \
    -o $output

  if [ "$?" -ne 0 ]; then
    echo "MarkDuplicate failed"
    exit 2;
  fi

  # Delete sorted bam file
  rm $input &

  # Checkpoint markdup bam file
  cp $output $bam_dir &
  cp ${output}.dups_stats $bam_dir &
fi

# Step 3: GATK BaseRecalibrate
# - Input: ${sample_id}.markdups.bam
# - Output: recalibrated ${sample_id}.markdups.recal.bam
if [[ "${do_stage["3"]}" == "1" ]]; then

  input=${tmp_dir[2]}/${sample_id}.markdups.bam
  if [ ! -f $input ]; then
    echo "Cannot find $input"
    exit 1
  fi
  rpt_dir=$output_dir/rpt
  output=$rpt_dir/${sample_id}.recalibration_report.grp

  $DIR/fcs-genome baseRecal \
    -r $ref_genome \
    -i $input \
    -o $output

  if [ "$?" -ne 0 ]; then
    echo "BaseRecalibrator failed"
    exit 3;
  fi

  if [[ "$chkpt_pid" != "" ]]; then
    wait $chkpt_pid
  fi
fi

# Step 4: GATK PrintRead
# - Input: ${sample_id}.markdups.recal.chr${chr}.bam
# - Output: per chromosome calibrated bam ${sample_id}_chr${chr}.bam
if [[ "${do_stage["4"]}" == "1" ]]; then

  input_bam=${tmp_dir[2]}/${sample_id}.markdups.bam
  input_bqsr=$output_dir/rpt/${sample_id}.recalibration_report.grp
  output_dir=${tmp_dir[1]}/bam_recal

  $DIR/fcs-genome printReads \
    -r $ref_genome \
    -i $input_bam \
    -bqsr $input_bqsr \
    -o $output_dir \
    -clean

  if [ "$?" -ne 0 ]; then
    echo "Print Reads failed"
    exit 4;
  fi
fi

# Step 5: GATK HaplotypeCaller
# - Input: ${sample_id}.markdups.recal.bam
# - Output: per chromosome varients ${sample_id}_$chr.gvcf
if [[ "${do_stage["5"]}" == "1" ]]; then

  input_dir=${tmp_dir[1]}/bam_recal
  vcf_dir=$output_dir/vcf

  $DIR/fcs-genome haplotypeCaller \
    -r $ref_genome \
    -i $sample_id \
    -c $input_dir \
    -o $vcf_dir \
    -clean

  if [ "$?" -ne 0 ]; then
    echo "Variant Calling failed"
    exit 5;
  fi
fi
end_ts=$(date +%s)
echo "Pipeline finishes in $((end_ts - start_ts))s"
