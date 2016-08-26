#############################################################################################
## This script generates gVCF files from a specified in a input file 
## (c) Di Wu 04/07/2016
#############################################################################################
#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

fastq_dir=/merlin_fs/merlin1/hdd1/diwu/fastq
output_dir=$(pwd)
tmp_dir=/genome_fs/local/temp/$USER

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
do_stage=( ["1"]="1" ["2"]="0" ["3"]="1" ["4"]="1" ["5"]="1" )

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
  1|2|3|4|5) # selective stage
    if [ $reset -eq 1 ]; then
      do_stage=( ["1"]="0" ["2"]="0" ["3"]="0" ["4"]="0" ["5"]="0" )
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

mkdir -p $log_dir
mkdir -p $bam_dir
mkdir -p $tmp_dir

start_ts=$(date +%s)

if [ "$force_clean" -eq 1 ]; then
  echo "Will remove intermediate results"
fi

# Step 1: BWA alignment and sort
if [[ "${do_stage["1"]}" == "1" ]]; then

  rg_info_avail=1
  fastq_1=$fastq_dir/${sample_id}_1.fastq
  fastq_2=$fastq_dir/${sample_id}_2.fastq
  if [ ! -f $fastq_1 ]; then
    fastq_1=$fastq_dir/${sample_id}_1.fq
    fastq_2=$fastq_dir/${sample_id}_2.fq
  fi
  if [ ! -f $fastq_1 ]; then
    fastq_1=$fastq_dir/${sample_id}_1.fastq.gz
    fastq_2=$fastq_dir/${sample_id}_2.fastq.gz
    rg_info_avail=0
  fi

  if [ "$rg_info_avail" -lt 0 ]; then
    FlowCell=`head -1 $fastq_1 | cut -d : -f 1 | sed 's/@//g'`
    Lane=`head -1 $fastq_1 | cut -d : -f 2`

    SP_id=$sample_id
    RG_ID="${FlowCell}_L${Lane}"
    platform=ILLUMINA
    library=L1
  else
    # use pseudo header
    SP_id=$sample_id
    SP_id=$sample_id
    platform=ILLUMINA
    library=L1
  fi

  output=$tmp_dir/${sample_id}.bam
  output_stat=$output_dir/${sample_id}.bam.flagstat

  fcs-genome align \
    -fq1 $fastq_1 \
    -fq2 $fastq_2 \
    -sp $SP_id \
    -rg $RG_ID \
    -pl $platform \
    -lb $library \
    -v 2 \
    -o $output

  if [ "$?" -ne 0 ]; then
    echo "Alignment failed"
    exit 1;
  fi

  # remove input fastq
#  if [ "$force_clean" -eq 1 ]; then
#    rm -f $fastq_1 &
#    rm -f $fastq_2 &
#  fi

  # checkpoint
  if [ ! -z "$checkpoint" ]; then
    cp $output $bam_dir &
  fi
fi

# Step 2: Mark Duplicate
# - Input: sorted ${sample_id}.bam
# - Output: duplicates-removed ${sample_id}.markdups.bam
if [[ "${do_stage["2"]}" == "1" ]]; then

  input=$tmp_dir/${sample_id}.bam
  output=$tmp_dir/${sample_id}.markdups.bam

  fcs-genome markDup \
    -i $input \
    -o $output \
    -v -f

  if [ "$?" -ne 0 ]; then
    echo "MarkDuplicate failed"
    exit 2;
  fi

  # Delete sorted bam file
  if [ "$force_clean" -eq 1 ]; then
    rm $input &
  fi

  # Checkpoint markdup bam file
  if [ ! -z "$checkpoint" ]; then
    #cp $output $bam_dir &
    cp ${output}.dups_stats $bam_dir 2> /dev/null
  fi
fi

# Step 3: GATK BaseRecalibrate
# - Input: ${sample_id}.markdups.bam
# - Output: recalibrated ${sample_id}.markdups.recal.bam
if [[ "${do_stage["3"]}" == "1" ]]; then

  if [ ${do_stage["2"]} -eq 1 ]; then
    input=$tmp_dir/${sample_id}.markdups.bam
  else
    input=$tmp_dir/${sample_id}.bam
  fi

  if [ ! -f $input ]; then
    echo "Cannot find $input"
    exit 1
  fi
  output=$tmp_dir/${sample_id}.recalibration_report.grp

  fcs-genome baseRecal \
    -i $input \
    -o $output \
    -v -f

  if [ "$?" -ne 0 ]; then
    echo "BaseRecalibrator failed"
    exit 3;
  fi
fi

# Step 4: GATK PrintRead
# - Input: ${sample_id}.markdups.recal.chr${chr}.bam
# - Output: per chromosome calibrated bam ${sample_id}_chr${chr}.bam
if [[ "${do_stage["4"]}" == "1" ]]; then

  if [ ${do_stage["2"]} -eq 1 ]; then
    input_bam=$tmp_dir/${sample_id}.markdups.bam
  else
    input_bam=$tmp_dir/${sample_id}.bam
  fi
  input_bqsr=$tmp_dir/${sample_id}.recalibration_report.grp
  output=$tmp_dir/${sample_id}.recal.bam

  fcs-genome printReads \
    -i $input_bam \
    -bqsr $input_bqsr \
    -o $output \
    -v -f

  if [ "$?" -ne 0 ]; then
    echo "Print Reads failed"
    exit 4;
  fi

  if [ "$force_clean" -eq 1 ]; then
    rm -f $input_bam
    rm -f ${input_bam}.bai
    rm -f ${input_bam}.dups_stats
    rm -f $input_bqsr
  fi

  if [ ! -z "$checkpoint" ]; then
    cp -r $output $output_dir/bam &
  fi
fi

# Step 5: GATK HaplotypeCaller
# - Input: ${sample_id}.markdups.recal.bam
# - Output: per chromosome varients ${sample_id}_$chr.gvcf
if [[ "${do_stage["5"]}" == "1" ]]; then

  input_dir=$tmp_dir/${sample_id}.recal.bam
  vcf_dir=$tmp_dir/${sample_id}.vcf

  fcs-genome haplotypeCaller \
    -i $input_dir \
    -o $vcf_dir \
    -v -f

  if [ "$?" -ne 0 ]; then
    echo "Variant Calling failed"
    exit 5;
  fi

  if [ "$force_clean" -eq 1 ]; then
    rm -r $input_dir
  fi

  # combine and compress the gVCFs
  fcs-genome concat \
    -i $vcf_dir \
    -o $output_dir \
    -v

  if [ "$?" -ne 0 ]; then
    echo "Concat gVCFs failed"
    exit 5;
  fi

  if [ "$force_clean" -eq 1 ]; then
    rm -r $vcf_dir
  fi
fi
end_ts=$(date +%s)
echo "Pipeline finishes in $((end_ts - start_ts)) seconds"
