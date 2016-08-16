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
  if [[ $pass_args != YES && $1 != gatk ]];then
    echo "USAGE:"
    echo "fcs-genome hptc|haplotypeCaller \\";
    echo "    -r <ref.fasta> \\";
    echo "    -i <input dir or input bam> \\ ";
    echo "    -o <vcf_dir>";
    echo 
    echo "The -i input could be a directory or a single file"
    echo "<vcf_dir> argument is the directory to put the vcf output results";
  else
    $JAVA -d64 -jar $GATK -T HaplotypeCaller --help
  fi
}

if [ $# -lt 2 ]; then
  print_help $1
  exit 1;
fi

# Get the input command
while [[ $# -gt 0 ]];do
  key="$1"
  case $key in
  gatk)
    pass_args=YES
  ;;
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
    if [ -z $pass_args ];then
      log_error "Failed to recongize argument '$1'"
      print_help
      exit 1
    else
      additional_args="$additional_args $1"
    fi
  esac
  shift # past argument or value
done

# If no argument is given then print help message
if [ ! -z $help_req ];then
  print_help
  exit 0;
fi

declare -A contig_bam
declare -A contig_vcf
declare -A pid_table
declare -A output_table

vcf_dir_default=$output_dir/vcf
nparts=32
contig_list="$(seq 1 $nparts)"


# Check the input arguments
check_arg "-i" "input"
check_arg "-r" "ref_fasta" "$ref_genome"
check_arg "-o" "vcf_dir" "$vcf_dir_default"
check_args

# Infer the input_base
if [ -d "$input" ];then
  input_base_withsuffix=`ls $input/*.bam | sed -e 'N;s/^\(.*\).*\n\1.*$/\1\n\1/;D'`; 
  input_base_withsuffix=$(basename $input_base_withsuffix)
  input_base=${input_base_withsuffix%%.*}
else
  input_base_withsuffix=$(basename $input)
  input_base=${input_base_withsuffix%%.*} 
fi
log_debug "Sample id is $input_base"

# Get absolute filepath for input/output
readlink_check ref_fasta
readlink_check input
readlink_check vcf_dir

# Create the directorys
create_dir $log_dir
create_dir $vcf_dir

# Check the inputs
if [ -d "$input" ]; then
  # Input is a directory, checkout for per-contig data
  for contig in $contig_list; do
    contig_bam["$contig"]=${input}/${input_base}.contig${contig}.bam
    check_input ${contig_bam["$contig"]}
  done
else
  # Input is a single file, use if for all per-contig HTC
  check_input $input
  for contig in $contig_list; do
    contig_bam["$contig"]=$input
  done
fi

# Check the outputs
for contig in $contig_list; do
  contig_vcf["$contig"]=$vcf_dir/${input_base}.contig${contig}.gvcf
  check_output ${contig_vcf[$contig]}
done

# Start manager
start_manager

trap "terminate" 1 2 3 9 15

# Start the jobs
log_info "Start stage for input $input"
log_info "Output files will be put in $vcf_dir"
start_ts_total=$(date +%s)

for contig in $contig_list; do
  $DIR/../fcs-sh "$DIR/haploTC_contig.sh \
      $ref_fasta \
      $DIR/intv_lists_${nparts}/intv${contig}.list \
      ${contig_bam[$contig]} \
      ${contig_vcf[$contig]} \
      $contig \
      $additional_args" 2> $log_dir/haplotypeCaller_contig${contig}.log &

  pid_table["$contig"]=$!
  output_table["$contig"]=${contig_vcf[$contig]}
done

# Wait on all the tasks
log_file=$log_dir/haplotypeCaller.log
rm -f $log_file
is_error=0
for contig in $contig_list; do
  pid=${pid_table[$contig]}
  wait "${pid}"
  if [ "$?" -gt 0 ]; then
    is_error=1
    log_error "Stage failed on contig $contig"
  fi

  # Concat log and remove the individual ones
  contig_log=$log_dir/haplotypeCaller_contig${contig}.log
  cat $contig_log >> $log_file
  rm -f $contig_log
done
end_ts=$(date +%s)

# Stop manager
stop_manager

unset contig_bam
unset contig_vcf
unset pid_table
unset output_table

if [ "$is_error" -ne 0 ]; then
  log_error "Stage failed, please check logs in $log_file for details."
  exit 1
fi

log_info "Stage finishes in $((end_ts - start_ts_total))s"
