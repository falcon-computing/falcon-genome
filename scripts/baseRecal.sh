#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

if [[ $# -lt 2 ]]; then
  echo "USAGE: $0 <input.bam> <output.grp>"
  exit 1;
fi

input=$1
output=$2

check_input $input
check_output $output

# check if index already exists
if [ ! -f ${input}.bai ]; then
  start_ts=$(date +%s)
  $SAMTOOLS index $input
  end_ts=$(date +%s)
  echo "#4 Samtools index for $(basename $input) finishes in $((end_ts - start_ts))s"
fi

start_ts=$(date +%s)
set -x
$JAVA -d64 -Xmx2g -jar $GATK \
    -T BaseRecalibrator \
    -R $ref_genome \
    -I $input \
    -knownSites $g1000_indels \
    -knownSites $g1000_gold_standard_indels \
    -knownSites $db138_SNPs \
    -o $output
set +x
end_ts=$(date +%s)
echo "#4 BaseRecalibrator for $(basename $input) finishes in $((end_ts - start_ts))s"
