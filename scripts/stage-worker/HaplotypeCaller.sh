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
  echo "    -i <input_base> \\ ";
  echo "    -c <chr_dir> \\";
  echo "    -o <vcf_dir>";
  echo 
  echo "<input_base> argument is the basename of the bam file, should not contain suffixes";
  echo "<chr_dir> argument is the directory to find the recaled bams";
  echo "<vcf_dir> argument is the directory to put the vcf output results";
}

if [ $# -lt 1 ]; then
  print_help
  exit 1;
fi

# Get the input command
while [[ $# -gt 0 ]];do
 key="$1"
 case $key in
 -r|--ref)
   ref_fasta="$2"
   shift # past argument
   ;;
 -i|--input_base)
   input_base="$2"
   shift # past argument
   ;;
 -c|--chr_dir)
   chr_dir="$2"
   shift # past argument
   ;;
 -o|--output)
   vcf_dir="$2"
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

# If no argument is given then print help message
if [ ! -z $help_req ];then
  print_help
  exit 0;
fi

# Check the input arguments
check_arg "-i" "$input"
check_arg "-c" "$chr_dir"
check_args

check_arg "-r" "ref_fasta" "$ref_genome"
vcf_dir_default=$output_dir/vcf
check_arg "-o" "vcf_dir" "$vcf_dir_default"
create_dir $vcf_dir

# Check the inputs
check_input $ref_fasta
chr_list="$(seq 1 22) X Y MT"
for chr in $chr_list; do
  chr_bam=$chr_dir/${input_base}.recal.chr${chr}.bam
  if [ ! -f $chr_bam ];then 
    log_error "Could not find the $chr_bam, please check if you have pointed the right directory"
    exit 1
  fi
done
# Check the outputs
for chr in $chr_list; do
   chr_vcf=$vcf_dir/${input_base}_chr${chr}.gvcf
   check_output $chr_vcf
done

# Create the directorys
create_dir $log_dir

# Start the manager
declare -A pid_table
declare -A output_table

# Start manager
start_ts=$(date +%s)
start_manager

trap "terminate" 1 2 3 15

# Start the jobs

for chr in $chr_list; do
  chr_bam=$chr_dir/${input_base}.recal.chr${chr}.bam
  chr_vcf=$vcf_dir/${input_base}_chr${chr}.gvcf
  $DIR/../fcs-sh "$DIR/haploTC_chr.sh $chr $chr_bam $chr_vcf $ref_fasta" 2> $log_dir/haplotypeCaller_chr${chr}.log &
  pid_table["$chr"]=$!
  output_table["$chr"]=$chr_vcf
done

# Wait on all the tasks
log_file=$log_dir/HaplotypeCaller.log
is_error=0
for chr in ${!pid_table[@]}; do
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

for chr in $chr_list; do
  chr_bam=$chr_dir/${input_base}.bam.recal.chr${chr}.bam
  chr_vcf=$vcf_dir/${input_base}_chr${chr}.gvcf

  if [ ! -e ${chr_vcf}.done ]; then
    is_error=1
  fi
  rm ${chr_vcf}.done
done
 
end_ts=$(date +%s)
# Stop manager
stop_manager

unset pid_table
unset output_table

if [ "$is_error" -ne 0 ]; then
  log_error "Stage failed, please check logs in $log_file for details."
  exit 1
fi

log_info "Stage finishes in $((end_ts - start_ts))s"
