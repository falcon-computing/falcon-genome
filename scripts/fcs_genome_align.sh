################################################################################
## This script generates sorted bam file from fastq using bwa-flow
################################################################################
#!/bin/bash

# Import global variables
SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $SCRIPT_DIR/globals.sh

# Get the input command 
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -f|--fastq)
    fastq_base="$2"
    shift # past argument
    ;;
    -o|--output)
    output="$2"
    shift # past argument
    ;;
    -t|--temp)
    tmp_dir="$2"
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
  echo  " USAGE: fcs_genome align -f <fastq_basename> -o <output> -t <temp_dir>"
  echo  " The <fastq_basename> argument is necessary for the script to run, should not \
contain suffixes and should be put in $fastq_dir in default."
  echo  " The <output> argument is the file to store the sorted bam, could be empty."
  echo  " The <temp_dir> argument is the directory to store the intermediate results, could be empty."
  exit 1;
fi

if [ -z $fastq_base ];then
  echo "The <fastq_basename> argument is nessary for the script to run, should not contain suffixes"
  echo "You should use -f <fastq_baseneme> to pass in the fastq basename"
  exit 1
fi

if [ -z $output ];then
  create_dir ${tmp_dir[1]}
  output=${tmp_dir[1]}/${fastq_base}.bam
  echo "Output file is not set, the output file is stored to "$output" as default"
  echo "If you want to set it, use the -o <output> option "
fi

if [ -z $tmp_dir ];then
  create_dir ${tmp_dir[2]}
  tmp_dir=${tmp_dir[2]}
  echo "Temp directory is not set, the intermediate files are stored to $tmp_dir as default"
  echo "If you want to set it, use the -t <temp_dir> option"
fi


# Find the input fastqs
fastq1_base=$fastq_dir/${fastq_base}_1  
fastq2_base=$fastq_dir/${fastq_base}_2
echo "Finding fastq files in $fastq_dir"
echo
if [ -f "${fastq1_base}.fastq" -a -f "${fastq2_base}.fastq" ];then
  fastq1=${fastq1_base}.fastq
  fastq2=${fastq2_base}.fastq 
  echo "Input file found,using $fastq1 and $fastq2 as input"
elif [ -f "${fastq1_base}.fastq.gz" -a -f "${fastq2_base}.fastq.gz" ];then
  fastq1=${fastq1_base}.fastq.gz
  fastq2=${fastq2_base}.fastq.gz 
  echo "Input file found,using $fastq1 and $fastq2 as input"
elif [ -f "${fastq1_base}.fq" -a -f "${fastq2_base}.fq" ];then
  fastq1=${fastq1_base}.fq
  fastq2=${fastq2_base}.fq
  echo "Input file found,using $fastq1 and $fastq2 as input"
else
  echo "Cannot find input file in $fastq_dir, please ensure that you have put the fastq input at $fastq_dir"
  exit -1
fi
  
bwa_sort=1

check_input $fastq1
check_input $fastq2
check_output $output
check_output_dir $tmp_dir

bwa_log_dir=$log_dir/bwa
create_dir $log_dir
create_dir $bwa_log_dir
check_output_dir $bwa_log_dir
output_parts_dir=$tmp_dir/$(basename $output).parts

# Use pseudo input for header
# TODO(yaoh) maybe let the below arguments flexible to user
sample_id=SEQ01
RG_ID=SEQ01
platform=ILLUMINA
library=HUMsgR2AQDCAAPE

if [ "$bwa_sort" -gt 0 ]; then
  ext_options="--sort --max_num_records=2000000"
else
  ext_options=
fi

echo "Started BWA alignment"
start_ts=$(date +%s)
$BWA mem -M \
    -R "@RG\tID:$RG_ID\tSM:$sample_id\tPL:$platform\tLB:$library" \
    --log_dir=$bwa_log_dir/ \
    --output_dir=$output_parts_dir \
    $ext_options \
    $ref_genome \
    $fastq1 \
    $fastq2 \
    2>$bwa_log_dir/bwa_run.log

if [ "$?" -ne 0 ]; then 
  echo "BWAMEM failed,please check $bwa_log_dir/bwa_run.log for detailed information"
  exit 1
fi
end_ts=$(date +%s)
echo "BWA mem finishes in $((end_ts - start_ts))s"

# Increase the max number of files that can be opened concurrently
ulimit -n 2048

sort_files=$(find $output_dir -name part-* 2>/dev/null)
if [[ -z "$sort_files" ]]; then
  echo "Folder $output_dir is empty"
  exit 1
fi

start_ts=$(date +%s)
if [ "$bwa_sort" -gt 0 ]; then
  $SAMTOOLS merge -r -c -p -l 1 -@ 10 ${output} $sort_files 2>$bwa_log_dir/samtool_run.log
else
  cat $sort_files | $SAMTOOLS sort -m 16g -@ 10 -l 0 -o $output 2>$bwa_log_dir/samtool_run.log
fi

if [ "$?" -ne 0 ]; then 
  echo "Sorting failed,please check $bwa_log_dir/samtool_run.log for detailed information"
  exit 1
fi

# Remove the partial files
rm -r $output_parts_dir &

end_ts=$(date +%s)
echo "Samtools sort for $(basename $output) finishes in $((end_ts - start_ts))s"
