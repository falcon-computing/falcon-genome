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

# Check the command
if [ ! -z $help_req ];then
  echo "USAGE: fcs_genome split -i <input> -o <output_dir> "
  exit 1;
fi

if [ -z $input ];then
  echo "The input is missing, please check the cmd"
  exit 1;
fi

if [ -z $output ];then
  output=${tmp_dir[1]}
  echo "The output dir is not specified, would put the splited bam to $output"
fi

check_input $input
check_output_dir $output

input_fname=$(basename $input)
input_base="${input_fname%.*}"
chr_list="$(seq 1 22) X Y MT"

# Split BAM by chromosome
start_ts=$(date +%s)
set -x
for chr in $chr_list; do
  chr_bam=$output/${input_base}.chr${chr}.bam
  if [ ! -f $chr_bam ]; then
    $SAMTOOLS view -u -b -S $input $chr > $chr_bam
    $SAMTOOLS index $chr_bam
  fi
done
set +x
end_ts=$(date +%s)
echo "Samtools split for $(basename $input) finishes in $((end_ts - start_ts))s" | tee -a $log_dir/${sample_id}.bqsr.time.log



