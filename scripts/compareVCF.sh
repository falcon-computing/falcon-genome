#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh
source $DIR/stage-worker/common.sh

print_help() {
  echo "USAGE: $0 -i <input_dir> -r <reference_dir> -t rtg|gatk"
}

if [[ $# -lt 2 ]]; then
  print_help 
  exit 1
fi

while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
  -i|--input_dir)
    comp_dir="$2"
    shift # past argument
    ;;
  -r|--ref_dir)
    base_dir="$2"
    shift # past argument
    ;;
  -t|--tool)
    tool="$2"
    shift # past argument
    ;;
  -o|--output)
    result_output="$2"
    shift # past argument
    ;;
  -v|--verbose)
    verbose=2
    ;;
  *) # unknown option
    log_error "failed to parse argument $key"
    ;;
  esac
  shift # past argument or value
done

check_arg "-i" "comp_dir"
check_arg "-r" "base_dir"
check_arg "-t" "tool" "rtg"
check_arg "-o" "result_output" "results.csv"
check_args

get_sample_id() {
  local input_dir=$1;
  local sample_id=`ls $input_dir/*.gvcf | sed -e 'N;s/^\(.*\).*\n\1.*$/\1\n\1/;D'`;
  local sample_id=$(basename $sample_id);
  local sample_id=${sample_id::-4};
  echo $sample_id;
}

check_input_chr() {
  local input_dir=$1;
  local chr_list="$(seq 1 22) X Y MT";
  for chr in $chr_list; do
    if [ -z "`find $input_dir -name *.chr${chr}.gvcf`" ]; then
      log_error "ERROR: cannot find VCF for chr$chr";
      return 1;
    fi;
  done;
}

rtg_format_vcf() {
  if [ "$#" -lt 2 ]; then
    return 1;
  fi;
  local input=$1;
  local output=$2;
  if [ ! -f $input ]; then
    log_error "ERROR: input file $input does not exist";
    return 1;
  fi;
  if [ -f $output ]; then
    log_warn "WARNING: output file $output already exists, skipping formatVCF()";
    return 0;
  fi;
  log_info "$BGZIP -c $input > $output";
  $BGZIP -c $input > $output;
  log_info "$TABIX -p vcf $output";
  $TABIX -p vcf $output;
}

rtg_merge_vcfs() {
  if [ "$#" -lt 2 ]; then
    return 1;
  fi;
  local input_dir=$1;
  local output_dir=$2;
  local sample_id=`get_sample_id $input_dir`;
  local output=$output_dir/${sample_id}.gvcf.gz;

  # Format vcf file for each chromosome
  local input_vcfs=;
  local chr_list="$(seq 1 22) X Y MT";
  for chr in $chr_list; do
    local chr_gvcf=$input_dir/${sample_id}_chr${chr}.gvcf;
    if [ ! -f ${chr_gvcf}.gz ]; then
      rtg_format_vcf $chr_gvcf ${chr_gvcf}.gz;
      if [ "$?" -ne 0 ]; then
        log_error "ERROR: format VCF failed for $chr_gvcf";
        return 1;
      fi;
    fi;
    local input_vcfs="$input_vcfs ${chr_gvcf}.gz";
  done;
  
  # Merge vcf file
  log_info "$RTGTOOL vcfmerge -o $output $input_vcfs";
  $RTGTOOL vcfmerge -o $output $input_vcfs;
  if [ "$?" -ne 0 ]; then
    return 1;
  fi;
}

rtg_compare_vcf() {
  if [ "$#" -lt 3 ]; then
    return 1;
  fi;
  local eval_vcf=$1;
  local comp_vcf=$2;
  local output=$3;
  local ref=${ref_genome}.sdf;

  log_info "$RTGTOOL vcfeval -t $ref \
    -b $comp_vcf \
    -c $eval_vcf \
    -o $output";

  $RTGTOOL vcfeval -t $ref \
    -b $comp_vcf \
    -c $eval_vcf \
    -o $output;

  if [ "$?" -ne 0 ]; then
    return 1;
  fi;
}

gatk_compare_vcf() {
  local eval_file=$1;
  local base_file=$2;
  local temp_eval=$(basename $1).eval.tmp;
  local temp_comp=$(basename $2).comp.tmp;
  local output_file=$(basename $1).compare.rpt;

  log_info "$JAVA -d64 -Xmx4g -jar $GATK \
      -T VariantEval \
      -R $ref_genome \
      -D $db138_SNPs \
      -noEV -EV CompOverlap \
      --eval $eval_file \
      --comp $base_file \
      -o $output_file"

  $JAVA -d64 -Xmx4g -jar $GATK \
      -T VariantEval \
      -R $ref_genome \
      -D $db138_SNPs \
      -noEV -EV CompOverlap \
      --eval $eval_file \
      --comp $base_file \
      -o $output_file 2> /dev/null;
}

# Check files in $comp_dir
check_input_chr $comp_dir
if [ "$?" -ne 0 ]; then
  log_error "ERROR: failed to find valid gvcf files in $comp_dir"
  exit 1
fi

# Check files in $base_dir
check_input_chr $base_dir
if [ "$?" -ne 0 ]; then
   log_error "ERROR: failed to find valid gvcf files in $base_dir"
  exit 1
fi

# Check if $sample_id of comp and base matches
comp_id=`get_sample_id $comp_dir`
base_id=`get_sample_id $base_dir`
if [[ "$comp_id" != "$base_id" ]]; then
  log_error "ERROR: sample id for baseline ($base_id) and target ($comp_id) does not match"
  exit 1
fi

# Start comparison
if [[ "$tool" == "rtg" ]]; then
  log_info "Using RTG Tool to evaluate VCFs"

  comp_vcf=$comp_dir/${comp_id}.gvcf.gz
  base_vcf=$base_dir/${base_id}.gvcf.gz
  if [ ! -f $comp_vcf ]; then
    log_info "$comp_vcf does not exist, start generating with rtg"
    rtg_merge_vcfs $comp_dir $comp_dir
    if [ "$?" -ne 0 ]; then
      log_error "ERROR: merge VCF failed for $comp_dir"
      exit 1
    fi
  fi
  
  if [ ! -f $base_vcf ]; then
    log_info "$base_vcf does not exist, start generating with rtg"
    rtg_merge_vcfs $base_dir $base_dir
    if [ "$?" -ne 0 ]; then
      log_error "ERROR: merge VCF failed for $base_dir"
      exit 1
    fi
  fi
  
  # Compare comp_vcf with base_vcf
  output_dname=${comp_id}_vcfeval
  counter=0
  while [ -d $output_dname ]; do
    counter=$((counter + 1))
    output_dname=${comp_id}_vcfeval.${counter}
  done
  
  results=`rtg_compare_vcf $comp_vcf $base_vcf $output_dname \
          | grep "None" \
          | awk -v OFS=',' '{print $2,$3,$4,$5,$6,$7}'`

  if [ "$?" -ne 0 ]; then
    log_error "ERROR: VCF evaluation failed"
    exit 1
  fi

  # Append results to a file and print it on screen
  echo $comp_id,$results >> $result_output
  log_error "$comp_id,$results"

elif [[ "$tool" == "gatk" ]]; then
  log_info "Using GATK to evaluate VCFs"

  sample_id=$comp_id
  chr_list="$(seq 1 22) X Y MT";
  for chr in $chr_list; do
    comp_vcf=$comp_dir/${sample_id}_chr${chr}.gvcf
    base_vcf=$base_dir/${sample_id}_chr${chr}.gvcf
    gatk_compare_vcf $comp_vcf $base_vcf &
    pid_table["$chr"]=$!
  done

  # Wait on all the tasks
  for pid in ${pid_table[@]}; do
    wait "${pid}"
  done

  n_extra=0
  n_concordant=0
  for chr in $chr_list; do
    output_file=${sample_id}_chr${chr}.gvcf.compare.rpt
    n_var=`grep novel $output_file | sed -n '2p' | awk '{print $(NF-3)}'`
    n_con=`grep novel $output_file | sed -n '2p' | awk '{print $(NF-1)}'`
    if [ $n_var -gt $n_con ]; then
      n_extra=$(($n_extra + $n_var - $n_con))
    fi
    n_concordant=$(($n_concordant + $n_con))
  done
  if [ "$n_extra" -eq "0" ]; then
    log_error "HTC for $sample_id passes verification"
  else
    concordance=`bc <<< "scale=2; 100.0*$n_concordant / ($n_concordant + $n_extra)"`
    log_error "HTC for $sample_id fails verification, concordance=$concordance"
  fi
else
  log_error "Error: Tool selection (-t) must either be 'rtg' or 'gatk'"
  exit 1
fi
