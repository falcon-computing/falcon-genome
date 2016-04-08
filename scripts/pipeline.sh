#############################################################################################
## This script generates gVCF files from a specified in a input file 
## (c) Di Wu 04/07/2016
#############################################################################################

#!/bin/bash

# Import global variables
source globals.sh

if [[ $# -lt 1 ]]; then
  echo "USAGE: $0 <sample id> [1..5]"
  exit 1;
fi

sample_id=$1
RG_ID=SEQ01
platform=ILLUMINA
library=HUMsgR2AQDCAAPE

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

set -x

# Step 1: BWA alignment
# - Input: pair_end fastq files: ${sample_id}_1.fastq.gz ${sample_id}_2.fastq.gz
# - Output: ${sample_id}.sam

if [[ "${do_stage["1"]}" == "1" ]]; then
  # Check if input file exist
  if [ ! -f $fastq_dir/${sample_id}_1.fastq ] || [ ! -f $fastq_dir/${sample_id}_1.fastq ]; then
    echo "Cannot find input fastq files"
    exit 1
  fi
  start_ts=$(date +%s)
  $BWA mem -M -t 24 $ref_genome \
      -R "@RG\tID:$RG_ID\tSM:$sample_id\tPL:$platform\tLB:$library" \
      $fastq_dir/${sample_id}_1.fastq \
      $fastq_dir/${sample_id}_2.fastq > $sam_dir/${sample_id}.sam
  end_ts=$(date +%s)
  echo "#1 BWA mem finishes in $((end_ts - start_ts))s"
fi

# Step 2: Samtools Sort
# - Input: ${sample_id}.sam
# - Output: sorted ${sample_id}.bam
if [[ "${do_stage["2"]}" == "1" ]]; then
  if [ ! -f $sam_dir/${sample_id}.sam ]; then
    echo "Cannot find $sam_dir/${sample_id}.sam"
    exit 1
  fi
  start_ts=$(date +%s)
  cat $sam_dir/${sample_id}.sam | $SAMTOOLS view -S -b -u - | $SAMTOOLS sort -o $bam_dir/${sample_id}.bam
  end_ts=$(date +%s)
  echo "#2 Samtools sort finishes in $((end_ts - start_ts))s"
fi

# These steps can be omitted
# Step 3 (REMOVED): Check BAM file
#echo "$SAMTOOL view -H $bam_dir/${sample_id}.bam > tmp.out"
#rm -f tmp.out err

# Step 4 (REMOVED): 
#$BAMTOOL merge \
#    -list $input_dir/$SampleID/Logs/list_BAMs_tomerge.txt \
#    -out $input_dir/$SampleID/$SampleID.merged.bam


# Step 3: Mark Duplicate
# - Input: sorted ${sample_id}.bam
# - Output: duplicates-removed ${sample_id}.markdups.bam
if [[ "${do_stage["3"]}" == "1" ]]; then
  if [ ! -f $bam_dir/${sample_id}.bam ]; then
    echo "Cannot find $bam_dir/${sample_id}.bam"
    exit 1
  fi
  start_ts=$(date +%s)
  $JAVA -XX:+UseSerialGC -Xmx2g -jar $PICARD \
      MarkDuplicates \
      TMP_DIR=/tmp COMPRESSION_LEVEL=5 \
      INPUT=$bam_dir/${sample_id}.bam \
      OUTPUT=$bam_dir/${sample_id}.markdups.bam \
      METRICS_FILE=$rpt_dir/${sample_id}.bam.dups_stats \
      REMOVE_DUPLICATES=true ASSUME_SORTED=true VALIDATION_STRINGENCY=SILENT
  end_ts=$(date +%s)
  echo "#3-1 Picard mark duplicate finishes in $((end_ts - start_ts))s"

  start_ts=$(date +%s)
  $SAMTOOLS index $bam_dir/${sample_id}.markdups.bam
  end_ts=$(date +%s)
  echo "#3-2 Samtools index finishes in $((end_ts - start_ts))s"
fi

# Step 4: GATK BaseRecalibrate
# - Input: ${sample_id}.markdups.bam
# - Output: recalibrated ${sample_id}.markdups.recal.bam
if [[ "${do_stage["4"]}" == "1" ]]; then
  if [ ! -f $bam_dir/${sample_id}.markdups.bam ]; then
    echo "Cannot find $bam_dir/${sample_id}.markdups.bam"
    exit 1
  fi
  # - first recalibrate the markdups.bam file and generate a report
  start_ts=$(date +%s)
  $JAVA -d64 -Xmx2g -jar $GATK \
      -T BaseRecalibrator \
      -R $ref_genome \
      -I $bam_dir/${sample_id}.markdups.bam \
      -knownSites $g1000_indels \
      -knownSites $g1000_gold_standard_indels \
      -knownSites $db138_SNPs \
      -o $rpt_dir/${sample_id}.recalibration_report.grp
  end_ts=$(date +%s)
  echo "#4-1 BaseRecalibrator finishes in $((end_ts - start_ts))s"
  
  # - then use print reads to apply the calibration
  start_ts=$(date +%s)
  $JAVA -Xmx2g -jar $GATK \
      -T PrintReads \
      -R $ref_genome \
      -I $bam_dir/${sample_id}.markdups.bam \
      -BQSR $rpt_dir/${sample_id}.recalibration_report.grp \
      -o $bam_dir/${sample_id}.markdups.recal.bam
  end_ts=$(date +%s)
  echo "#4-2 PrintReads finishes in $((end_ts - start_ts))s"
  
  # - then recalibrate again to generate another report
  start_ts=$(date +%s)
  $JAVA -d64 -Xmx2g -jar $GATK \
      -T BaseRecalibrator \
      -R $ref_genome \
      -I $bam_dir/${sample_id}.markdups.recal.bam \
      -knownSites $g1000_indels \
      -knownSites $g1000_gold_standard_indels \
      -knownSites $db138_SNPs \
      -o $rpt_dir/${sample_id}.postrecalibration_report.grp
  end_ts=$(date +%s)
  echo "#4-3 BaseRecalibrator finishes in $((end_ts - start_ts))s"
  
  # - finally calculate a covariate report
  start_ts=$(date +%s)
  $JAVA -d64 -Xmx2g -jar $GATK \
      -T AnalyzeCovariates \
      -R $ref_genome \
      -before $rpt_dir/${sample_id}.recalibration_report.grp \
      -after  $rpt_dir/${sample_id}.postrecalibration_report.grp \
      -plots  $rpt_dir/${sample_id}.recalibration_plot.pdf
  end_ts=$(date +%s)
  echo "#4-4 AnalyzeCovariates finishes in $((end_ts - start_ts))s"
  
  start_ts=$(date +%s)
  $SAMTOOLS index $bam_dir/${sample_id}.markdups.recal.bam
  end_ts=$(date +%s)
  echo "#4-5 Samtools index finishes in $((end_ts - start_ts))s"
fi

# Step 5: GATK HaplotypeCaller
# - Input: ${sample_id}.markdups.recal.bam
# - Output: per chromosome varients ${sample_id}_$chr.gvcf
if [[ "${do_stage["5"]}" == "1" ]]; then
  if [ ! -f $bam_dir/${sample_id}.markdups.recal.bam ]; then
    echo "Cannot find $bam_dir/${sample_id}.markdups.recal.bam"
    exit 1
  fi
  chr_list="$(seq 1 22) X Y MT"
  for chr in $chr_list; do
  start_ts=$(date +%s)
  $JAVA -Xmx2g -jar $GATK \
      -T HaplotypeCaller \
      -R $ref_genome \
      -I $bam_dir/${sample_id}.markdups.recal.bam \
      --emitRefConfidence GVCF \
      --variant_index_type LINEAR \
      --variant_index_parameter 128000 \
      -L $chr \
      -o $vcf_dir/${sample_id}_${chr}.gvcf
  end_ts=$(date +%s)
  echo "#5 HaplotypeCaller on CH:$chr finishes in $((end_ts - start_ts))s"
  done
fi
set +x
