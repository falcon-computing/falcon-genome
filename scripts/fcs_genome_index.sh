################################################################################
## This script generates the bam index of the input bam file using samtools
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
  echo " The output *.bai file would be put in the same directory as the input"
fi

if [ -z $input ];then
  echo "The input argument is missing, please check the cmd"
  echo " USAGE: fcs_genome index -i <input_file>"
  exit 1;
fi

if [ ! -f $input ]; then
  >&2 echo "Cannot find $input for index, please check if the input file exist"
  exit 1
fi

output=${input}.bai
check_output $output

echo "Starting index"
start_ts=$(date +%s)
  
$SAMTOOLS index $input 

end_ts=$(date +%s)
echo "Samtools index for $(basename $input) finishes in $((end_ts - start_ts))s"
echo "The output file is ${input}.bai"
