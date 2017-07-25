#!/bin/bash
DIR=$(cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd)
PARENTDIR="$(dirname "$DIR" )"
source $PARENTDIR/globals.sh.template

#Input
fastq1=$1
fastq2=$2

performance=performance.txt

start_ts=$(date +%s)

#Make directory
mkdir log_dir 

#Index reference file
#$BWA index $ref_genome

#Align to reference using BWA-MEM
$BWA mem -M \
  -R "@RG\tID:Cancer\tSM:Cancer\tPL:ILLUMINA" \
  $ref_genome \
  $fastq1 \
  $fastq2 >aln.sam #&>log_dir/bwa.log

#Sort
java -jar $PICARD SortSam \
  INPUT=aln.sam \
  OUTPUT=sorted.bam \
  SORT_ORDER=coordinate

if [ $? -ne 0 ];then
  echo "Alignment to reference failed"
  exit 1
fi

end_ts=$(date +%s)
echo "Alignment to reference finishes in $((end_ts - start_ts))s" >> $performance

start_ts=$(date +%s)

#Index BAM file

samtools index sorted.bam

end_ts=$(date +%s)
echo "Indexing BAM file finishes in $((end_ts - start_ts))s" >> $performance

start_ts=$(date +%s)

#Mark duplicates
java -jar $PICARD MarkDuplicates \
  INPUT=sorted.bam \
  OUTPUT=dup_reads.bam \
  METRICS_FILE=metrics.txt #\
  #&> log_dir/mark_dup.log

if [ $? -ne 0 ];then
  echo "Mark duplicates failed"
  exit 1
fi

end_ts=$(date +%s)
echo "Mark duplicates finishes in $((end_ts - start_ts))s" >> $performance

start_ts=$(date +%s)

#Index BAM file

samtools index dup_reads.bam

end_ts=$(date +%s)
echo "Indexing BAM file finishes in $((end_ts - start_ts))s" >> $performance

start_ts=$(date +%s)

#Indel Realignment Target Creator
java -jar $GATK \
  -T RealignerTargetCreator \
  -R $ref_genome \
  -I dup_reads.bam \
  --known $g1000_indels --known $g1000_gold_standard_indels --known $db138_SNPs \
  -nt 18 \
  -o target.intervals &>log_dir/rtc.log

if [ $? -ne 0 ];then
  echo "Indel Realignment Target Creator failed"
  exit 1
fi

end_ts=$(date +%s)

start_ts=$(date +%s)

#Indel Realignment
java -jar $GATK \
  -T IndelRealigner \
  -R $ref_genome \
  -I dup_reads.bam \
  -known $g1000_indels -known $g1000_gold_standard_indels -known $db138_SNPs \
  --targetIntervals target.intervals \
  -o realigned.bam

if [ $? -ne 0 ];then
  echo "Indel Realignment failed"
  exit 1
fi

end_ts=$(date +%s)
 
start_ts=$(date +%s)
#Base Recalibration
java -jar $GATK \
  -T BaseRecalibrator \
  -R $ref_genome \
  -I  realigned.bam \
  -knownSites $db138_SNPs -knownSites $g1000_indels -knownSites $g1000_gold_standard_indels \
  -nct 4 \
  -o recal_data.table 

if [ $? -ne 0 ];then
  echo "BaseRecalibrator failed"
  exit 1
fi

end_ts=$(date +%s)
echo "BQSR finishes in $((end_ts - start_ts))s" >> $performance

start_ts=$(date +%s)

#Print Reads
java -jar $GATK \
  -T PrintReads \
  -R $ref_genome \
  -I realigned.bam \
  -BQSR recal_data.table \
  -nct 4 \
  -o reads.recal.bam

end_ts=$(date +%s)
echo "Print reads finishes in $((end_ts - start_ts))s" >> $performance

#Call Mutect2

./mutect2.sh reads.recal.bam

