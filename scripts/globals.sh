# Modify the following paths accordingly
fastq_dir=/merlin_fs/merlin1/hdd1/diwu/fastq
ref_dir=/space/scratch/genome/ref
output_dir=$(pwd)
tools_dir=/fcs_common/merlin1/tools

bam_dir=$output_dir/bam
rpt_dir=$output_dir/rpt
vcf_dir=$output_dir/vcf

# Reference Genome Info
ref_genome=$ref_dir/human_g1k_v37.fasta
db138_SNPs=$ref_dir/dbsnp_138.b37.vcf
g1000_indels=$ref_dir/1000G_phase1.indels.b37.vcf
g1000_gold_standard_indels=$ref_dir/Mills_and_1000G_gold_standard.indels.b37.vcf

# Tools
BWA=$tools_dir/bwa/bwa
SAMTOOLS=$tools_dir/samtools-1.3/samtools
GATK=$tools_dir/gatk-3.5/GenomeAnalysisTK.jar
PICARD=$tools_dir/picard-tools-1.141/picard.jar
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

# create output dir if it does not exist 
create_dir() {
  dir=$1
  if [[ ! -d "$dir" ]]; then
    mkdir $dir &> /dev/null;
    if [ $? -ne 0 ]; then
      echo "Cannot create dir $dir";
      exit 1;
    fi
  fi;
}
