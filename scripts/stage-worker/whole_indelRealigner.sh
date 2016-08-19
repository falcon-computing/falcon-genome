################################################################################
## This script creates tge realigner target
################################################################################
#!/bin/bash

# Import global variables
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

stage_name=Indelrealign

# Prevent this script to be running alone
#if [[ $0 != ${BASH_SOURCE[0]} ]]; then
#  # Script is sourced by another shell
#  cmd_name=`basename $0 2> /dev/null`
#  if [[ "$cmd_name" != "fcs-genome" ]]; then
#    log_error "This script should be started by 'fcs-genome'"
#    return 1
#  fi
#else
#  # Script is executed directly
#  log_error "This script should be started by 'fcs-genome'"
#  exit 1
#fi

print_help() {
  echo "USAGE:"
  echo "fcs-genome RTC \\"
  echo "    -r <ref.fasta> \\"
  echo "    -i <input.bam> \\"
  echo "    -known <indels.vcf> \\"
  echo "    -rtc <target.intervals> \\"
  echo "    -o <realigned.bam>"
}

if [ $# -lt 1 ]; then
  print_help $1
  exit 1;
fi

declare -A known
ks_index=0

while [[ $# -gt 0 ]]; do
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
  -known)
    known["$ks_index"]="$2"
    ks_index=$[$ks_index+1]
    shift # past argument
    ;;
  -rtc|RTC)
    target_interval="$2"
    shift
    ;;
  -o|--output|-O)
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
    log_error "Failed to recongize argument '$1'"
    print_help
    exit 1
    ;;
  esac
  shift # past argument or value
done

check_arg "-i" "input"
check_arg "-o" "output"
check_arg "-rtc" "target_interval"
check_args

check_arg "-r" "ref_fasta" "$ref_genome"

if [ -z ${known[0]} ]; then
  known_string="-known $g1000_indels \
                     -known $g1000_gold_standard_indels \
                     -known $db138_SNPs"
  log_warn "The known are not specified, by default the following reference "
  log_warn "will be used:"
  log_warn "- $g1000_indels"
  log_warn "- $g1000_gold_standard_indels"
  log_warn "- $db138_SNPs"
  log_warn "You can use the -known option to chose different ones."
fi

for ks in ${known[@]}; do
   known_string="$known_string -known $ks"  
done

# Get absolute filepath for input/output
readlink_check ref_fasta
readlink_check input
readlink_check target_interval
readlink_check output
readlink_check log_dir

create_dir $log_dir

check_input $input
check_input $target_interval
check_output $output

start_ts=$(date +%s);

$JAVA -jar $GATK \
  -T IndelRealigner \
  -R $ref_fasta \
  -I $input \
  $known_string \
  -targetIntervals $target_interval \
  -o $output &>$log_dir/realign.log

if [ "$?" -ne 0 ];then
  log_error "Stage failed, please check logs in $log_dir/realign.log for details."
  exit 1
fi

end_ts=$(date +%s);

log_info "Stage finishes in $((end_ts-start_ts))s"
 



