#!/usr/bin/env bats

load test_helper

@test "fcs-genome align without arguments invokes usage of the command" {
  run fcs-genome align
  [ "$status" != "0" ]
  [ "${lines[1]}" == "'fcs-genome align' options:" ]
}

@test "fcs-genome align with invalid argument for FASTQ input produces error" {
  run fcs-genome align --ref $ref_genome --fastq1 $test_fastq/wrong_1.fastq.gz --fastq2 $test_fastq/wrong_2.fastq.gz --output $output/wrong_aligned.bam --rg wrong --sp wrong --pl Illumina --lb Lib1
  [ "$status" != "0" ]
}

@test "fcs-genome align for small input" {
  run fcs-genome align --ref $ref_genome --fastq1 $test_fastq/small_1.fastq.gz --fastq2 $test_fastq/small_2.fastq.gz --output $output/small_aligned.bam --rg small --sp small --pl Illumina --lb Lib1
  [ "$status" == "0" ]
}

@test "fcs-genome bqsr for small input" {
  run fcs-genome bqsr --ref $ref_genome --input $output/small_aligned.bam --output $output/small_final_BAM.bam --knownSites $db138_SNPs --knownSites $g1000_indels --knownSites $g1000_gold_standard_indels
  [ "$status" == "0" ]
}

@test "fcs-genome htc for small input" {
  run fcs-genome htc --ref $ref_genome --input $output/small_final_BAM.bam --output $output/small.gvcf
  [ "$status" == "0" ]
}


  
