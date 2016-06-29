#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

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

# Use pseudo input for header
sample_id=SEQ01
RG_ID=SEQ01
platform=ILLUMINA
library=HUMsgR2AQDCAAPE

start_ts=$(date +%s)
$BWA mem -M -t 32 $ref_genome \
    -R "@RG\tID:$RG_ID\tSM:$sample_id\tPL:$platform\tLB:$library" \
    $fastq1 \
    $fastq2 | $SAMTOOLS view -S -b -u - | $SAMTOOLS sort -m 16g -@ 10 -l 1 -o $output -o $output
end_ts=$(date +%s)
echo "BWA mem for $(basename $output) finishes in $((end_ts - start_ts))s"
