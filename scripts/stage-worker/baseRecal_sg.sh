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
check_arg "-o" "output_rpt"
check_args

check_arg "-r" "ref_fasta" "$ref_genome"

if [ -z ${knownSites[0]} ]; then
  knownSites_string="--knownSites $g1000_indels \
                     --knownSites $g1000_gold_standard_indels \
                     --knownSites $db138_SNPs"
  log_warn "The knownSites are not specified, by default the following reference "
  log_warn "will be used:"
  log_warn "- $g1000_indels"
  log_warn "- $g1000_gold_standard_indels"
  log_warn "- $db138_SNPs"
  log_warn "You can use the -ks|-knownSites option to chose different ones."
fi

for ks in ${knownSites[@]}; do
   knownSites_string="$knownSites_string --knownSites $ks"  
done

# Get absolute filepath for input/output
readlink_check ref_fasta
readlink_check input
readlink_check output_rpt
readlink_check log_dir

rpt_dir=$output_dir/scatter_rpt

create_dir $log_dir
create_dir $rpt_dir
check_output_dir $rpt_dir

# Table storing all the pids for tasks within one stage
declare -A contig_rpt
declare -A pid_table
declare -A output_table

nparts=32
contig_list="$(seq 1 $nparts)"

# Get the input list
input_bam_string=
if [ -d $input ]; then
  for contig in $contig_list; do
    contig_bam=`ls -v $input/*.contig${contig}.bam`
    input_bam_string="$input_bam_string -I $contig_bam"
  done
else
  input_bam_string="-I $input"
fi

for contig in $contig_list; do
  contig_rpt["$contig"]=$rpt_dir/$(basename $input).contig${contig}.recal.rpt
  check_output ${contig_rpt["$contig"]}
done 

# Start manager
start_manager
trap "terminate" 1 2 3 9 15

export PATH=$DIR:$PATH

start_ts=$(date +%s)
for contig in $contig_list; do
  $DIR/../fcs-sh "$DIR/baseRecal_contig.sh \
      $ref_fasta \
      $DIR/scatter_intervals/intv${contig}.intervals \
      \"$input_bam_string\" \
      \"$knownSites_string\" \
      ${contig_rpt["$contig"]} \
      $contig" 2> $log_dir/baseRecal_contig${contig}.log &
    
  pid_table["$contig"]=$!
  output_table["$contig"]=${contig_rpt["$contig"]}
done
   
# Wait on all the tasks
log_file=$log_dir/baseRecal.log
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
  contig_log=$log_dir/baseRecal_contig${contig}.log
  cat $contig_log >> $log_file
  rm -f $log_dir/printReads_contig${contig}.log
done
end_ts=$(date +%s)

# Stop manager
stop_manager

if [ "$is_error" -ne 0 ]; then
  log_error "Stage failed, please check logs in $log_file for details."
  exit 1
fi

# Gather the reports
for contig in $contig_list; do
  gather_input_string="$gather_input_string I=${contig_rpt[$contig]}"
done

$JAVA -cp $GATK org.broadinstitute.gatk.tools.GatherBqsrReports  \
  $gather_input_string \
  O=$output_rpt &>>$log_file

if [ "$?" -ne 0 ];then
  log_error "Stage failed, please check logs in $log_file for details."
  exit 1
fi

for contig in $contig_list; do
  rm $rpt_dir -rf
done

unset contig_rpt
unset pid_table
unset output_table


log_info "Stage finishes in $((end_ts - start_ts))s"
