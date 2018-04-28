#!/usr/bin/env bats
 
load "${BATS_TEST_DIRNAME}/../settings.bash"

@test "Check for fcs-genome" {
  run fcs-genome
  [[ "$output" == *"Falcon Genome Analysis Toolkit"* ]]
}

@test "fcs-genome mutect2 without arguments invokes usage of the command" {
  run fcs-genome mutect2
  [ "$status" != "0" ]
  [[ "$output" == *"'fcs-genome mutect2' options:"* ]] 
}

@test "fcs-genome mutect2 with missing reference produces error message" {
  run fcs-genome mutect2 --normal $test_fastq/S9_BAM.bam --tumor $test_fastq/S10_BAM.bam --output $output/mutect2.vcf -f
  [ "$status" != "0" ]
  [[ "$output" == *"ERROR: Missing argument '--ref'"* ]]
}

@test "fcs-genome mutect2 with missing argument- normal BAM - produces error" {
  run fcs-genome mutect2 --ref $ref_genome --tumor $test_fastq/S10_BAM.bam --output $output/mutect2.vcf -f
  [ "$status" != "0" ]
  [[ "$output" == *"ERROR: Missing argument '--normal'"* ]]
}

@test "fcs-genome mutect2 with missing argument- tumor BAM - produces error" {
  run fcs-genome mutect2 --ref $ref_genome --normal $test_fastq/S9_BAM.bam --output $output/mutect2.vcf -f
  [ "$status" != "0" ]
  [[ "$output" == *"ERROR: Missing argument '--tumor'"* ]]
}

@test "fcs-genome mutect2 with missing output produces error" {
  run fcs-genome mutect2 --ref $ref_genome --normal $test_fastq/S9_BAM.bam --tumor $test_fastq/S10_BAM.bam -f
  [ "$status" != "0" ]
  [[ "$output" == *"ERROR: Missing argument '--output'"* ]]
  
}

@test "fcs-genome mutect2 with invalid path to ref" {
  run fcs-genome mutect2 --ref $wrong_ref/human_g1k_v37.fasta --normal $test_fastq/S9_BAM.bam --tumor $test_fastq/S10_BAM.bam --output $output/mutect2.vcf -f
   [ "$status" != "0" ]
  [[ "$output" == *"ERROR: Cannot find $wrong_ref/human_g1k_v37.fasta"* ]]
} 

@test "fcs-genome mutect2 with invalid path to normal BAM file" {
  run fcs-genome mutect2 --ref $ref_genome --normal $test_fastq/S9_BAM --tumor $test_fastq/S10_BAM.bam --output $output/mutect2.vcf -f
  [ "$status" != "0" ]
  [[ "$output" == *"ERROR: Cannot find $test_fastq/S9_BAM"* ]]
}

@test "fcs-genome mutect2 with invalid path to tumor BAM file" {
  run fcs-genome mutect2 --ref $ref_genome --normal $test_fastq/S9_BAM.bam --tumor $test_fastq/S10_BAM --output $output/mutect2.vcf -f
  [ "$status" != "0" ]
  [[ "$output" == *"ERROR: Cannot find $test_fastq/S10_BAM"* ]]
}

@test "fcs-genome mutect2 with invalid/unwritable output path" {
  run fcs-genome mutect2 --ref $ref_genome --normal $test_fastq/S9_BAM.bam --tumor $test_fastq/S10_BAM.bam --output $output_unwritable/mutect2.vcf -f
  [ "$status" != "0" ]
  [[ "$output" == *"ERROR: Cannot write to output path"* ]]

}

@test "fcs-genome mutect2 with required arguments: successful run" {
  run fcs-genome mutect2 --ref $ref_genome --normal $test_fastq/S9_BAM.bam --tumor $test_fastq/S10_BAM.bam --output $output/mutect2.vcf -f
  [ "$status" == "0" ]
}

@test "fcs-genome mutect2 with dbsnp argument: successful run" {
  run fcs-genome mutect2 --ref $ref_genome --normal $test_fastq/S9_BAM.bam --tumor $test_fastq/S10_BAM.bam --dbsnp $db138_SNPs --output $output/mutect2.vcf -f
  [ "$status" == "0" ]
}

@test "fcs-genome mutect2 with extra-opts argument: successful run" {
  run fcs-genome mutect2 --ref $ref_genome --normal $test_fastq/S9_BAM.bam --tumor $test_fastq/S10_BAM.bam --extra-options -inorder_output --output $output/mutect2.vcf -f
  [ "$status" == "0" ]
}

@test "fcs-genome mutect2 with cosmic argument: successful run" {   
  run fcs-genome mutect2 --ref $ref_genome --normal $test_fastq/S9_BAM.bam --tumor $test_fastq/S10_BAM.bam --cosmic $cosmic --output $output/mutect2.vcf -f
  [ "$status" == "0" ]
}

@test "fcs-genome mutect2 with all optional arguments: successful run" {  
  run fcs-genome mutect2 --ref $ref_genome --normal $test_fastq/S9_BAM.bam --tumor $test_fastq/S10_BAM.bam --dbsnp $db138_SNPs --cosmic $cosmic --extra-options -inorder_output --output $output/mutect2.vcf -f
  [ "$status" == "0" ]
}

