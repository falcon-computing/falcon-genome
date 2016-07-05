################################################################################
## This script generates sorted bam file from fastq using bwa-flow
################################################################################
#!/bin/bash

# Import global variables
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

# Get the input command 
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -i|--input)
    input="$2"
    shift # past argument
    ;;
    -h|--help)
    help_req=YES
    ;;
    *)
            # unknown option
    ;;
esac
shift # past argument or value
done

if [ ! -z $help_req ];then
  echo " USAGE: fcs_genome index -i <input_file>"
  echo " The input_file argument is the markduped bam file"
fi

if [ -z $input ];then
  echo "The input argument is missing, please check the cmd"
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
