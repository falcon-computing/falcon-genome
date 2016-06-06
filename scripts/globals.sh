# Modify the following paths accordingly
output_dir=$(pwd)
input_dir=/pool/hdd1/diwu
tools_dir=/fcs_common/merlin1/tools

# Directories setup
fastq_dir=$input_dir/fastq
ref_dir=/space/scratch/genome/ref
sam_dir=$output_dir/sam
bam_dir=$output_dir/bam
rpt_dir=$output_dir/rpt
vcf_dir=$output_dir/vcf
log_dir=$output_dir/log

# Reference Genome Info
ref_genome=$ref_dir/factor4/human_g1k_v37.fasta
db138_SNPs=$ref_dir/dbsnp_138.b37.vcf
g1000_indels=$ref_dir/1000G_phase1.indels.b37.vcf
g1000_gold_standard_indels=$ref_dir/Mills_and_1000G_gold_standard.indels.b37.vcf

# Tools
#BWA=/curr/diwu/prog/bwa-flow/bin/run-bwa.sh
BWA=$tools_dir/bwa-fcs/bwa
SAMTOOLS=$tools_dir/fcs-samtools
PICARD=$tools_dir/fcs-picard.jar
GATK=$tools_dir/gatk-3.5/GenomeAnalysisTK.jar
#GATK_QUEUE=$tools_dir/gatk-3.5/Queue.jar
GATK_QUEUE=$tools_dir/fcs-gatk/Queue.jar
JAVA=/curr/diwu/tools/jdk1.7.0_80/bin/java

check_input() {
  filename=$1;
  if [ ! -f $filename ]; then
    echo "Input file $filename does not exist";
    exit 1;
  fi;
}

check_output() {
  filename=$1;
  if [ -f $filename ]; then
    echo "Output file $filename already exists";
    exit 1;
  fi;
  # Check if output folder writtable
  echo "1" > $filename;
  if [ ! -f $filename ]; then
    echo "Cannot write to folder $(dirname $filename)";
    exit 1;
  fi;
  rm $filename;
}
