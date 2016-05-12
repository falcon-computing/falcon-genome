#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

if [[ $# -lt 2 ]]; then
  echo "USAGE: $0 <input.sam> <output.bam>"
  exit 1;
fi

input=$1
output=$2

check_input $input
check_output $output

start_ts=$(date +%s)
cat $input | $SAMTOOLS view -S -b -u - | $SAMTOOLS sort -o $output
end_ts=$(date +%s)
echo "Samtools sort for $(basename $input) finishes in $((end_ts - start_ts))s"
