#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

stage_name=compressVCF

input_gvcf_file=$1
ref_fasta=$2
output_vcf=${input_gvcf_file}.genotype.vcf

echo $(hostname) >${input_gvcf_file}.pid
echo $BASHPID >> ${input_gvcf_file}.pid


kill_task_pid() {
  log_info "kill $task_pid"
  kill $task_pid 2> /dev/null
  exit 1
}

trap "kill_task_pid" 1 2 3 9 15

$BGZIP -c $input_gvcf_file > ${input_gvcf_file}.gz &
task_pid=$!
wait "$task_pid"
if [ "$?" -ne "0" ]; then
  log_error "bgzip compression failed"
  rm ${input_gvcf_file}.pid -f
  exit 1;
fi

$TABIX -p vcf ${input_gvcf_file}.gz &
task_pid=$!
wait "$task_pid"
if [ "$?" -ne "0" ]; then
  log_error "tabix failed"
  rm ${input_gvcf_file}.pid -f
  exit 1;
fi

$JAVA -d64 -Xmx4g -jar $GATK \
      -T GenotypeGVCFs \
      -R $ref_fasta \
      --variant ${input_gvcf_file}.gz \
      -o $output_vcf &
task_pid=$!
wait "$task_pid"
if [ "$?" -ne "0" ]; then
  log_error "GATK GenotypeGVCFs failed"
  rm ${input_gvcf_file}.pid -f
  exit 1;
fi

rm ${input_gvcf_file}.pid -f
