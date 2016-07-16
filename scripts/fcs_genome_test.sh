#############################################################################
## This script does the test of each stage
############################################################################
#!/usr/bin/env bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh
source $DIR/stage-worker/common.sh
# input and output dir, could be changed by user
input_fastq_dir=/merlin_fs/merlin1/hdd1/yaoh/fastq_short
results_dir=/merlin_fs/merlin2/ssd1/diwu
ref_vcf_dir=/merlin_fs/merlin2/ssd1/yaoh/v1.0

# Create seperate dir for each stage
create_dir $results_dir/bams_al
create_dir $results_dir/bams_md
create_dir $results_dir/rpt
create_dir $results_dir/bams_pr
create_dir $results_dir/vcf
create_dir $results_dir/compare

declare -A BASENAME
BASENAME[1]=na_1M
BASENAME[2]=CDMD1015_100k
BASENAME[3]=exome01_100k
BASENAME[4]=exome02_100k
BASENAME[5]=exome03_100k
BASENAME[6]=A15_100k
BASENAME[7]=DSDEX72_100k
BASENAME[8]=NS17_100k

index_list="$(seq 1 8)"
for index in $index_list; do
  base_of_input=${BASENAME[$index]}
  # Start alignment
  fcs-genome al -fq1 $input_fastq_dir/${base_of_input}_1.fastq \
                -fq2 $input_fastq_dir/${base_of_input}_2.fastq \
                -o $results_dir/bams_al/${base_of_input}.bam \
                -rg SEQ01 -sp SEQ01 -pl ILLUMINA -lb HUMsgR2AQDCAAPE \
                -v 1 -f
  
  if [ "$?" -ne 0 ];then
     log_error "Alignment for ${base_of_input} failed"
     exit 1;
  fi
  
  # Start mark-dup
  fcs-genome md -i $results_dir/bams_al/${base_of_input}.bam \
                -o $results_dir/bams_md/${base_of_input}.markdups.bam \
                -v 1 -f
  
  if [ "$?" -ne 0 ];then
     log_error "Mark-duplicates for ${base_of_input} failed"
     exit 1;
  fi
  
  # Start bqsr
  fcs-genome bqsr -i $results_dir/bams_md/${base_of_input}.markdups.bam \
                  -o $results_dir/rpt/${base_of_input}.markdups.bam.recal.rpt \
                  -v 1 -f
  
  if [ "$?" -ne 0 ];then
     log_error "Base recalibration for ${base_of_input} failed"
     exit 1;
  fi
  
  # Start printreads
  create_dir $results_dir/bams_pr/${base_of_input}
  fcs-genome pr -i $results_dir/bams_md/${base_of_input}.markdups.bam \
                -bqsr $results_dir/rpt/${base_of_input}.markdups.bam.recal.rpt \
                -o $results_dir/bams_pr/${base_of_input} \
                -v 1 -f
         
  if [ "$?" -ne 0 ];then
     log_error "Printreads for ${base_of_input} failed"
     exit 1;
  fi
  
  # Start HaplotypeCaller
  create_dir $results_dir/vcf/${base_of_input}
  fcs-genome hptc -i $results_dir/bams_pr/${base_of_input} \
                  -o $results_dir/vcf/${base_of_input} \
                  -v 1 -f 
  
  if [ "$?" -ne 0 ];then
     log_error "HaplotypeCaller for ${base_of_input} failed"
     exit 1;
  fi
  
  # Compare vcf results
  compareVCF.sh -i $results_dir/vcf/${base_of_input} \
                -r $ref_vcf_dir/${base_of_input} \
                -t rtg \
                -o $results_dir/compare/${base_of_input}.results.csv
  
  if [ "$?" -ne 0 ];then
     log_error "VCF compare for ${base_of_input} failed"
     #exit 1;
  fi
done
echo "All test finished"
