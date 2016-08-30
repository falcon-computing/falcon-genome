#############################################################################################
## This script generates gVCF files from a specified in a input file 
## (c) Di Wu 04/07/2016
#############################################################################################
#!/bin/bash

# Global variables
fastq_dir=/merlin_fs/merlin1/hdd1/diwu/fastq
output_dir=$(pwd)
tmp_dir=/pool/ssd1/$USER/temp

ref_dir=/space/scratch/genome/ref
ref_genome=$ref_dir/factor4/human_g1k_v37.fasta
db138_SNPs=$ref_dir/dbsnp_138.b37.vcf
g1000_indels=$ref_dir/1000G_phase1.indels.b37.vcf
g1000_gold_standard_indels=$ref_dir/Mills_and_1000G_gold_standard.indels.b37.vcf

if [[ $# -lt 1 ]]; then
  echo "USAGE: $0 <sample id> [1..5]"
  exit 1;
fi

# dictionary to enable selective stages
declare -A do_stage
do_stage=( ["1"]="1" ["2"]="1" ["3"]="1" ["4"]="1" ["5"]="1" ["6"]="1" ["7"]="1" )

reset=1
force_clean=0

while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
  -d|--input_dir)
    fastq_dir="$2"
    shift # past argument
    ;;
  -s|--sample_id)
    sample_id="$2"
    shift # past argument
    ;;
  -o|--output_dir)
    output_dir="$2"
    shift # past argument
    ;;
  -c|--checkpoint)
    checkpoint=1
    ;;
  -f|--force_clean)
    force_clean=1
    ;;
  1|2|3|4|5|6|7) # selective stage
    if [ $reset -eq 1 ]; then
      do_stage=( ["1"]="0" ["2"]="0" ["3"]="0" ["4"]="0" ["5"]="0" ["6"]="0" ["7"]="0" )
      reset=0
    fi
    do_stage["$key"]=1
    ;;
  *) # unknown option
    ;;
  esac
  shift # past argument or value
done

if [ -z "$sample_id" ]; then
  echo "Must specify a sample id"
  exit 1
fi

# Check input data
if [[ "${do_stage["1"]}" == "1" ]]; then
  basename=$fastq_dir/${sample_id}_1
  if [ ! -f "${basename}.fastq" ] && [ ! -f "${basename}.fastq.gz" ] && [ ! -f "${basename}.fq" ]; then
    echo "Cannot find input file in $fastq_dir" 
    exit -1
  fi
fi

# Check if fcs-genome is available
which fcs-genome &> /dev/null
if [ "$?" -gt 0 ]; then
  echo "Cannot find 'fcs-genome' in PATH"
  exit -1
fi

# Preparation of output directories
log_dir=$output_dir/log
bam_dir=$output_dir

# Preparation of output directories
mkdir -p $log_dir
mkdir -p $bam_dir
mkdir -p $tmp_dir

start_ts=$(date +%s)

# Step 1: BWA alignment and sort
if [[ "${do_stage["1"]}" == "1" ]]; then

  fastq_1=$fastq_dir/${sample_id}_1.fastq
  fastq_2=$fastq_dir/${sample_id}_2.fastq
  if [ ! -f $fastq_1 ]; then
    fastq_1=$fastq_dir/${sample_id}_1.fastq.gz
    fastq_2=$fastq_dir/${sample_id}_2.fastq.gz
  fi
  if [ ! -f $fastq_1 ]; then
    fastq_1=$fastq_dir/${sample_id}_1.fq
    fastq_2=$fastq_dir/${sample_id}_2.fq
  fi

  # Use these pseudo information for testing purpose
  SP_id=$sample_id
  RG_ID=$sample_id
  platform=ILLUMINA
  library=1

  output=${tmp_dir}/${sample_id}.bam

  fcs-genome align \
    -r $ref_genome \
    -fq1 $fastq_1 \
    -fq2 $fastq_2 \
    -sp $SP_id \
    -rg $RG_ID \
    -pl $platform \
    -lb $library \
    -o $output \
    -v 2 -f

  if [ "$?" -ne 0 ]; then
    echo "Alignment failed"
    exit 1;
  fi

  # checkpoint
  if [ -z "$checkpoint" ]; then
    cp $output $bam_dir &
  fi
fi

# Step 2: GATK Indel Realigner
# - Input: ${sample_id}.markdups.bam
# - Output: ${sample_id}.realn.bam
if [[ "${do_stage["2"]}" == "1" ]]; then

  input_bam=$tmp_dir/${sample_id}.bam
  intervals=${tmp_dir}/${sample_id}.intervals
  output=${tmp_dir}/${sample_id}.realn.bam

  fcs-genome rtc \
    -r $ref_genome \
    -i $input_bam \
    -o $intervals \
    -v -f

  if [ "$?" -ne 0 ]; then
    echo "RealignTargetCreator failed"
    exit 3;
  fi

  fcs-genome ir \
    -r $ref_genome \
    -i $input_bam \
    -rtc $intervals \
    -o $output \
    -v -f

  if [ ! -z "$checkpoint" ]; then
    cp -r $output $output_dir/bam &
  fi

  if [ "$force_clean" -eq 1 ]; then
    rm -f $input_bam
    rm -f $input_target
  fi
fi

# Step 3: GATK baseRecalibrator
# - Input: ${sample_id}.realn.bam
# - Output_dir ${sample_id}.recal.rpt
if [[ "${do_stage["3"]}" == "1" ]]; then
  input_bam=${tmp_dir}/${sample_id}.realn.bam
  output_rpt=${tmp_dir}/${sample_id}.recal.rpt
  
  fcs-genome bqsr \
    -r $ref_genome \
    -i $input_bam \
    -o $output_rpt \
    -v -f

  if [ "$?" -ne 0 ]; then
    echo "BaseRecalibrator failed"
    exit 5;
  fi
fi

# Step 4: GATK PrintRead
# - Input: ${sample_id}.realigned.bam
# - Ouput: per partition recaled bam 
if [[ "${do_stage["4"]}" == "1" ]]; then

  input_bam=${tmp_dir}/${sample_id}.realn.bam
  input_bqsr=${tmp_dir}/${sample_id}.recal.rpt
  pr_output_dir=${tmp_dir}/${sample_id}.recal.bam

  fcs-genome printReads \
    -r $ref_genome \
    -i $input_bam \
    -bqsr $input_bqsr \
    -o $pr_output_dir \
    -v -f

  if [ "$?" -ne 0 ]; then
    echo "Print Reads failed"
    exit 6;
  fi

  if [ "$force_clean" -eq 1 ]; then
    rm -rf $input_bam
    rm -rf $input_bqsr
  fi

  if [ ! -z "$checkpoint" ]; then
    cp -r $pr_output_dir $output_dir/bam &
  fi
fi

# Step 5: GATK unifiedGenotyper
# - Input: per partition recaled bam
# - Output: per partition gvcf
if [[ "${do_stage["5"]}" == "1" ]]; then

  input_dir=$tmp_dir/${sample_id}.recal.bam
  output_vcf_dir=$tmp_dir/${sample_id}.vcf

  fcs-genome ug \
    -r $ref_genome \
    -i $input_dir \
    -o $output_vcf_dir \
    -v -f

  if [ "$?" -ne 0 ]; then
    echo "UnifiedGenotyper failed"
    exit 7;
  fi

  if [ "$force_clean" -eq 1 ]; then
    rm -r $input_dir
  fi

  fcs-genome concat \
    -i $output_vcf_dir \
    -o $output_dir \
    -v 

  if [ "$?" -ne 0 ]; then
    echo "Concat vcf failed"
    exit 7;
  fi

  if [ "$force_clean" -eq 1 ]; then
    rm -r $output_vcf_dir
  fi
fi
end_ts=$(date +%s)
echo "Pipeline finishes in $((end_ts - start_ts))s"
