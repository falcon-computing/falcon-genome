#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $SCRIPT_DIR/globals.sh

if [[ $# -lt 3 ]]; then
  echo "USAGE: $0 <fastq_1> <fastq_2> <output>"
  exit 1;
fi

fastq1=$1
fastq2=$2
output=$3

check_input $fastq1
check_input $fastq2
check_output $output

output_dir=$(dirname $output)/${output}.parts

# Use pseudo input for header
sample_id=SEQ01
RG_ID=SEQ01
platform=ILLUMINA
library=HUMsgR2AQDCAAPE

start_ts=$(date +%s)
$BWA mem -M \
    -R "@RG\tID:$RG_ID\tSM:$sample_id\tPL:$platform\tLB:$library" \
    --output_dir=$output_dir \
    $ref_genome \
    $fastq1 \
    $fastq2

# Increase the max number of files that can be opened concurrently
ulimit -n 2048

end_ts=$(date +%s)
echo "BWA mem finishes in $((end_ts - start_ts))s"

sort_files=$(find $output_dir -name part-* 2>/dev/null)
if [[ -z "$sort_files" ]]; then
  echo "Folder $output_dir is empty"
  exit 1
fi

start_ts=$(date +%s)
cat $output_dir/header $sort_files | $SAMTOOLS sort -m 16g -@ 10 -l 0 -o $output

# Remove the partial files
rm -r $output_dir

end_ts=$(date +%s)
echo "Samtools sort for $(basename $output) finishes in $((end_ts - start_ts))s"
