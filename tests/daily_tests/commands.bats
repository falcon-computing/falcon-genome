#!/usr/bin/env bats

load ../settings

@test "Check for fcs-genome" {
  usage=$(fcs-genome | head -n 1)
  [[ "$usage" =~ "Falcon Genome Analysis Toolkit" ]]
}

@test "fcs-genome align without arguments invokes usage of the command" {
  run fcs-genome align
  [ "$status" != "0" ]
  [ "${lines[1]}" == "'fcs-genome align' options:" ]
}

@test "fcs-genome align with invalid argument for FASTQ input produces error" {
  run fcs-genome align --ref $ref_genome --fastq1 $test_fastq/wrong_1.fastq.gz --fastq2 $test_fastq/wrong_2.fastq.gz --output $output/aligned.bam --rg wrong --sp wrong --pl Illumina --lb Lib1
  [ "$status" != "0" ]
}

@test "fcs-genome align for small input" {
  run fcs-genome align --ref $ref_genome --fastq1 $test_fastq/small_1.fastq.gz --fastq2 $test_fastq/small_2.fastq.gz --output $output/small_aligned.bam --rg small --sp small --pl Illumina --lb Lib1
  [ "$status" == "0" ]
}

@test "fcs-genome bqsr without arguments invokes usage of the command" {
  run fcs-genome bqsr
  [ "$status" == "1" ]
  [ "${lines[1]}" == "'fcs-genome bqsr' options:" ]
}

@test "fcs-genome bqsr with invalid argument for BAM input produces error" {
  run fcs-genome bqsr --ref $ref_genome --input $output/wrong_aligned.bam --output $output/final_BAM.bam --knownSites $db138_SNPs --knownSites $g1000_indels --knownSites $g1000_gold_standard_indels
  [ "$status" != "0" ]
}

@test "fcs-genome bqsr for small input" {
  run fcs-genome bqsr --ref $ref_genome --input $output/small_aligned.bam --output $output/small_final_BAM.bam --knownSites $db138_SNPs --knownSites $g1000_indels --knownSites $g1000_gold_standard_indels
  [ "$status" == "0" ]
}

@test "fcs-genome htc without arguments invokes usage of the command" {
  run fcs-genome htc
  [ "$status" == "1" ]
  [ "${lines[1]}" == "'fcs-genome htc' options:" ]
}

@test "fcs-genome htc with invalid argument for BAM input produces error" {
  run fcs-genome htc --ref $ref_genome --input $output/wrong_final_BAM.bam --output $output/small.gvcf
  [ "$status" != "0" ]
}

@test "fcs-genome htc for small input" {
  run fcs-genome htc --ref $ref_genome --input $output/small_final_BAM.bam --output $output/small.gvcf
  [ "$status" == "0" ]
}
