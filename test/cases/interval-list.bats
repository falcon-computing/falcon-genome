#!/usr/bin/env bats

#load "${BATS_TEST_DIRNAME}"/../settings.bash

@test "Run BQSR option" {
  run fcs-genome bqsr
  [ "$status" != "0" ]
  [[ "$output" == *"-L [ --intervalList ] arg"* ]]
}

@test "Run BQSR without -L option: successful run" {
  run fcs-genome bqsr --ref /pool/local/ref/human_g1k_v37.fasta --input /pool/storage/niveda/Results_validation/GATK-3.8/A15_sample_marked.bam --output /curr/niveda/temp/A15.bam -K /pool/local/ref/dbsnp_138.b37.vcf -f 
  [ "$status" == "0" ]
  [[ "$output" != *"-isr INTERSECTION"* ]]
}

@test "Run BQSR with -L option: successful run" {
  run fcs-genome bqsr --ref /pool/local/ref/human_g1k_v37.fasta --input /pool/storage/niveda/Results_validation/GATK-3.8/A15_sample_marked.bam --output /curr/niveda/temp/A15.bam -K /pool/local/ref/dbsnp_138.b37.vcf -L /pool/local/ref/hg19_coding_merge.bed -f
  [ "$status" == "0" ]
  [[ "$output" == *"-L /pool/local/ref/hg19_coding_merge.bed"* ]]
  [[ "$output" == *"-isr INTERSECTION"* ]]
}

@test "Missing path for -L option: error" {
  run fcs-genome bqsr --ref /pool/local/ref/human_g1k_v37.fasta --input /pool/storage/niveda/Results_validation/GATK-3.8/A15_sample_marked.bam --output /curr/niveda/temp/A15.bam -K /pool/local/ref/dbsnp_138.b37.vcf -L /pool/local/ref/hg19_coding_merge.bed -L -f
  [ "$status" != "0" ]
  [[ "$output" == *"'--intervalList' is missing"* ]]
}
