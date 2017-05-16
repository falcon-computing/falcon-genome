#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)
PARENTDIR="$(dirname "$DIR")"
source $PARENTDIR/globals.sh.template

sample="$2"
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
      if [[ $? -ne 0 ]]; then
        echo 'genomecov failed'
        exit 1
      fi
    fi
  done

#If single file, then do coverage calculation for only that file
elif [ -f "$PATH_TO_BAM" ];then
  samtools view -b -F 0x400 "$PATH_TO_BAM" | "$BEDTOOLS" genomecov -ibam stdin -bga >> "${sample}_coverage.bed" 
  if [[ $? -ne 0 ]]; then
    echo 'genomecov failed'
    exit 1
  fi

#If file of that name does not exist, quit
else
  echo "$PATH_TO_BAM is not valid"
  exit 1
fi

#Filter to only coding regions	
"$BEDTOOLS" intersect -loj -a "$CODING_REGION_PATH" -b "$sample"_coverage.bed > "${sample}_codingcov.bed" 
if [[ $? -ne 0 ]]; then 
  echo 'intersect failed'
  exit 1
fi

#Coverage stats calculation
perl cov_calculate.pl $sample 
if [[ $? -ne 0 ]]; then
  echo 'stats calculation failed'
  exit 1
fi

sed -i 's/,$//' "${sample}_${sample}Coverage.csv"
if [[ $? -ne 0 ]]; then
  echo 'error manipulating stats file'
  exit 1
fi

#Coverage stats plot
python cov_graph.py $sample
if [[ $? -ne 0 ]]; then
  echo 'error with plot computation'
  exit 1
fi

#Remove intermediate files
rm *.bed 
