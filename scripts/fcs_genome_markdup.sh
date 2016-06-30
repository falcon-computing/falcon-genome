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
    -o|--output)
    output="$2"
    shift
    ;;
    -t|--temp_dir)
    tmp_dir="$2"
    shift
    ;;
    *)
            # unknown option
    ;;
esac
shift # past argument or value
done

if [ -z $input ];then
  echo "The bam input is missing, please check the cmd"
  exit 1;
fi 

if [ -z $output ];then
  output=${tmp_dir[2]}/$(basename $input).markdups.bam
  echo "The output file is not specified, would store the bam to $output"
fi

if [ -z $tmp_dir ];then
  tmp_dir=${tmp_dir[1]}
  echo "The temp dir is not specified, would store the temp files to $tmp_dir"
fi
# Find the input bam

check_input $input
check_output $output
check_output ${output}.dup_stats
check_output_dir $tmp_dir

$JAVA -XX:+UseSerialGC -Xmx160g -jar $PICARD \
    MarkDuplicates \
    TMP_DIR=$tmp_dir COMPRESSION_LEVEL=1 \
    INPUT=$input \
    OUTPUT=$output \
    METRICS_FILE=${output}.dups_stats \
    REMOVE_DUPLICATES=false ASSUME_SORTED=true VALIDATION_STRINGENCY=SILENT

