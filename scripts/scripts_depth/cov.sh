#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)
PARENTDIR="$(dirname "$DIR")"
source $PARENTDIR/globals.sh

sample="$2"
path_to_bam="$1"

#Start
start_ts_total=$(date +%s)
print_help() {
  echo "USAGE: $0 <Path_to_BAM> <Sample_Type>"
}

log_msg() {
  local msg=$1
  >&2 echo "[coverage-stats] $msg"
}

log_error() {
  log_msg "ERROR: $1"
}

log_info() {
  log_msg "INFO: $1"
}

if [[ $# -ne 2 ]];then
  print_help
  exit 1
fi

#Declare array
declare -A pid_table

#Create directory
mkdir log_dir

#Check if the PATH_TO_BAM is a directory containing multiple recalibrated BAM files or a single recalibrated BAM file

#If directory, then do coverage calculation for each file
if [ -d "$path_to_bam" ];then  
  for file in $(ls "$path_to_bam")
  do
    if [[ $file =~ bam$ ]];then
      #Ignore duplicates in BAM file | Calculate coverage values 
      samtools view -b -F 0x400 "$path_to_bam"/"$file" | "$BEDTOOLS" genomecov -ibam stdin -bga >> "log_dir/${file}_coverage.bed" & 
      
      pid_table["$file"]=$!
    fi
  done
  
  for file in $(ls "$path_to_bam")
  do
    if [[ $file =~ bam$ ]];then
      pid=${pid_table[$file]}
      wait "${pid}"
      if [[ $? -ne 0 ]]; then
        log_error "Failed to calculate genome coverage for $file"
        exit 1
      fi 
      file_log=log_dir/${file}_coverage.bed
      cat $file_log >> ${sample}_coverage.bed
      rm -f $file_log
    fi
  done

#If single file, then do coverage calculation for only that file
elif [ -f "$path_to_bam" ];then
  samtools view -b -F 0x400 "$path_to_bam" | "$BEDTOOLS" genomecov -ibam stdin -bga >> "${sample}_coverage.bed" 
  if [[ $? -ne 0 ]]; then
    log_error "Failed to calculate genome coverage for $path_to_bam"
    exit 1
  fi

#If file of that name does not exist, quit
else
  log_error "$path_to_bam is not a valid path"
  exit 1
fi

#Filter to only coding regions	
"$BEDTOOLS" intersect -loj -a "$coding_region_path" -b "$sample"_coverage.bed > "${sample}_codingcov.bed" 
if [[ $? -ne 0 ]]; then 
  log_error "Failed to filter just coding regions from the genome coverage file"  
  exit 1
fi

#Coverage stats calculation
perl cov_calculate.pl $sample 
if [[ $? -ne 0 ]]; then
  log_error "Failed to calculate coverage statistics"
  exit 1
fi

sed -i 's/,$//' "${sample}_${sample}Coverage.csv"
if [[ $? -ne 0 ]]; then
  log_error "Failed to manipulate the coverage statistics file"
  exit 1
fi

#Coverage stats plot
python cov_graph.py $sample
if [[ $? -ne 0 ]]; then
  log_error "Failed to compute coverage plot"
  exit 1
fi

#Remove intermediate files
rm ${sample}_coverage.bed
rm ${sample}_codingcov.bed
rm -r log_dir

#Time taken
end_ts=$(date +%s)
log_info "Finishes in $((end_ts - start_ts_total))s" 
