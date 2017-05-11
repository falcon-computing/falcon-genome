#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)
source $DIR/globals.sh.template

sample="Tumor"
PATH_TO_BAM="$1"     #/pool/storage/diwu/annovar/LB-2907-TumorDNA.recal.bam
CODING_REGION_PATH="/curr/niveda/coverage/FCS/hg19_coding_merge.bed"

#Check if the PATH_TO_BAM is a directory containing multiple recalibrated BAM files or a single recalibrated BAM file

#If directory, then do coverage calculation for each file
if [ -d "$PATH_TO_BAM" ];then  
  for file in $(ls "$PATH_TO_BAM")
  do
    if [[ $file =~ bam$ ]];then
      #Ignore duplicates in BAM file | Calculate coverage values 
      samtools view -b -F 0x400 "$PATH_TO_BAM"/"$file" | "$BEDTOOLS" genomecov -ibam stdin -bga >> "${sample}_coverage.bed"
    fi
  done
fi

#If single file, then do coverage calculation for only that file
if [ -f "$PATH_TO_BAM" ];then
  samtools view -b -F 0x400 "$PATH_TO_BAM"/"$file" | "$BEDTOOLS" genomecov -ibam stdin -bga >> "${sample}_coverage.bed"
fi

#Filter to only coding regions	
"$BEDTOOLS" intersect -loj -a "$CODING_REGION_PATH" -b "$sample"_coverage.bed > "${sample}_codingcov.bed"

#Coverage stats calculation
perl cov_calculate.pl $sample


sed -i 's/,$//' "${sample}_${sample}Coverage.csv"

#Coverage stats plot
python cov_graph.py $sample
 
