################################################################################
## This script generates sorted bam file from fastq using bwa-flow
################################################################################
#!/bin/bash

# Import global variables
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

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
  echo "USAGE: "
  echo "fcs_genome baseRecal \\"
  echo "    -r <ref.fasta> \\"
  echo "    -i <input.bam> \\"
  echo "    -knownSites <site.vcf> \\"
  echo "    -o <output.rpt>" 
  echo "<input.bam> argument is the markduped bam file";
  echo "<output.rpt> argument is the output BQSR results";
}

if [ $# -lt 1 ]; then
  print_help
  exit 1;
fi

declare -A knownSites

ks_index=0
while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
  -r|--ref)
    ref_fasta="$2"
    shift # past argument
    ;;
  -i|--input)
    input="$2"
    shift # past argument
    ;;
  -ks|-knownSites)
    knownSites["$ks_index"]="$2"
    # knownSites["$ks_index"] is ${knownSites[$ks_index]}"
    ks_index=$[$ks_index+1]
    shift # past argument
    ;;
  -o|--output)
    output_rpt="$2"
    shift # past argument
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

# Check the command
if [ ! -z $help_req ]; then
  print_help
  exit 0;
fi

check_arg "-r" "ref_fasta" "$ref_genome"
check_arg "-i" "$input"
check_arg "-o" "output_rpt" "$output_dir/rpt/$(basename $input).recal.rpt"

if [ -z ${knownSites[0]} ]; then
  knownSites_string="-knownSites $g1000_indels \
                     -knownSites $g1000_gold_standard_indels \
                     -knownSites $db138_SNPs"
  log_warn "The knownSites are not specified, by default the following reference "
  log_warn "will be used:"
  log_warn "- $g1000_indels"
  log_warn "- $g1000_gold_standard_indels"
  log_warn "- $db138_SNPs"
  log_warn "You can use the -ks|-knownSites option to chose different ones."
fi

for ks in ${knownSites[@]}; do
   knownSites_string="$knownSites_string -knownSites $ks"  
done
log_debug "$knownSites_string"

bqsr_log_dir=$log_dir/bqsr
rpt_dir=$(dirname $output_rpt)

create_dir $log_dir
create_dir $rpt_dir
create_dir $bqsr_log_dir

check_input $input
check_output $output_rpt

# Clear the done files to recover
if [ -z $force_flag ]; then
  rm -f $rpt_dir/.${output_rpt}*
  rm -f $rpt_dir/${output_rpt}.*
  rm -rf .queue 
fi

# Table storing all the pids for tasks within one stage
declare -A pid_table

# Start the index option
if [ ! -f ${input}.bai ]; then
  log_info "Start the samtools index"

  start_ts=$(date +%s)
  $SAMTOOLS index $input 
  end_ts=$(date +%s)

  log_info "Samtools index for $(basename $input) finishes in $((end_ts - start_ts))s"
fi

# Start manager
start_manager

export PATH=$DIR:$PATH

start_ts=$(date +%s)

case $verbose in
    0|1|2)
    # Put all the information to log, not displaying
    $JAVA -Djava.io.tmpdir=/tmp -jar ${GATK_QUEUE} \
    -S $DIR/BaseRecalQueue.scala \
    -R $ref_fasta \
    -I $input \
    $knownSites_string \
    -o $output_rpt \
    -jobRunner ParallelShell \
    -maxConcurrentRun 32 \
    -scatterCount 32 \
    -run
    > $bqsr_log_dir/bqsr_run.log 2> $bqsr_log_dir/bqsr_run_err.log 
    ;;
   # TODO(yaoh) add the remain verbose options after figure out how the bqsr print information 
esac

# Stop manager
stop_manager

end_ts=$(date +%s);
echo "Base Recalibration for $(basename $output_rpt) finishes in $((end_ts - start_ts))s"
