#!/bin/bash

#Start
start_ts_total=$(date +%s)
<<COMMENT1
DIR=$( cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd)
PARENTDIR="$(dirname "$DIR")"
source $PARENTDIR/globals.sh

stage_name=mutect2

#Prevent this script from running alone 
if [[ $0 != ${BASH_SOURCE[0]} ]]; then
  #Script is sourced by another shell
  cmd_name=`basename $0 2> /dev/null`
  if [[ "$cmd_name" != "fcs_genome" ]]; then
    log_error "This script should be started by 'fcs-genome'"
    exit 1
  fi
else
  #Script is executed directly 
  log_error "This script should be started by 'fcs-genome'"
  exit 1
fi

print_help() {
  echo "USAGE: $JAVA -jar $GATK -T MuTect2 --help"
}

if [ $# -lt 2 ]; then
  print_help 
  exit 1;
fi

#Get input command
while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
  gatk)
    pass_args=YES
  ;;
  -r|--ref|-R)
    ref_fasta="$2"
    shift 
  ;;
  -I:tumor|-i:tumor)
    tumor_bam="$2"
    shift
  ;;
  -I:normal|-i:normal)
    normal_bam="$2"
    shift
  ;;
  -o|--output|-O)
    vcf_dir="$2"
    shift
  ;;
  *)
    if [ -z $pass_args ];then
      log_error "Failed to recognize argument '$1'"
      print_help
      exit 1
    else
      additional_args="$additional_args $1"
    fi
  esac
  shift
done

COMMENT1

tumor_bam=/pool/storage/diwu/annovar/LB-2907-TumorDNA.recal.bam/
normal_bam=/pool/storage/diwu/annovar/LB-2907-BloodDNA.recal.bam/
sample="Tumor"
#Declare array 
declare -A pid_table

#Make directory
mkdir outfile

#Check of the tumor_bam is a directory containing multiple recalibrated BAM files or a single recalibrated BAM file

#If directory, then do coverage calculation for each file
if [ -d "$tumor_bam" ];then
  if [ -d "$normal_bam" ];then
    for file in $(ls /pool/storage/diwu/annovar/LB-2907-TumorDNA.recal.bam/)
    do
      if [[ $file =~ bam$ ]];then
        java -jar /curr/diwu/tools/gatk-3.6/GenomeAnalysisTK.jar \
          -T MuTect2 \
          -R /pool/local/ref/human_g1k_v37.fasta \
          -I:tumor /pool/storage/diwu/annovar/LB-2907-TumorDNA.recal.bam/$file \
          -I:normal /pool/storage/diwu/annovar/LB-2907-BloodDNA.recal.bam/$file \
          --dbsnp /pool/local/ref/dbsnp_138.b37.vcf \
          -L /pool/local/ref/RefSeq_Exon_Intervals_Gene.interval_list \
          -o outfile/${file}_mutect.vcf 
          -nct 4 &
      
        pid_table["$file"]=$! 
      fi
    done

    for file in $(ls "$tumor_bam")
    do
      if [[ $file =~ bam$ ]];then
        pid=${pid_table[$file]}
        wait "${pid}"
        if [[ $? -ne 0 ]];then
          echo "Failed to run Mutect2 for $file"
          exit 1
        fi
        file_log=outfile/${file}_mutect.vcf
        cat $file_log >> Mutect2.vcf
      fi
    done
  fi
fi  


end_ts=$(date +%s)
echo "Finishes in $((end_ts - start_ts_total))s"

