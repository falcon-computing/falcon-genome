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

nthreads=4
if [[ $chr > 0 && $chr < 3 ]]; then
    nthreads=8
fi
if [[ $chr > 4 && $chr < 9 ]]; then
    nthreads=6
fi

start_ts=$(date +%s)
$JAVA -d64 -Xmx$((nthreads * 2))g -jar $GATK \
    -T PrintReads \
    -R $ref_genome \
    -L $chr \
    -I $input \
    -L $chr \
    -BQSR $BQSR \
    -nct $nthreads \
    -o $output

if [ "$?" -ne "0" ]; then
  echo "PrintReads for $(basename $input) failed"
  exit -1;
fi
end_ts=$(date +%s)
echo "PrintReads for $(basename $input) finishes in $((end_ts - start_ts))s"
echo "done" > ${output}.done
