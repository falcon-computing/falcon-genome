#!/usr/bin/env bats

#load test_helper

@test "fcs-genome align without arguments invokes usage of the command" {
  run fcs-genome align
  [ "$status" != "0" ]
  [ "${lines[1]}" == "'fcs-genome align' options:" ]
} 

@test "fcs-genome align with invalid argument for FASTQ input produces error" {
  run fcs-genome align --ref /genome/ref/human_g1k_v37.fasta --fastq1 /curr/niveda/test_fastq/wrong_1.fastq.gz --fastq2 /curr/niveda/test_fastq/wrong_2.fastq.gz --output /genome/local/temp/wrong_aligned.bam --rg wrong --sp wrong --pl Illumina --lb Lib1
  [ "$status" != "0" ]
}

@test "fcs-genome align for small input" {
  skip
  run fcs-genome align --ref /genome/ref/human_g1k_v37.fasta --fastq1 /curr/niveda/test_fastq/small_1.fastq.gz --fastq2 /curr/niveda/test_fastq/small_2.fastq.gz --output /genome/local/temp/small_aligned.bam --rg small --sp small --pl Illumina --lb Lib1
  [ "$status" == "0" ]
}    

@test "fcs-genome bqsr for small input" {
  skip
  run fcs-genome bqsr --ref /genome/ref/human_g1k_v37.fasta --input /genome/local/temp/small_aligned.bam --output /genome/local/temp/small_final_BAM.bam --knownSites /genome/ref/dbsnp_138.b37.vcf --knownSites /genome/ref/1000G_phase1.indels.b37.vcf --knownSites /genome/ref/Mills_and_1000G_gold_standard.indels.b37.vcf
  [ "$status" == "0" ]
}

@test "fcs-genome htc for small input" {
  run fcs-genome htc --ref /genome/ref/human_g1k_v37.fasta --input /genome/local/temp/small_final_BAM.bam --output small.gvcf
  [ "$status" == "0" ]
}


  
