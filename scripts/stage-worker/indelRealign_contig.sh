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
known_string=$4
target_interval=$5
output=$6
contig=$7

stage_name=realign-contig$contig

echo $(hostname) >${output}.pid
echo $BASHPID >> ${output}.pid

trap "kill_task_pid" 1 2 3 9 15 


start_ts=$(date +%s)
$JAVA -d64 -jar $GATK \
  -T IndelRealigner \
  -R $ref \
  -I $input \
  -L $interval \
  $known_string \
  -targetIntervals $target_interval \
  -o $output &
   

task_pid=$!
wait "$task_pid"

if [ "$?" -ne "0" ]; then
  rm ${output}.pid -f
  exit 1;
fi

rm ${output}.pid -f

end_ts=$(date +%s)
