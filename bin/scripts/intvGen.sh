#!/bin/bash

skip_chr=0

# Get the input command 
while [[ $# -gt 0 ]];do
  key="$1"
  case $key in
  -r|--ref)
    ref_fasta="$2"
    shift
    ;;
  -n|--num-partitions)
    num_parts="$2"
    shift
    ;;
  -l|--no-pseudo-chromo)
    skip_chr=1
    ;;
  *)
    # unknown option
    exit 1
    ;;
  esac
  shift # past argument or value
done

ref_dict="${ref_fasta%%.*}.dict"
if [ ! -f $ref_dict ]; then
  &>2 echo "Cannot find $ref_dict"
  exit 1
fi

intv_dir=$(pwd)/.fcs-genome/intv_$num_parts
if [ -d $intv_dir ]; then
  rm -r $intv_dir
#  is_ready=1
#  for idx in $(seq 1 $num_parts); do
#    if [ ! -f $intv_dir/intv${idx}.list ]; then
#      is_ready=0
#      break;
#    fi
#  done
#  if [ $is_ready -eq 1 ]; then
#    exit 0
#  fi
fi
mkdir -p $intv_dir

# get contig information from ref.dict
dict_file=$intv_dir/contigs
cat $ref_dict | grep "@SQ" | awk '{print $2,$3}' | sed -e 's/SN://g' | sed -e 's/LN://g' > $dict_file
if [ "$skip_chr" -ne 0 ]; then
  head -n 25 $dict_file > ${dict_file}.new
  mv ${dict_file}.new $dict_file
fi

declare -A intv_list
group_list=(`cat $dict_file | awk '{print $1}'`)
total_npos=0

IFS=$'\n' 
for line in $(cat $dict_file); do
  key=`echo $line | awk '{print $1}'`
  value=`echo $line | awk '{print $2}'`
  intv_list["$key"]=`echo $line | awk '{print $2}'`
  total_npos=$((total_npos + value))
done

# number of positions per contig
contig_npos=$((total_npos / num_parts + 1))
echo $contig_npos

remain_npos=$contig_npos
contig_idx=1
lbound=1
ubound=$part_npos
for group in "${group_list[@]}"; do
  group_npos=${intv_list["$group"]}
  npos=$group_npos
  while [ "$npos" -ge "$remain_npos" ]; do
    echo "npos: $npos"
    ubound=$((remain_npos + lbound - 1))
    echo "$group:$lbound-$ubound" >> $intv_dir/intv${contig_idx}.list
    lbound=$((ubound + 1))
    npos=$((npos - remain_npos))
    contig_idx=$((contig_idx + 1))
    remain_npos=$contig_npos
  done
  if [ $npos -gt 0 ]; then
    echo "$group:$lbound-$group_npos" >> $intv_dir/intv${contig_idx}.list
    remain_npos=$((remain_npos - npos))
    lbound=1
  fi
done
