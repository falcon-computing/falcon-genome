#!/usr/bin/env bats

load "${BATS_TEST_DIRNAME}"/../settings.bash

@test "Run HTC with no extra-options: default parameters are run" {
  skip
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf -f
  [ "$status" == "0" ]
  [[ "$output" == *"--emitRefConfidence GVCF --variant_index_type LINEAR --variant_index_parameter 128000"* ]]
}

@test "Run HTC with single extra-options: user specified arg is run; default for every other option" {
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--emitRefConfidence NONE" -f
  [ "$status" == "0" ]
  [[ "$output" == *"--emitRefConfidence NONE --variant_index_type LINEAR --variant_index_parameter 128000"* ]]
  [[ "$output" != *"--emitRefConfidence GVCF"* ]]
}

@test "Run HTC with single extra-options: arg name is different" {
  skip
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output/A15_sample.vcf --extra-options "-ERC false"
}

@test "Run HTC with all extra-options" {
  skip
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output/A15_sample.vcf --extra-options "--emitRefConfidence false --variant_index_parameter -1 --variant_index_type DYNAMIC_SEEK"
}

@test "Run single boolean option" {
  skip
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output/A15_sample.vcf --extra-options "--debug"
}

@test "Run multiple option boolean+non-boolean with bool as the first option" {
  skip
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output/A15_sample.vcf --extra-options "--debug --emitRefConfidence false --variant_index_parameter -1"  
}

@test "Run multiple option boolean+non-boolean with bool as the middle option" {
  skip
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output/A15_sample.vcf --extra-options "--emitRefConfidence false --debug --variant_index_parameter -1"
}

@test "Run multiple option boolean+non-boolean with bool as the last option" {
  skip
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output/A15_sample.vcf --extra-options "--emitRefConfidence false --variant_index_parameter -1 --debug"
}

@test "Run options by giving more than one --extra-opts parameter" {
  skip
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output/A15_sample.vcf --extra-options "--emitRefConfidence false --debug" --extra-options "--variant_index_parameter -1 --variant_index_type DYNAMIC_SEEK"
}

