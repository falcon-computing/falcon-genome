#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

stage_name=combineGVCF
print_help() {
  echo "USAGE:"
  echo "fcs-genome cb|combineGVCF \\";
  echo "    -r <ref.fasta> \\"
  echo "    -i <input_dir> \\"
  echo "    -o <output_dir> \\"
}

if [[ $# -lt 2 ]]; then
  print_help 
  exit 1
fi

while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
  -r|--ref)
    ref_fasta="$2"
    shift
    ;;
  -i|--input_dir)
    input_dir="$2"
    shift # past argument
    ;;
  -o|--output_dir)
    output_dir="$2"
    shift # past argument
    ;;
  -v|--verbose)
    if [ $2 -eq $2 2> /dev/null ]; then
      # user specified an integer as input
      verbose="$2"
      shift
    else
      verbose=2
      shift
    fi
    ;;
  *) # unknown option
    log_error "failed to parse argument $key"
    ;;
  esac
  shift # past argument or value
done

check_input_chr() {
  local input_dir=$1;
  local chr_list="$(seq 1 22) X Y MT";
  for chr in $chr_list; do
    if [ -z "`find $input_dir -name *.chr${chr}.gvcf`" ]; then
      log_error "ERROR: cannot find VCF for chr$chr";
      exit 1;
    fi;
  done;
}

concatVCF() {
  local input_dir=$1;
  local output=$2;
  output_base=${output%%.*}
  $BCFTOOLS concat $input_dir/*.chr*.gvcf -o ${output_base}.gvcf
  $BGZIP -c ${output_base}.gvcf > $output
  $TABIX -p vcf $output
}
# Check the args
check_arg "-i" "input_dir"
check_arg "-o" "output_dir"
check_output_dir $output_dir
check_arg "-r" "ref_fasta" "$ref_genome"

# Get absolute file path
readlink_check ref_fasta
readlink_check input_dir
readlink_check output_dir
readlink_check log_dir

create_dir $output_dir
create_dir $log_dir
check_output_dir $output_dir
check_output_dir $log_dir

# Get a sample list from input dir
IFS=$'\r\n' GLOBIGNORE='*' command eval 'sample_list=($(ls $input_dir))'

input_samples=()
for sample_id in "${sample_list[@]}"; do
  check_input_chr "$input_dir/$sample_id"
  if [ "$?" -eq 0 ]; then
    input_samples+=("$sample_id")
  fi
done

if [ ${#input_samples[@]} -lt 1 ]; then
  log_error "Cannot find valid input files"
  exit 1
fi
sample_num=${#input_samples[@]}
# First concat and compress the vcf files

log_info "Start concating chr to one for $sample_num samples"
start_ts=$(date +%s)

declare -A pid_table
start_manager
trap "terminate" 1 2 3 15

echo "Concating for $sample_num samples" >$log_dir/combine.log

# Start the concat jobs distributely
for sample in "${input_samples[@]}"; do
  $DIR/../fcs-sh "$DIR/concatVCF.sh $input_dir/$sample $input_dir/$sample/${sample}.gvcf.gz" \
  &> $log_dir/concat.${sample}.log &
  pid_table["$sample"]=$!
done

# Wait for all the tasks
is_error=0
for sample in "${input_samples[@]}"; do
  pid=${pid_table[$sample]}
  wait "${pid}"
  if [ "$?" -gt 0 ]; then
    is_error=1
    log_error "Failed on sample $sample"
  fi
  # Concat logs
  sample_log=$log_dir/concat.${sample}.log
  cat $sample_log >> $log_dir/combine.log
  rm -f $sample_log
done

stop_manager
unset pid_table

if [ "$is_error" -ne 0 ];then
  log_error "Stage failed, please check logs in $log_dir/combine.log for details"
  exit 1
fi

end_ts=$(date +%s)
log_info "Concating finishes in $((end_ts - start_ts))s "
 
# Write the callsets json file
echo "{" > callset.json
echo ' "callsets" : {' >> callset.json
row_idx=0
for sample in "${input_samples[@]}"; do
  echo '   "'$sample'" : {' >> callset.json
  echo '      "row_idx" : '$row_idx, >> callset.json
  echo '      "idx_in_file" : 0,' >> callset.json
  echo '      "filename":' '"'$input_dir/$sample/${sample}.gvcf.gz'"' >> callset.json
  row_idx=$[$row_idx+1]
  if [ $row_idx -eq $sample_num ];then
    echo "   }" >> callset.json
  else
    echo "   }," >> callset.json
  fi
done
echo " }" >> callset.json
echo "}" >> callset.json

# Compute the interval to be equal
GENOME_LENGTH=3101976562
workspace=${tmp_dir[1]}/ws_combine
#TODO:automatically detect the nparts based on machine
nparts=24
interval_size=$((GENOME_LENGTH/nparts))

# Write the loader json file
size_per_column_partition=$((16*1024*sample_num))
begin_column_id=0
partition_id=1
echo "{" > loader.json
echo ' "row_based_partitioning" : false,' >>loader.json
echo ' "produce_combined_vcf" : true,' >>loader.json
echo ' "column_partitions" : [' >>loader.json
while [ $partition_id -le $nparts ];do
  echo '  { "begin": ' $begin_column_id, >>loader.json
  echo '   "workspace":"'$workspace'",' >>loader.json
  echo '   "array": "partition-'$partition_id'",' >>loader.json     
  echo '   "vcf_output_filename":"'$output_dir/part-${partition_id}.gvcf'"' >>loader.json
  if [ $partition_id -eq $nparts ];then
    echo "  }" >>loader.json
  else
    echo "  }," >>loader.json
  fi
  begin_column_id=$((begin_column_id+interval_size))
  partition_id=$[$partition_id+1]
done
echo ' ],' >>loader.json
echo ' "callset_mapping_file" : "callset.json",' >>loader.json
echo ' "vid_mapping_file" : "'$DIR/../vid.json'",' >>loader.json
echo ' "treat_deletions_as_intervals" : true,' >>loader.json
echo ' "reference_genome" : "'$ref_fasta'",' >>loader.json
echo ' "do_ping_pong_buffering" : true,' >>loader.json
echo ' "size_per_column_partition" :'$size_per_column_partition, >>loader.json
echo ' "offload_vcf_output_processing" : true' >>loader.json
echo "}" >>loader.json

# Start the combine operation
start_ts=$(date +%s)

echo "run GenomicDB for $nparts patitions" >>$log_dir/combine.log
mpirun -n $nparts -hostfile $DIR/GenomicsDB_hostfile $GenomicsDB loader.json &>>$log_dir/combine.log
if [ "$?" -ne 0 ]; then 
  log_error "combineGVCF failed, please check $log_dir/combine.log for details"
  exit 1
fi

end_ts=$(date +%s)
log_info "Stage finishes in $((end_ts - start_ts))s"
