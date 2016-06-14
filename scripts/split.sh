#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

if [[ $# -lt 1 ]]; then
  echo "USAGE: $0 <sample_id> "
  exit 1;
fi

input=$1
output_dir=$2

check_input $input
check_output_dir $output_dir

input_fname=$(basename $input)
input_base="${input_fname%.*}"
chr_list="$(seq 1 22) X Y MT"

# Split BAM by chromosome
start_ts=$(date +%s)
set -x
for chr in $chr_list; do
  chr_bam=$output_dir/${input_base}.chr${chr}.bam
  if [ ! -f $chr_bam ]; then
    $SAMTOOLS view -u -b -S $input $chr > $chr_bam
    $SAMTOOLS index $chr_bam
  fi
done
set +x
end_ts=$(date +%s)
echo "Samtools split for $(basename $input) finishes in $((end_ts - start_ts))s" | tee -a $log_dir/${sample_id}.bqsr.time.log
