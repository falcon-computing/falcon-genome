#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

if [[ $# -lt 4 ]]; then
  echo "USAGE: $0 <chr> <input.bam> <bqsr.grp> <output.bam>"
  exit 1;
fi

chr=$1
input=$2
BQSR=$3
output=$4

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

start_ts=$(date +%s)
$JAVA -d64 -Xmx4g -jar $GATK \
    -T PrintReads \
    -R $ref_genome \
    -L $chr \
    -I $input \
    -BQSR $BQSR \
    -o $output
end_ts=$(date +%s)
echo "PrintReads for $(basename $input) finishes in $((end_ts - start_ts))s"
