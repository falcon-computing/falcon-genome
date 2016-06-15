#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

if [[ $# -lt 2 ]]; then
  echo "USAGE: $0 <input.bam> <output.bam> <tmp_dir>"
  exit 1;
fi

input=$1
output=$2
tmp_dir=$3

check_input $input
check_output $output
check_output ${output}.dup_stats
check_output_dir $tmp_dir

$JAVA -XX:+UseSerialGC -Xmx100g -jar $PICARD \
    MarkDuplicates \
    TMP_DIR=$tmp_dir COMPRESSION_LEVEL=1 \
    INPUT=$input \
    OUTPUT=$output \
    METRICS_FILE=${output}.dups_stats \
    REMOVE_DUPLICATES=false ASSUME_SORTED=true VALIDATION_STRINGENCY=SILENT
