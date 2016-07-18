################################################################################
## This script does the printreads stage
################################################################################
#!/usr/bin/env bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

stage_name=haplotypeCaller

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
  echo "fcs-genome hptc|haplotypeCaller \\";
  echo "    -r <ref.fasta> \\";
  echo "    -i <input dir or input bam> \\ ";
  echo "    -o <vcf_dir>";
  echo 
  echo "The -i input could be a directory or a single file"
  echo "<vcf_dir> argument is the directory to put the vcf output results";
}

if [ $# -lt 2 ]; then
  print_help
  exit 1;
fi

# Get the input command
while [[ $# -gt 0 ]];do
  key="$1"
  case $key in
  -r|--ref|-R)
    ref_fasta="$2"
    shift # past argument
    ;;
  -i|--input|-I)
    input="$2"
    shift # past argument
    ;;
  -o|--output|-O)
    vcf_dir="$2"
    shift # past argument
    ;;
  -v|--verbose)
    if [ $2 -eq $2 2> /dev/null ]; then
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
    ;;
  esac
  shift # past argument or value
done

# If no argument is given then print help message
if [ ! -z $help_req ];then
  print_help
  exit 0;
fi

declare -A chr_bam
declare -A chr_vcf
declare -A pid_table
declare -A output_table

vcf_dir_default=$output_dir/vcf
chr_list="$(seq 1 22) X Y MT"

log_debug "Sample id is $input_base"

# Check the input arguments
check_arg "-i" "input"
check_arg "-r" "ref_fasta" "$ref_genome"
check_arg "-o" "vcf_dir" "$vcf_dir_default"
check_args

input_base_withsuffix=$(basename $input)
input_base=${input_base_withsuffix%%.*} 

# Get absolute filepath for input/output
readlink_check ref_fasta
readlink_check input
readlink_check vcf_dir

# Create the directorys
create_dir $log_dir
create_dir $vcf_dir

# Check the inputs
if [ -d "$input" ]; then
  # Input is a directory, checkout for per-chr data
  for chr in $chr_list; do
    chr_bam["$chr"]=${input}/${input_base}.chr${chr}.bam
    check_input ${chr_bam["$chr"]}
  done
else
  # Input is a single file, use if for all per-chr HTC
  check_input $input
  for chr in $chr_list; do
    chr_bam["$chr"]=$input
  done
fi

# Check the outputs
for chr in $chr_list; do
  chr_vcf["$chr"]=$vcf_dir/${input_base}.chr${chr}.gvcf
  check_output ${chr_vcf[$chr]}
done

# Start manager
start_manager

trap "terminate" 1 2 3 15

# Start the jobs
log_info "Start stage for input $input"
log_info "Output files will be put in $vcf_dir"
start_ts_total=$(date +%s)

for chr in $chr_list; do
  $DIR/../fcs-sh "$DIR/haploTC_chr.sh \
      $ref_fasta \
      $chr \
      ${chr_bam[$chr]} \
      ${chr_vcf[$chr]} \
      $verbose" 2> $log_dir/haplotypeCaller_chr${chr}.log &

  pid_table["$chr"]=$!
  output_table["$chr"]=${chr_vcf[$chr]}
done

# Wait on all the tasks
log_file=$log_dir/haplotypeCaller.log
rm -f $log_file
is_error=0
for chr in $chr_list; do
  pid=${pid_table[$chr]}
  wait "${pid}"
  if [ "$?" -gt 0 ]; then
    is_error=1
    log_error "Stage failed on chromosome $chr"
  fi

  # Concat log and remove the individual ones
  chr_log=$log_dir/haplotypeCaller_chr${chr}.log
  cat $chr_log >> $log_file
  rm -f $chr_log
done
end_ts=$(date +%s)

#for chr in $chr_list; do
#  if [ ! -e ${chr_vcf[$chr]}.done ]; then
#    is_error=1
#  fi
#  rm ${chr_vcf[$chr]}.done
#done
 

# Stop manager
stop_manager

unset chr_bam
unset chr_vcf
unset pid_table
unset output_table

if [ "$is_error" -ne 0 ]; then
  log_error "Stage failed, please check logs in $log_file for details."
  exit 1
fi

log_info "Stage finishes in $((end_ts - start_ts_total))s"
