#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

if [[ $# -lt 5 ]]; then
  exit 1;
fi

ref=$1
interval=$2
input=$3
BQSR=$4
output=$5
contig=$6

shift 6
user_args=$@
verbose=3
stage_name=printReads-contig$contig

echo $(hostname) >${output}.pid
echo $BASHPID >> ${output}.pid

trap "kill_task_pid" 1 2 3 9 15 

nthreads=4
#if [[ $chr > 0 && $chr < 3 ]]; then
#    nthreads=8
#fi
#if [[ $chr > 4 && $chr < 9 ]]; then
#    nthreads=6
#fi
start_ts=$(date +%s)
$JAVA -d64 -Xmx$((nthreads * 2))g -jar $GATK \
    -T PrintReads \
    -R $ref \
    $input \
    -L $interval \
    -BQSR $BQSR \
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
