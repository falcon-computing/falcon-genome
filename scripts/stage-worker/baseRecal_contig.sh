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
knownSites="$4"
output=$5
contig=$6

stage_name=baseRecal-contig$contig

echo $(hostname) > ${output}.pid
echo $BASHPID >> ${output}.pid

trap "kill_task_pid" 1 2 3 9 15 

nthreads=4

start_ts=$(date +%s)
$JAVA -d64 -Xmx$((nthreads * 2))g -jar $GATK \
   -T BaseRecalibrator \
   -R $ref \
   $input \
   $knownSites \
   -L $interval \
   -nct $nthreads \
   -o $output &
 
task_pid=$!
wait "$task_pid"

if [ "$?" -ne "0" ]; then
  rm ${output}.pid -f
  exit 1;
fi

rm ${output}.pid -f

end_ts=$(date +%s)
>&2 echo "BQSR for contig $contig finishes in $((end_ts - start_ts))s"
