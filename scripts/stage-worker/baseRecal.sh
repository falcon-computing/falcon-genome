################################################################################
## This script does the Base Recalibration stage
################################################################################
#!/bin/bash

# Import global variables
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

stage_name=baseRecal

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
    echo "fcs-genome baseRecal \\"
    echo "    -r <ref.fasta> \\"
    echo "    -i <input.bam> \\"
    echo "    -knownSites <site.vcf> \\"
    echo "    -o <output.rpt>" 
    echo "<input.bam> argument is the markduped bam file";
    echo "<output.rpt> argument is the output BQSR results";
  else
    $JAVA  -d64 -jar $GATK -T BaseRecalibrator --help
  fi
}

if [ $# -lt 1 ]; then
  print_help $1
  exit 1;
elif [ $# -eq 1 -a $1 == gatk ];then
  print_help $1
  exit 1;
fi

declare -A knownSites

ks_index=0

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
  -ks|-knownSites)
    knownSites["$ks_index"]="$2"
    # knownSites["$ks_index"] is ${knownSites[$ks_index]}"
    ks_index=$[$ks_index+1]
    shift # past argument
    ;;
  -o|--output|-O)
    output_rpt="$2"
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

# Check the command
if [ ! -z $help_req ]; then
  print_help
  exit 0;
fi

check_arg "-i" "input"
check_args

check_arg "-r" "ref_fasta" "$ref_genome"
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

# Get absolute filepath for input/output
readlink_check ref_fasta
readlink_check input
readlink_check output_rpt
readlink_check log_dir

rpt_dir=$(dirname $output_rpt)


create_dir $log_dir
create_dir $rpt_dir

check_input $input
check_output $output_rpt

# Clear the done files to rerun
rpt_donefile=$rpt_dir/.$(basename $output_rpt).done
rpt_out_donefile=$rpt_dir/.$(basename $output_rpt).out.done
if [ -f $rpt_donefile ]; then
  rm -f $rpt_donefile
fi
if [ -f $rpt_out_donefile ]; then
  rm -f $rpt_out_donefile
fi
if [ -d .queue ]; then
  rm -rf .queue
fi

# Table storing all the pids for tasks within one stage
declare -A pid_table

# Start the index option
# usually the old bai would cause the bqsr error
if [ -f ${input}.bai ];then
  rm -f input.bai
fi

log_info "Stage starts"

start_ts=$(date +%s)
$SAMTOOLS index $input 
end_ts=$(date +%s)

log_info "Samtools index for $(basename $input) finishes in $((end_ts - start_ts))s"

# Start manager
start_manager

export PATH=$DIR:$PATH

start_ts=$(date +%s)

rm -f $log_dir/bqsr.log
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
  $additional_args \
  -run \
  &> $log_dir/bqsr.log

# Stop manager
stop_manager

# concat the log here

index_list="$(seq -w 1 32)"
for index in $index_list; do
  if [ -f .queue/scatterGather/BaseRecalQueue-1-sg/temp_${index}_of_32/*.out ];then
    cat .queue/scatterGather/BaseRecalQueue-1-sg/temp_${index}_of_32/*.out >> $log_dir/bqsr.log
  fi
done

if [ ! -f $rpt_donefile ];then
  log_error "Stage failed, you can check $log_dir/bqsr.log for details" 
  exit 1
fi

# Remove the temp files
if [ -f $rpt_donefile ]; then
  rm -f $rpt_donefile
fi
if [ -f $rpt_out_donefile ]; then
  rm -f $rpt_out_donefile
fi
if [ -d .queue ];then
  rm .queue -rf
fi

end_ts=$(date +%s);
log_info "Stage finishes in $((end_ts - start_ts))s"
