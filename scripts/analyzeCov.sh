#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

if [[ $# -lt 2 ]]; then
  echo "USAGE: $0 <before.grp> <after.grp>"
  exit 1;
fi

input=$1
output=$2

check_input $input
check_output $output
check_output ${output}.pdf

start_ts=$(date +%s)
$JAVA -d64 -Xmx2g -jar $GATK \
    -T AnalyzeCovariates \
    -R $ref_genome \
    -before $input \
    -after  $output \
    -plots  ${output}.pdf
end_ts=$(date +%s)
echo "AnalyzeCovariates for $(basename $input) finishes in $((end_ts - start_ts))s"
