#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

while [[ $# -gt 0 ]]
do
key="$1"

case $key in
  -f|--fastq)
  fastq_base="$2"
  shift
  ;;
  -o|--output)
  output="$2"
  shift
  ;;
  -t1|--temp1)
  temp_dir1="$2"
  shift
  ;;
  -t2|--temp2)
  temp_dir2="$2"
  shift
  ;;
  -cl|--clean)
  clean_flag="$2"
  shift
  ;;
  -h|--help)
  help_req=YES
  ;;
esac
shift
done


# Check the command 
if [ ! -z $help_req ];then
   echo "USAGE: fcs_genome pipeline -f <fastq_basename> -o <output_dir> -t1 <temp_dir1> -t2 <temp_dir2>"
   exit 1
fi


if [ -z $fastq_base ];then
  echo "Fastq basename is missing, please check cmd"
  exit 1
fi

if [ -z $output ];then
  output=$output_dir
  echo "Output file name is not set, the output file is stored to "$output" as default"
fi

if [ -z $temp_dir1 ];then
  create_dir ${tmp_dir[1]}
  tmp_dir1=${tmp_dir[1]}
  echo "Temp directory 1 is not set, using default "$tmp_dir1" instead"
fi

if [ -z $temp_dir2 ];then
  create_dir ${tmp_dir[2]}
  tmp_dir2=${tmp_dir[2]}
  echo "Temp directory 2 is not set, using default "$tmp_dir2" instead"
fi

# Start the bwa mem alignment
fcs_genome align -f $fastq_base -o $tmp_dir1/${fastq_base}.bam -t $tmp_dir2

if [ "$?" -ne 0 ]; then
  echo "Alignment failed"
  exit 1; 
fi

# Start the Mark Duplicates
fcs_genome markdup -i $tmp_dir1/${fastq_base}.bam -o $tmp_dir2/${fastq_base}.bam.markdups.bam -t $tmp_dir1

if [ "$?" -ne 0 ]; then
  echo "MarkDuplicate failed"
  exit 2;
fi

if [ "$clean_flag" =="true" ]; then
  echo "Clean the input sorted bam for space"
  rm $tmp_dir1/${fastq_base}.bam &
fi

# Start the GATK BaseRecalibrate
# first index the bam file 
fcs_genome index -i $tmp_dir2/${fastq_base}.bam.markdups.bam 

if [ "$?" -ne 0 ]; then
  echo "Index failed"
  exit 3;
fi

# then split the bam file 
fcs_genome split -i $tmp_dir2/${fastq_base}.bam.markdups.bam -o $tmp_dir1 #&
#split_pid=$!

# then do the bqsr
rpt_dir=$output_dir/rpt
create_dir $rpt_dir
fcs_genome bqsr -i $tmp_dir2/${fastq_base}.bam.markdups.bam -o $rpt_dir  

if [ "$?" -ne 0 ]; then
  echo "bqsr failed"
  exit 4;
fi

# Wait for split to finish
#wait "${split_pid}"

# Start PrintRead
fcs_genome printread -i $fastq_base -c $tmp_dir1 -r $rpt_dir

if [ "$?" -ne 0 ]; then
  echo "Printread failed"
  exit 5;
fi

# Start HaplotypeCaller
vcf_dir=$output_dir/vcf
create_dir $vcf_dir
fcs_genome haplotypecaller -i gcat_300k -c $tmp_dir2 -o $vcf_dir 
if [ "$?" -ne 0 ]; then
  echo "HaplotypeCaller failed"
  exit 5;
fi

