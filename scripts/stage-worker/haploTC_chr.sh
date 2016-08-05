#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

if [[ $# -lt 4 ]]; then
  exit 1;
fi

ref=$1
chr=$2
input=$3
output=$4
shift 4
user_args=$@

stage_name=haploptypeCaller-chr$chr

check_input $input
check_output $output

echo $(hostname) >${output}.pid
echo $BASHPID >> ${output}.pid

trap "kill_task_pid" 1 2 3 9 15 

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
    $user_args \
    -o $output &

task_pid=$!
wait "$task_pid"

if [ "$?" -ne "0" ]; then
  rm ${output}.pid -f
  exit 1;
fi

rm ${output}.pid -f

end_ts=$(date +%s)
#log_info "Finishes in $((end_ts - start_ts))s"
