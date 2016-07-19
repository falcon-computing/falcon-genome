#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

if [[ $# -lt 5 ]]; then
  exit 1;
fi

ref=$1
chr=$2
input=$3
BQSR=$4
output=$5
shift 5
user_args=$@

stage_name=printReads-chr$chr

echo $BASHPID > ${output}.pid

trap "kill_process" 1 2 3 15 

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
    -R $ref \
    -I $input \
    -L $chr \
    -BQSR $BQSR \
    -nct $nthreads \
    $user_args \
    -o $output &

pr_java_pid=$!
echo $pr_java_pid > ${output}.java.pid
wait "$pr_java_pid"

if [ "$?" -ne "0" ]; then
  exit 1;
fi

rm ${output}.java.pid
rm ${output}.pid

end_ts=$(date +%s)
#log_info "Finishes finishes in $((end_ts - start_ts))s"
