#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

if [[ $# -lt 3 ]]; then
  echo "USAGE: $0 <input.bam> <bqsr.grp> <output.bam>"
  exit 1;
fi

input=$1
BQSR=$2
output=$3

check_input $input
check_input $BQSR
check_output $output

start_ts=$(date +%s)
$JAVA -d64 -Xmx4g -jar $GATK \
    -T PrintReads \
    -R $ref_genome \
    -I $input \
    -BQSR $BQSR \
    -o $output
end_ts=$(date +%s)
echo "PrintReads for $(basename $input) finishes in $((end_ts - start_ts))s"
