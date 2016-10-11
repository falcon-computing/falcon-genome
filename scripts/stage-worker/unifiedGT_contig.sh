#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

if [[ $# -lt 4 ]]; then
  exit 1;
fi

ref=$1
interval=$2
input=$3
output=$4
contig=$5
shift 5
user_args=$@

stage_name=unifiedGT-contig$contig

check_input $input
check_output $output

echo $(hostname) >${output}.pid
echo $BASHPID >> ${output}.pid

trap "kill_task_pid" 1 2 3 9 15 

nthreads=4
#if [[ $chr > 0 && $chr < 3 ]]; then
#    nthreads=12
#fi
#if [[ $chr > 2 && $chr < 8 ]]; then
#    nthreads=8
#fi

start_ts=$(date +%s)
$JAVA -d64 -Xmx$((nthreads * 2 + 4))g -jar $GATK \
    -T UnifiedGenotyper \
    -R $ref \
    -I $input \
    -L $interval \
    -nt $nthreads \
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
