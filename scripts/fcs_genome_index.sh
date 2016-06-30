#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -i|--input)
    input="$2"
    shift # past argument
    ;;
    *)
            # unknown option
    ;;
esac
shift # past argument or value
done

if [ -z $input ];then
  echo "The input arg is missing, please check the cmd"
  exit 1;
fi

if [ ! -f $input ]; then
  echo "Cannot find $input for index"
  exit 1
fi

start_ts=$(date +%s)
# Indexing input if it is not indexed
if [ ! -f ${input}.bai ]; then
  $SAMTOOLS index $input 
else
  echo "The input is already indexed"
fi

end_ts=$(date +%s)
echo "Samtools index for $(basename $input) finishes in $((end_ts - start_ts))s"
