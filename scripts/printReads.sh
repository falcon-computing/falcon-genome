#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

if [[ $# -lt 3 ]]; then
  echo "USAGE: $0 <input.bam> <bqsr.grp> <output.bam> <chr>"
  exit 1;
fi

input=$1
BQSR=$2
output=$3
chr=$4

check_input $input
check_input $BQSR
check_output $output

# check if index already exists
if [ ! -f ${input}.bai ]; then
  start_ts=$(date +%s)
  $SAMTOOLS index $input
  end_ts=$(date +%s)
  echo "Samtools index for $(basename $input) finishes in $((end_ts - start_ts))s"
fi

nthreads=8
if [[ $chr > 0 && $chr < 5 ]]; then
    nthreads=16
fi
if [[ $chr > 4 && $chr < 9 ]]; then
    nthreads=12
fi

start_ts=$(date +%s)
$JAVA -d64 -Xmx12g -jar $GATK \
    -T PrintReads \
    -R $ref_genome \
    -I $input \
    -BQSR $BQSR \
    -nct $nthreads \
    -o $output
end_ts=$(date +%s)
echo "PrintReads for $(basename $input) finishes in $((end_ts - start_ts))s"

# Delete the uncalibrated input
rm $input
rm ${input}.bai
