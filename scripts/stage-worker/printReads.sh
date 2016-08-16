################################################################################
## This script does the printreads stage
################################################################################
#!/usr/bin/env bash

# Import global variables
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

stage_name=printReads

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
    echo "fcs-genome printReads \\";
    echo "    -r <ref.fasta> \\";
    echo "    -i <input.bam> \\ ";
    echo "    -bqsr <calibrate.rpt> \\";
    echo "    -o <output_dir>";
    echo 
    echo "<input.bam> argument is the markduped bam file";
    echo "<calibrate.rpt> argument is the report file generated by bqsr stage";
    echo "<output_dir> argument is the directory to put the output results";
  else
    $JAVA -d64 -jar $GATK -T PrintReads --help
  fi
}

if [ $# -lt 2 ]; then
  print_help $1
  exit 1;
fi

# Get the input command
while [[ $# -gt 0 ]]; do
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
  -bqsr|-BQSR)
    bqsr_rpt="$2"
    shift # past argument
    ;;
  -o|--output)
    output="$2"
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
    ;;
  esac
  shift # past argument or value
done

# If no argument is given then print help message
if [ ! -z $help_req ]; then
  print_help
  exit 0;
fi

declare -A contig_recal_bam
declare -A pid_table
declare -A output_table

nparts=32
contig_list="$(seq 1 $nparts)"

# Check the input arguments
check_arg "-r" "ref_fasta" "$ref_genome"
check_arg "-i" "input"
check_arg "-bqsr" "bqsr_rpt"
check_args

input_base_withsuffix=$(basename $input)
input_base=${input_base_withsuffix%%.*} 
output_default=${tmp_dir[1]}/${input_base}
check_arg "-o" "output" "$output_default"

# Get absolute filepath for input/output
# This is necessary because the jobs can be run distributedly
readlink_check ref_fasta
readlink_check input
readlink_check output
readlink_check bqsr_rpt
readlink_check log_dir

create_dir $log_dir
create_dir $output

# Check input
check_input $ref_fasta
check_input $input
check_input $bqsr_rpt

# Check output
for contig in $contig_list; do
  contig_recal_bam["$contig"]=${output}/${input_base}.contig${contig}.bam
  check_output ${contig_recal_bam["$contig"]}
done

# Start manager
start_manager

trap "terminate" 1 2 3 9 15

# check if index already exists
if [ ! -f ${input}.bai ]; then
  log_warn "Cannot find index for $(basename $input), use samtool to index"
  start_ts=$(date +%s)
  $SAMTOOLS index $input
  end_ts=$(date +%s)
  log_info "Samtools index for $(basename $input) finishes in $((end_ts - start_ts))s"
fi

# Start the jobs
log_info "Start stage for input $input_base"
log_info "Output will be put in $output"
start_ts_total=$(date +%s)

for contig in $contig_list; do
  $DIR/../fcs-sh "$DIR/printReads_contig.sh \
      $ref_fasta \
      $DIR/intv_lists_${nparts}/intv${contig}.list \
      $input \
      $bqsr_rpt \
      ${contig_recal_bam["$contig"]} \
      $contig \
      $additional_args" 2> $log_dir/printReads_contig${contig}.log 1> /dev/null &

  pid_table["$contig"]=$!
  output_table["$contig"]=${contig_recal_bam[$contig]}
done

# Wait on all the tasks
log_file=$log_dir/printReads.log
rm -f $log_file
is_error=0
for contig in $contig_list; do
  pid=${pid_table[$contig]}
  wait "${pid}"
  if [ "$?" -gt 0 ]; then
    is_error=1
    log_error "Failed on contig $contig"
  fi

  # Concat log and remove the individual ones
  contig_log=$log_dir/printReads_contig${contig}.log
  cat $contig_log >> $log_file
  rm -f $log_dir/printReads_contig${contig}.log
done
end_ts=$(date +%s)

# Stop manager
stop_manager

unset contig_recal_bam
unset pid_table
unset output_table

if [ "$is_error" -ne 0 ]; then
  log_error "Stage failed, please check logs in $log_file for details."
  exit 1
fi

log_info "Stage finishes in $((end_ts - start_ts_total))s"
