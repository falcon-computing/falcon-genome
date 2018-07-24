#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

if [ -z "$FALCON_DIR" ]; then
  FALCON_DIR=/curr/niveda/falcon-genome/
fi

ref_dir=/pool/local/ref/
ref_genome=$ref_dir/human_g1k_v37.fasta
db138_SNPs=$ref_dir/dbsnp_138.b37.vcf
g1000_indels=$ref_dir/1000G_phase1.indels.b37.vcf
g1000_gold_standard_indels=$ref_dir/Mills_and_1000G_gold_standard.indels.b37.vcf
geneList=/pool/storage/niveda/Depth/newsort_chr1.txt

workdir=${DIR}/work_dir/
mkdir -p $workdir
baseline=/pool/storage/niveda/Depth/originalGATK/
input_bam=/pool/storage/niveda/Results_validation/GATK-3.8/WES/

data_list=data.list

function compare_depthDiff {

local depth_baseline=$1;
local id=$2;
threshold=0.1;
equal=0.0;
ret_val=0;

baseline=$(head -n2 $depth_baseline | tail -n1);
subject=$(tail -n1 $workdir/${id}/${id}.sample_summary);

IFS=$'\t' b_array=($baseline);
IFS=$'\t' s_array=($subject);
#b_array=$(cut -d $baseline);
#s_array=$(cut -d '\t' $subject);
#b_array=( $(cat $depth_baseline | awk '{print $2}') )
#s_array=( $(cat $workdir/${id}/${id}.sample_summary | awk '{print $2}') )
for idx in ${!b_array[*]}; do
#for (( idx=1; idx<${#b_array[@]}; idx++)); do
  if [ $idx -ne 0 ]; then
  DIFF=$(echo "${b_array[$idx]} ${s_array[$idx]}" | awk '{print ($1 - $2)}');
  #DIFF=$(( ${b_array[$idx]} - ${s_array[$idx]} ))
  
  if [ DIFF ]; then
    equal=$(awk -v dividend="$DIFF" -v divisor="${b_array[$idx]}" 'BEGIN {printf "%.2f", sqrt((dividend/divisor)^2); exit(0)}')
    if (( $(echo "$equal $threshold" | awk '{print ($1 >= $2)}') )); then 
      ret_val=1
    fi
  fi
fi
done;

if [ $ret_val -ne "0" ]; then
  echo "Failed Depth compare"
  return 1
else
  return 0
fi;
}
