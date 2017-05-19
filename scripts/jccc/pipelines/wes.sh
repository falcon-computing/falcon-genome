#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../common.sh
source $DIR/../settings.sh

# Check if fcs-genome is available
which fcs-genome &> /dev/null
if [ "$?" -gt 0 ]; then
  echo "Cannot find 'fcs-genome' in PATH"
  exit -1
fi

if [ $# -lt 3 ]; then
  echo "USAGE: $0 sample_id fastq_dir dest_dir"
  exit -1
fi

sample_id=$1
fastq_dir=$2
dest_dir=$3

mkdir -p $dest_dir

log_file=$log_dir/wes-${sample_id}.log
lock_dir=/tmp/wes-${sample_id}-lock
if mkdir $lock_dir; then
  log_info "Start dnaseq pipeline for $sample_id"
else
  echo "Another dnaseq pipeline for $sample_id is running, exiting..." >&2
  rm -r $lock_dir
  exit -1
fi

eval 'fastq_files=($(ls $fastq_dir/${sample_id}_S*))'
num_groups=$((${#fastq_files[@]} / 2))

start_ts=$(date +%s)
for i in $(seq 0 $((num_groups - 1))); do 

  fastq_1=${fastq_files[$(($i * 2))]}
  fastq_2=${fastq_files[$(($i * 2 + 1))]}

  read_group=`echo $(basename $fastq_1) | sed 's/\_R1.*//'`
  library=$sample_id

  if [ ! -f $fastq_1 ] && [ ! -f $fastq_2 ]; then
    log_error "Cannot find input files"
    rm -r $lock_dir
    exit 1
  fi

  fcs-genome align \
      -1 $fastq_1 \
      -2 $fastq_2 \
      -o $tmp_dir/$sample_id \
      -r $ref_genome \
      --align-only \
      -S $sample_id \
      -R $read_group \
      -L $library \
      -P Illumina \
      -f 2>> $log_file

  if [ $? -ne 0 ]; then
    log_error "alignment failed for $fastq_1"
    rm -r $lock_dir
    exit -1
  fi
done

fcs-genome markDup \
    -i ${tmp_dir}/$sample_id \
    -o ${tmp_dir}/${sample_id}.bam \
    -f 2>> $log_file 

if [ $? -ne 0 ]; then
  log_error "markdup failed for "
  rm -r $lock_dir
  exit -1
fi

rm -rf ${tmp_dir}/${sample_id} &
cp $tmp_dir/${sample_id}.bam $dest_dir &

fcs-genome ir \
    -r $ref_genome \
    -i ${tmp_dir}/${sample_id}.bam \
    -o ${tmp_dir}/${sample_id}.realign.bam \
    -K $g1000_indels \
    -K $g1000_gold_standard_indels \
    -f 2>> $log_file

if [ $? -ne 0 ]; then
  log_error "indel realignment failed for ${tmp_dir}/${sample_id}.bam"
  rm -r $lock_dir
  exit -1
fi

rm -rf ${tmp_dir}/${sample_id}.bam &

fcs-genome bqsr \
    -r $ref_genome \
    -i ${tmp_dir}/${sample_id}.realign.bam \
    -o ${tmp_dir}/${sample_id}.recal.bam \
    -K $db138_SNPs \
    -K $g1000_indels \
    -K $g1000_gold_standard_indels \
    -f 2>> $log_file

if [ $? -ne 0 ]; then
  log_error "base recal failed for ${tmp_dir}/${sample_id}.realign.bam"
  rm -r $lock_dir
  exit -1
fi

rm -rf ${tmp_dir}/${sample_id}.realign.bam &

# annovar flow 
$DIR/annovar.sh $sample_id $dest_dir >> $log_file

if [ $? -ne 0 ]; then
  log_error "annotate_variation for ${sample_id}.vcf failed"
  rm -r $lock_dir
  exit -1
fi

rm -rf ${tmp_dir}/${sample_id}.recal.bam &

# TODO rsync
rsync -ar `readlink -f $dest_dir/..` $remote_vcf_loc
if [ $? -ne 0 ]; then
  log_error "cannot transfer results to remote"
  rm -r $lock_dir
  exit -1
fi

end_ts=$(date +%s)
log_info "dnaseq pipeline finishes in $((end_ts - start_ts)) seconds"

# cleanup
rm -rf $dest_dir
rm -r $lock_dir
