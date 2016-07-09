#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

if [[ $# -lt 3 ]]; then
  echo "USAGE: $0 <input.bam> <bqsr.grp> <output.bam> <chr>"
  exit 1;
fi

input=$1
BQSR=$2
output=$3
chr=$4
ref=$5

check_input $input
check_input $BQSR
#check_output ${output}.done

echo $BASHPID
echo $BASHPID > ${output}.pid

kill_process(){
#  echo "PrintReads interrupted"
  kill -5 $(jobs -p)
#  kill "$pr_java_pid"
  exit 1;
}
trap "kill_process" 1 2 3 15 
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
    -R $ref \
    -I $input \
    -BQSR $BQSR \
    -nct $nthreads \
    -L $chr \
    -o $output &
pr_java_pid=$!
echo $pr_java_pid > ${output}.java.pid

#if [ "$?" -ne "0" ]; then
#  echo "PrintReads for $(basename $input) failed"
#  exit -1;
#fi
wait "$pr_java_pid"
rm ${output}.java.pid
rm ${output}.pid

end_ts=$(date +%s)
echo "PrintReads for $(basename $output) finishes in $((end_ts - start_ts))s"

echo "done" > ${output}.done
