#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

if [[ $# -lt 2 ]]; then
  echo "USAGE: $0 <input.bam> <baseline.bam>"
  exit 1;
fi

eval_file=$1
base_file=$2

check_input $eval_file
check_input $base_file

# First check binary match or not
diff $eval_file $base_file &> /dev/null
if [ "$?" -ne 0 ]; then
  cnt=`$BAMUTIL diff --in1 $eval_file --in2 $base_file | wc -l`
  if [ "$cnt" -ne 0 ]; then
    echo "$(basename $eval_file) and $(basename $base_file) have $cnt differences"
    exit -1
  fi
fi
echo "" > $(basename $eval_file).done
