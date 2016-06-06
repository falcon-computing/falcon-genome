#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

if [[ $# -lt 2 ]]; then
  echo "USAGE: $0 <input.sam> <output.bam>"
  exit 1;
fi

input=$1
output=$2

# check_input $input
if [ ! -d $input ]; then
  echo "Input folder $input does not exist"
  exit 1
fi

input_files=$(find $input -name part-* 2>/dev/null)
if [[ -z "$input_files" ]]; then
  echo "Input folder $input is empty"
  exit 1
fi

check_output $output

# Increase the max number of files that can be opened concurrently
ulimit -n 2048

start_ts=$(date +%s)
$SAMTOOLS merge -r -c -p -l 0 -@ 8 ${output} $input_files
end_ts=$(date +%s)
echo "Samtools sort for $(basename $input) finishes in $((end_ts - start_ts))s"
