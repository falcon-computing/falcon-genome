#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

if [[ $# -lt 2 ]]; then
  echo "USAGE: $0 <chr> <input.bam> <output.gvcf>"
  exit 1;
fi

chr=$1
input=$2
output=$3

check_input $input
check_output $output

chr_list="$(seq 1 22) X Y MT"
# Check chromosome format 
if [[ "$chr_list" != *"$chr"* ]]; then
  echo "Does not recognize '$chr' as valid chromosome format"
  exit 1;
fi

start_ts=$(date +%s)
set -x
$JAVA -d64 -Xmx32g -jar $GATK \
    -T HaplotypeCaller \
    -R $ref_genome \
    -I $input \
    --emitRefConfidence GVCF \
    --variant_index_type LINEAR \
    --variant_index_parameter 128000 \
    -L $chr \
    -nct 8 \
    -o $output
set +x
end_ts=$(date +%s)
echo "HaplotypeCaller on CH:$chr of $(basename $input) finishes in $((end_ts - start_ts))s"
