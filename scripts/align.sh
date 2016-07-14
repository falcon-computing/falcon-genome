#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $SCRIPT_DIR/globals.sh

if [[ $# -lt 3 ]]; then
  echo "USAGE: $0 <fastq_1> <fastq_2> <output> <tmp_dir>"
  exit 1;
fi

bwa_sort=1

fastq1=$1
fastq2=$2
output=$3
tmp_dir=$4

check_input $fastq1
check_input $fastq2
check_output $output
check_output_dir $tmp_dir

output_dir=$tmp_dir/$(basename $output).parts

# Use pseudo input for header
sample_id=SEQ01
RG_ID=SEQ01
platform=ILLUMINA
library=HUMsgR2AQDCAAPE

if [ "$bwa_sort" -gt 0 ]; then
  ext_options="--sort --max_num_records=2000000"
else
  ext_options=
fi

echo "Started BWA alignment"
start_ts=$(date +%s)
$BWA mem -M \
    -R "@RG\tID:$RG_ID\tSM:$sample_id\tPL:$platform\tLB:$library" \
    --log_dir=$log_dir \
    --output_dir=$output_dir \
    $ext_options \
    $ref_genome \
    $fastq1 \
    $fastq2

if [ "$?" -ne 0 ]; then 
  echo "BWAMEM failed"
  exit 1
fi
end_ts=$(date +%s)
echo "BWA mem finishes in $((end_ts - start_ts))s"

# Increase the max number of files that can be opened concurrently
ulimit -n 2048

sort_files=$(find $output_dir -name part-* 2>/dev/null)
if [[ -z "$sort_files" ]]; then
  echo "Folder $output_dir is empty"
  exit 1
fi

start_ts=$(date +%s)
if [ "$bwa_sort" -gt 0 ]; then
  $SAMTOOLS merge -r -c -p -l 1 -@ 10 ${output} $sort_files
else
  cat $sort_files | $SAMTOOLS sort -m 16g -@ 10 -l 0 -o $output
fi

if [ "$?" -ne 0 ]; then 
  echo "Sorting failed"
  exit 1
fi

# Remove the partial files
rm -r $output_dir

end_ts=$(date +%s)
echo "Samtools sort for $(basename $output) finishes in $((end_ts - start_ts))s"
