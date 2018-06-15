#!/usr/bin/env bats

load "${BATS_TEST_DIRNAME}"/../settings.bash

@test "Run joint with no extra-options: default run" {
  run fcs-genome joint --ref $ref_genome -i $input_gvcf -o $output_vcf -f
  [ "$status" == "0" ]
}

@test "Run joint with extra-options" {
  run fcs-genome joint --ref $ref_genome -i $input_gvcf -o $output_vcf -O "--dbsnp $dbsnp -stand_call_conf 50" -f
  [ "$status" == "0" ]
  [[ "$output" == *"Parsing one extra option: Key=--dbsnp, Value=$dbsnp"* ]]
  [[ "$output" == *"Parsing one extra option: Key=-stand_call_conf, Value=50"* ]]
}

@test "Run joint with multiple extra-options of the same key" {
  run fcs-genome joint --ref $ref_genome -i $input_gvcf -o $output_vcf -O "--dbsnp $dbsnp -stand_call_conf 50 -A ReadPosRankSumTest -A MappingQualityRankSumTest" -f
  [ "$status" == "0" ]
}

@test "Run HTC with duplicate argument to overwrite default" {
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--emitRefConfidence NONE" -f
  [ "$status" == "0" ]
  [[ "$output" == *"--emitRefConfidence NONE --variant_index_type LINEAR --variant_index_parameter 128000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--emitRefConfidence, Value=NONE"* ]] 
  [[ "$output" != *"--emitRefConfidence GVCF"* ]]
}

@test "Run extra-option having boolean value" {
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--debug" -f
  [ "$status" == "0" ]
  [[ "$output" == *"--debug"* ]]
  [[ "$output" == *"--emitRefConfidence GVCF --variant_index_type LINEAR --variant_index_parameter 128000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--debug, Value="* ]]
} 
