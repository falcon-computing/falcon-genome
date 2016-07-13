#############################################################################################
## This script generates gVCF files from a specified in a input file 
## (c) Di Wu 04/07/2016
#############################################################################################
#!/bin/bash

# Import global variables
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

if [[ $# -lt 1 ]]; then
  echo "USAGE: $0 <sample id> [1..5]"
  exit 1;
fi

sample_id=$1

shift

# dictionary to enable selective stages
declare -A do_stage
if [[ $# -gt "0" ]]; then
  do_stage=( ["1"]="0" ["2"]="0" ["3"]="0" ["4"]="0" ["5"]="0" )
  for i in $@; do 
    do_stage["$i"]=1
  done
else
  do_stage=( ["1"]="1" ["2"]="1" ["3"]="1" ["4"]="1" ["5"]="1" )
fi

# Check input data
if [[ $INPUT_DIR != "" ]]; then
  fastq_dir=$INPUT_DIR
fi
if [[ "${do_stage["1"]}" == "1" ]]; then
  basename=$fastq_dir/${sample_id}_1
  if [ -f "${basename}.fastq" -o -f "${basename}.fastq.gz" -o -f "${basename}.fq" ]; then
    echo "Input file found"
  else
    echo "Cannot find input file in $fastq_dir" 
    exit -1
  fi
fi

# Table storing all the pids for tasks within one stage
declare -A pid_table

# Start manager
if [ -e "host_file" ]; then
  host_file=host_file
else
  host_file=$DIR/host_file
fi
echo "$DIR/manager/manager --v=1 --log_dir=. $host_file"
source $DIR/manager/config.mk
$DIR/manager/manager --v=1 --log_dir=. $host_file &
manager_pid=$!
sleep 1
if [[ ! $(ps -p "$manager_pid" -o comm=) =~ "manager" ]]; then
  echo "Cannot start manager, exiting"
  exit -1
fi

# Preparation of output directories
create_dir $log_dir
create_dir ${tmp_dir[1]}
create_dir ${tmp_dir[2]}

chkpt_pid=

# Step 1: BWA alignment and sort
if [[ "${do_stage["1"]}" == "1" ]]; then

  create_dir $bam_dir
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

  # Put output in tmp_dir[1]
  output=${tmp_dir[1]}/${sample_id}.bam
  $DIR/align.sh $fastq_1 $fastq_2 $output ${tmp_dir[2]}

  if [ "$?" -ne 0 ]; then
    echo "Alignment failed"
    exit 1;
  fi

  # checkpoint
  cp $output $bam_dir &
fi

# Step 2: Mark Duplicate
# - Input: sorted ${sample_id}.bam
# - Output: duplicates-removed ${sample_id}.markdups.bam
if [[ "${do_stage["2"]}" == "1" ]]; then
  start_ts=$(date +%s)
  input=${tmp_dir[1]}/${sample_id}.bam
  output=${tmp_dir[2]}/${sample_id}.markdups.bam

  $DIR/markDup.sh $input $output ${tmp_dir[1]} 2> >(tee $log_dir/markDup.log >&2)

  if [ "$?" -ne 0 ]; then
    echo "MarkDuplicate failed"
    exit 2;
  fi

  end_ts=$(date +%s)
  echo "MarkDuplicate for $(basename $input) finishes in $((end_ts - start_ts))s"
  
  # Delete sorted bam file
  rm $input &

  # Checkpoint markdup bam file
  cp $output $bam_dir &
  cp ${output}.dups_stats $bam_dir &
fi

# Step 3: GATK BaseRecalibrate
# - Input: ${sample_id}.markdups.bam
# - Output: recalibrated ${sample_id}.markdups.recal.bam
if [[ "${do_stage["3"]}" == "1" ]]; then
  input=${tmp_dir[2]}/${sample_id}.markdups.bam
  if [ ! -f $input ]; then
    echo "Cannot find $input"
    exit 1
  fi

  start_ts=$(date +%s)
  # Indexing input if it is not indexed
  if [ ! -f ${input}.bai ]; then
    $SAMTOOLS index $input 
  fi
  end_ts=$(date +%s)
  cp ${input}.bai $bam_dir &
  echo "Samtools index for $(basename $input) finishes in $((end_ts - start_ts))s"

  rpt_dir=$output_dir/rpt
  create_dir $rpt_dir

  start_ts=$(date +%s)
  output=$rpt_dir/${sample_id}.recalibration_report.grp

  $DIR/baseRecal.sh $input $output 2> >(tee $log_dir/baseRecal.log >&2)
  if [ "$?" -ne 0 ]; then
    echo "BaseRecalibrator failed"
    exit 3;
  fi
  end_ts=$(date +%s)

  echo "BaseRecalibrator for $(basename $input) finishes in $((end_ts - start_ts))s"

  if [[ "$chkpt_pid" != "" ]]; then
    wait $chkpt_pid
  fi
fi

# Step 4: GATK PrintRead
# - Input: ${sample_id}.markdups.recal.chr${chr}.bam
# - Output: per chromosome calibrated bam ${sample_id}_chr${chr}.bam
if [[ "${do_stage["4"]}" == "1" ]]; then
  start_ts=$(date +%s)

  rpt_dir=$output_dir/rpt
  chr_list="$(seq 1 22) X Y MT"
  for chr in $chr_list; do
    # The splited bams are in tmp_dir[1], the recalibrated bams should be in [2]
    chr_bam=${tmp_dir[1]}/${sample_id}.markdups.bam
    chr_rpt=$rpt_dir/${sample_id}.recalibration_report.grp
    chr_recal_bam=${tmp_dir[2]}/${sample_id}.recal.chr${chr}.bam

    $DIR/fcs-sh "$DIR/printReads.sh $chr $chr_bam $chr_rpt $chr_recal_bam" 2> $log_dir/printReads_chr${chr}.log &
    pid_table["$chr"]=$!
  done
  # Wait on all the tasks
  for pid in ${pid_table[@]}; do
    wait "${pid}"
  done
  end_ts=$(date +%s)

  for chr in $chr_list; do
    chr_bam=${tmp_dir[1]}/${sample_id}.markdups.bam
    chr_recal_bam=${tmp_dir[2]}/${sample_id}.recal.chr${chr}.bam
    if [ ! -e ${chr_recal_bam}.done ]; then
      exit 4;
    fi
    rm ${chr_recal_bam}.done

    # Check point for PrintReads
    cp $chr_recal_bam $bam_dir &
    cp ${chr_recal_bam%.bam}.bai $bam_dir &
  done
  echo "PrintReads stage finishes in $((end_ts - start_ts))s"

  # Remove temp output for MarkDup
  rm $input &
  rm ${input}.bai &
  rm ${input}.dups_stats &
fi

# Step 5: GATK HaplotypeCaller
# - Input: ${sample_id}.markdups.recal.bam
# - Output: per chromosome varients ${sample_id}_$chr.gvcf
if [[ "${do_stage["5"]}" == "1" ]]; then
  chr_list="$(seq 1 22) X Y MT"
  vcf_dir=$output_dir/vcf
  create_dir $vcf_dir

  start_ts=$(date +%s)
  for chr in $chr_list; do
    chr_bam=${tmp_dir[2]}/${sample_id}.recal.chr${chr}.bam
    chr_vcf=$vcf_dir/${sample_id}_chr${chr}.gvcf
    $DIR/fcs-sh "$DIR/haploTC.sh $chr $chr_bam $chr_vcf" 2> $log_dir/haplotypeCaller_chr${chr}.log &
    pid_table["$chr"]=$!
  done

  # Wait on all the tasks
  for pid in ${pid_table[@]}; do
    wait "${pid}"
  done
  end_ts=$(date +%s)

  for chr in $chr_list; do
    chr_bam=${tmp_dir[2]}/${sample_id}.recal.chr${chr}.bam
    chr_vcf=$vcf_dir/${sample_id}_chr${chr}.gvcf
    if [ ! -e ${chr_vcf}.done ]; then
      exit 5;
    fi
    rm ${chr_vcf}.done
    rm $chr_bam 
    rm ${chr_bam%.bam}.bai
    # Note, the output of samtool index for $chr_bam is not 
    # ${chr_bam}.bai because it is generated by GATK
    # so 'rm ${chr_bam}.bai' is wrong
  done
  echo "HaplotypeCaller stage finishes in $((end_ts - start_ts))s"
fi

# Stop manager
kill $manager_pid
