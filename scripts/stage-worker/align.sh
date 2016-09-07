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
  echo "<input_1.fastq> and <input_2.fastq> are the input fastq file";
  echo "<output.bam> argument is the file to put the output bam results";
  echo "<readgroup_id> <sample_id> <platform_id> <library_id> specifies read group information";
}

# check the command 
if [ $# -lt 2 ];then
  print_help
  exit 1;
fi

do_markdup=1

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
  --align-only)
    do_markdup=0
    ;;
  -rg|--readgroup_id)
    read_group="$2"
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
    if [ "$2" -eq "$2" 2> /dev/null ]; then
      # user specified an integer as input
      verbose="$2"
      shift
    else
      verbose=2
    fi
    ;;
  -f|--force)
    force_flag=YES
    ;;
  -h|--help)
    help_req=YES
    ;;
    *)
    # unknown option
    log_error "Failed to recongize argument '$1'"
    print_help
    exit 1
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
check_arg "-rg" "read_group"
check_arg "-sp" "sample_id"
check_arg "-pl" "platform"
check_arg "-lb" "library"
check_args 

#fastq_base_withsuffix=$(basename $fastq1)
#fastq_base=${fastq_base_withsuffix%_1.*} 
#log_debug "$fastq_base is the fastq base name"

if [ "$do_markdup" -eq 1 ]; then
check_arg "-o" "output" "${tmp_dir[1]}/${read_group}.bam"
fi
check_arg "-r" "ref_fasta" "$ref_genome"

tmp_dir=${tmp_dir[2]}
log_debug "The intermediate files of BWA alignment are stored to $tmp_dir"

# Get absolute filepath for input/outputs
readlink_check ref_fasta
readlink_check fastq1
readlink_check fastq2
readlink_check output
readlink_check log_dir

# Check input and output
check_input $ref_fasta
check_input $fastq1
check_input $fastq2

if [ "$do_markdup" -eq 1 ]; then
  check_output $output
  output_parts_dir=$tmp_dir/$sample_id
  if [ -d $output_parts_dir ]; then
    rm -rf $output_parts_dir
    log_info "Temp folder $output_parts_dir already exist, removing it."
  fi
else
  output_dir=$output
  create_dir $output

  output_parts_dir=$output_dir/$read_group
  check_output $output_parts_dir
fi

# Create the directories of the run
create_dir $log_dir
check_output_dir $log_dir

log_info "Started BWA alignment"
start_ts=$(date +%s)
start_ts_total=$start_ts

# Put all the information to log, not displaying
$BWA mem -M \
    -R "@RG\tID:$read_group\tSM:$sample_id\tPL:$platform\tLB:$library" \
    --logtostderr=1 \
    --output_dir=$output_parts_dir \
    --sort \
    --max_num_records=10000000 \
    $ref_fasta \
    $fastq1 \
    $fastq2 \
    &> $log_dir/bwa.log

if [ "$?" -ne 0 ]; then 
  log_error "BWAMEM failed, please check $log_dir/bwa.log for details"
  exit 1
fi
end_ts=$(date +%s)
log_info "bwa mem finishes in $((end_ts - start_ts)) seconds"

if [ "$do_markdup" -eq 1 ]; then
  # Increase the max number of files that can be opened concurrently
  ulimit -n 4096
  
  sort_files=$(find $output_parts_dir -name part-* 2>/dev/null)
  if [[ -z "$sort_files" ]]; then
    log_error "Folder $output_parts_dir is empty, could not start sorting"
    exit 1
  fi
  
  log_info "Start mark duplicates"
  start_ts=$(date +%s)
  $SAMBAMBA markdup -l 1 -t 16 $sort_files $output 2> $log_dir/markdup.log
  
  if [ "$?" -ne 0 ]; then 
    log_error "Sorting failed, please check $log_dir/markdup.log for detailed information"
    exit 1
  fi
  end_ts=$(date +%s)
  log_info "Markdup finishes in $((end_ts - start_ts)) seconds"
  
  # Remove the partial files
  rm -r $output_parts_dir

  log_info "Stage finishes in $((end_ts - start_ts_total)) seconds"
fi
