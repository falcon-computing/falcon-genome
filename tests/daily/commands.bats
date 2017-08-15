#!/usr/bin/env bats

load "${BATS_TEST_DIRNAME}/../settings.bash"
fcs_genome="${BATS_TEST_DIRNAME}/../../bin/fcs-genome"

@test "Check for fcs-genome" {
  usage=$("$fcs_genome" | head -n 1)
  [[ "$usage" =~ "Falcon Genome Analysis Toolkit" ]]
}

@test "fcs-genome align without arguments invokes usage of the command" {
  run "$fcs_genome" align
  [ "$status" != "0" ]
  [ "${lines[1]}" == "'fcs-genome align' options:" ]
}

@test "fcs-genome align with invalid argument for FASTQ input produces error" {
  run "$fcs_genome" align --ref $ref_genome --fastq1 $test_fastq/wrong_1.fastq.gz --fastq2 $test_fastq/wrong_2.fastq.gz --output $output/aligned.bam --rg wrong --sp wrong --pl Illumina --lb Lib1
  [ "$status" != "0" ]
}

@test "fcs-genome align with missing FASTQ1 input produces error" {
  run "$fcs_genome" align --ref $ref_genome --fastq2 $test_fastq/small_2.fastq.gz --output $output/small_aligned.bam --rg small --sp small --pl Illumina --lb Lib1
  [ "$status" != "0" ]
  [[ "${lines[0]}" =~ "ERROR: Missing argument '--fastq1'" ]]
}

@test "fcs-genome align with missing FASTQ2 input produces error" {
  run "$fcs_genome" align --ref $ref_genome --fastq1 $test_fastq/small_1.fastq.gz --output $output/small_aligned.bam --rg small --sp small --pl Illumina --lb Lib1
  [ "$status" != "0" ]
  [[ "${lines[0]}" =~ "ERROR: Missing argument '--fastq2'" ]]
}

@test "fcs-genome align with missing output option produces error" {
  run "$fcs_genome" align --ref $ref_genome --fastq1 $test_fastq/small_1.fastq.gz --fastq2 $test_fastq/small_2.fastq.gz --rg small --sp small --pl Illumina --lb Lib1
  [ "$status" != "0" ]
  [[ "${lines[0]}" =~ "ERROR: Missing argument '--output'" ]]
}

@test "fcs-genome align with missing read group ID produces error" {
  run "$fcs_genome" align --ref $ref_genome --fastq1 $test_fastq/small_1.fastq.gz --fastq2 $test_fastq/small_2.fastq.gz --output $output/small_aligned.bam --sp small --pl Illumina --lb Lib1
  [ "$status" != "0" ]
  [[ "${lines[0]}" =~ "ERROR: Missing argument '--rg'" ]]
}

@test "fcs-genome align with missing sample ID produces error" {
  run "$fcs_genome" align --ref $ref_genome --fastq1 $test_fastq/small_1.fastq.gz --fastq2 $test_fastq/small_2.fastq.gz --output $output/small_aligned.bam --rg small --pl Illumina --lb Lib1
  [ "$status" != "0" ]
  [[ "${lines[0]}" =~ "ERROR: Missing argument '--sp'" ]]
}

@test "fcs-genome align with missing platform produces error" {
  run "$fcs_genome" align --ref $ref_genome --fastq1 $test_fastq/small_1.fastq.gz --fastq2 $test_fastq/small_2.fastq.gz --output $output/small_aligned.bam --rg small --sp small --lb Lib1
  [ "$status" != "0" ]
  [[ "${lines[0]}" =~ "ERROR: Missing argument '--pl'" ]]
}

@test "fcs-genome align with missing library ID produces error" {
  run "$fcs_genome" align --ref $ref_genome --fastq1 $test_fastq/small_1.fastq.gz --fastq2 $test_fastq/small_2.fastq.gz --output $output/small_aligned.bam --rg small --sp small --pl Illumina
  [ "$status" != "0" ]
  [[ "${lines[0]}" =~ "ERROR: Missing argument '--lb'" ]]
}

@test "fcs-genome align for small input" {
  run "$fcs_genome" align --ref $ref_genome --fastq1 $test_fastq/small_1.fastq.gz --fastq2 $test_fastq/small_2.fastq.gz --output $output/small_aligned.bam --rg small --sp small --pl Illumina --lb Lib1
  [ "$status" == "0" ]
}

@test "fcs-genome bqsr without arguments invokes usage of the command" {
  run "$fcs_genome" bqsr
  [ "$status" == "1" ]
  [ "${lines[1]}" == "'fcs-genome bqsr' options:" ]
}

@test "fcs-genome bqsr with invalid argument for BAM input produces error" {
  run "$fcs_genome" bqsr --ref $ref_genome --input $output/wrong_aligned.bam --output $output/final_BAM.bam --knownSites $db138_SNPs --knownSites $g1000_indels --knownSites $g1000_gold_standard_indels
  [ "$status" != "0" ]
}

@test "fcs-genome bqsr with missing reference argument produces error" {
  run "$fcs_genome" bqsr --input $output/small_aligned.bam --output $output/final_BAM.bam --knownSites $db138_SNPs --knownSites $g1000_indels --knownSites $g1000_gold_standard_indels
  [ "$status" != "0" ]
  [[ "${lines[0]}" =~ "ERROR: Missing argument '--ref'" ]]
}

@test "fcs-genome bqsr with missing input BAM file produces error" {
  run "$fcs_genome" bqsr --ref $ref_genome --output $output/final_BAM.bam --knownSites $db138_SNPs --knownSites $g1000_indels --knownSites $g1000_gold_standard_indels
  [ "$status" != "0" ]
  [[ "${lines[0]}" =~ "ERROR: Missing argument '--input'" ]]
}

@test "fcs-genome bqsr with missing output argument produces error" {
  run "$fcs_genome" bqsr --ref $ref_genome --input $output/wrong_aligned.bam
  [ "$status" != "0" ]
  [[ "${lines[0]}" =~ "ERROR: Missing argument '--output'" ]]
}

@test "fcs-genome bqsr for small input" {
  run "$fcs_genome" bqsr --ref $ref_genome --input $output/small_aligned.bam --output $output/small_final_BAM.bam --knownSites $db138_SNPs --knownSites $g1000_indels --knownSites $g1000_gold_standard_indels
  [ "$status" == "0" ]
}

@test "fcs-genome htc without arguments invokes usage of the command" {
  run "$fcs_genome" htc
  [ "$status" == "1" ]
  [ "${lines[1]}" == "'fcs-genome htc' options:" ]
}

@test "fcs-genome htc with invalid argument for BAM input produces error" {
  run "$fcs_genome" htc --ref $ref_genome --input $output/wrong_final_BAM.bam --output $output/small.gvcf
  [ "$status" != "0" ]
}

@test "fcs-genome htc with missing reference argument produces error" {
  run "$fcs_genome" htc --input $output/small_final_BAM.bam --output $output/small.gvcf
  [ "$status" != "0" ]
  [[ "${lines[0]}" =~ "ERROR: Missing argument '--ref'" ]]
}

@test "fcs-genome htc with missing input argument produces error" {
  run "$fcs_genome" htc --ref $ref_genome --output $output/small.gvcf
  [ "$status" != "0" ]
  [[ "${lines[0]}" =~ "ERROR: Missing argument '--input'" ]]
}

@test "fcs-genome htc with missing output argument produces error" {
  run "$fcs_genome" htc --ref $ref_genome --input $output/small_final_BAM.bam
  [ "$status" != "0" ]
  [[ "${lines[0]}" =~ "ERROR: Missing argument '--output'" ]]
}

@test "fcs-genome htc for small input" {
  run "$fcs_genome" htc --ref $ref_genome --input $output/small_final_BAM.bam --output $output/small.gvcf
  [ "$status" == "0" ]
}
