#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Missing command"
  exit -1;
fi

output=`manager/client`
output_arr=($output)
host=${output_arr[0]}
pid=${output_arr[1]}

# launch process
set -x
if [ -e "$1" ]; then
  ssh $host 'bash -s' < $1
else
  ssh $host $1
fi
set +x

manager/client $pid
