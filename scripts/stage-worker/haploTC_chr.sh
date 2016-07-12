#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

if [[ $# -lt 2 ]]; then
  echo "USAGE: $0 <chr> <input.bam> <output.gvcf>"
  exit 1;
fi

chr=$1
input=$2
output=$3
ref=$4

check_input $input
check_output $output

echo $BASHPID > ${output}.pid
kill_process(){
  kill -5 $(jobs -p)
  exit 1;
}
trap "kill_process" 1 2 3 15 

chr_list="$(seq 1 22) X Y MT"
# Check chromosome format 
if [[ "$chr_list" != *"$chr"* ]]; then
  echo "Does not recognize '$chr' as valid chromosome format"
  exit 1;
fi

nthreads=4
if [[ $chr > 0 && $chr < 3 ]]; then
    nthreads=12
fi
if [[ $chr > 2 && $chr < 8 ]]; then
    nthreads=8
fi

start_ts=$(date +%s)
$JAVA -d64 -Xmx$((nthreads * 2 + 4))g -jar $GATK \
    -T HaplotypeCaller \
    -R $ref \
    -I $input \
    --emitRefConfidence GVCF \
    --variant_index_type LINEAR \
    --variant_index_parameter 128000 \
    -L $chr \
    -nct $nthreads \
    -o $output &
hptc_java_pid=$!
echo $hptc_java_pid > ${output}.java.pid
wait "$hptc_java_pid"

if [ "$?" -ne "0" ]; then
  exit 1;
fi

rm ${output}.java.pid
rm ${output}.pid
end_ts=$(date +%s)
log_info "HaplotypeCaller on CH:$chr of $(basename $input) finishes in $((end_ts - start_ts))s"

echo "done" > ${output}.done
