################################################################################
## This script generates sorted bam file from fastq using bwa-flow
################################################################################
#!/bin/bash

# Import global variables
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

stage_name=align
# Prevent this script to be running alone
if [[ $0 != ${BASH_SOURCE[0]} ]]; then
  # Script is sourced by another shell
  cmd_name=`basename $0 2> /dev/null`
  if [[ "$cmd_name" != "fcs-genome" ]]; then
    log_error "This script should be started by 'fcs-genome'"
    return 1
  fi
else
  # Script is executed directly
  log_error "This script should be started by 'fcs-genome'"
  exit 1
fi

print_help() {
  echo "USAGE:"
  echo "fcs-genome al|align \\";
  echo "    -r <ref.fasta> \\";
  echo "    -fq1 <input_1.fastq> \\ ";
  echo "    -fq2 <input_2.fastq> \\ ";
  echo "    -o <output.bam> \\";
  echo "    -rg <readgroup_id> \\ ";
  echo "    -sp <sample_id> \\ "
  echo "    -pl <platform_id> \\ "
  echo "    -lb <library_id>"
  echo 
  echo "<input.bam> argument is the markduped bam file";
  echo "<input_1.fastq> and <input_2.fastq> are the input fastq file";
  echo "<output.bam> argument is the file to put the output bam results";
  echo "<readgroup_id> <sample_id> <platform_id> <library_id> specifies read group information";
}

# check the command 
if [ $# -lt 2 ];then
  print_help
  exit 0;
fi

# Get the input command 
while [[ $# -gt 0 ]];do
  key="$1"
  case $key in
  -r|--ref)
    ref_fasta="$2"
    shift
    ;;
  -fq1|--fastq1)
    fastq1="$2"
    shift
    ;;
  -fq2|--fastq2)
    fastq2="$2"
    shift
    ;;
  -o|--output)
    output="$2"
    shift
    ;;
  -rg|--readgroup_id)
    RG_ID="$2"
    shift
    ;;
  -sp|--sample_id)
    sample_id="$2"
    shift
    ;;
  -pl|--platform_id)
    platform="$2"
    shift
    ;;
  -lb|--library_id)
    library="$2"
    shift
    ;;
  -v|--verbose)
    verbose="$2"
    shift
    ;;
  -f|--force)
    force_flag=YES
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

# check the command 
if [ ! -z $help_req ];then
  print_help
  exit 0;
fi

# Check the input arguments
check_arg "-fq1" "fastq1"
check_arg "-fq2" "fastq2"
check_arg "-rg" "RG_ID"
check_arg "-sp" "sample_id"
check_arg "-pl" "platform"
check_arg "-lb" "library"
check_args 

fastq_base_withsuffix=$(basename $fastq1)
fastq_base=${fastq_base_withsuffix%_1.*} 
log_info "$fastq_base is the fastq base name"
output_default=${tmp_dir[1]}/$fastq_base.bam

check_arg "-o" "output" "$output_default"
check_arg "-r" "ref_fasta" "$ref_genome"

bwa_sort=1

tmp_dir=${tmp_dir[2]}
log_info "The intermediate files of BWA alignment are stored to $tmp_dir"

# Check input
check_input $ref_fasta
check_input $fastq1
check_input $fastq2

#check output
check_output $output
check_output_dir $tmp_dir

# Create the directories of the run

create_dir $log_dir
bwa_log_dir=$log_dir/bwa
create_dir $bwa_log_dir
check_output_dir $bwa_log_dir
output_parts_dir=$tmp_dir/$(basename $output).parts

# Use pseudo input for header

if [ "$bwa_sort" -gt 0 ]; then
  ext_options="--sort --max_num_records=2000000"
else
  ext_options=
fi

log_info "Started BWA alignment"
start_ts=$(date +%s)
start_ts_total=$start_ts

# Put all the information to log, not displaying
$BWA mem -M \
    -R "@RG\tID:$RG_ID\tSM:$sample_id\tPL:$platform\tLB:$library" \
    --log_dir=$bwa_log_dir/ \
    --output_dir=$output_parts_dir \
    $ext_options \
    $ref_fasta \
    $fastq1 \
    $fastq2 \
    2> $bwa_log_dir/bwa_run_err.log 

if [ "$?" -ne 0 ]; then 
  log_error "BWAMEM failed, please check $bwa_log_dir/bwa_run_err.log for details"
  exit 1
fi
end_ts=$(date +%s)
log_info "BWA mem finishes in $((end_ts - start_ts))s"

# Increase the max number of files that can be opened concurrently
ulimit -n 2048

sort_files=$(find $output_parts_dir -name part-* 2>/dev/null)
if [[ -z "$sort_files" ]]; then
  log_error "Folder $output_parts_dir is empty, could not start sorting"
  exit 1
fi

log_info "Start sorting"
start_ts=$(date +%s)
if [ "$bwa_sort" -gt 0 ]; then
  $SAMTOOLS merge -r -c -p -l 1 -@ 10 ${output} $sort_files -f 2> $bwa_log_dir/samtool_run.log
else
  cat $sort_files | $SAMTOOLS sort -m 16g -@ 10 -l 0 -o $output 2> $bwa_log_dir/samtool_run.log
fi

if [ "$?" -ne 0 ]; then 
  log_error "Sorting failed, please check $bwa_log_dir/samtool_run.log for detailed information"
  exit 1
fi

# Remove the partial files
rm -r $output_parts_dir &

end_ts=$(date +%s)
log_info "Samtools sort finishes in $((end_ts - start_ts))s"
log_info "Stage finishes in $((end_ts - start_ts_total))s"
