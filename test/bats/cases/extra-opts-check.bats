#!/usr/bin/env bats

load "${BATS_TEST_DIRNAME}"/../settings.bash

@test "Run HTC with no extra-options: default parameters are run" {
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf -f
  [ "$status" == "0" ]
  [[ "$output" == *"--emitRefConfidence GVCF --variant_index_type LINEAR --variant_index_parameter 128000"* ]]
}

@test "Run HTC with single extra-options: user specified arg is run; default for every other option" {
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--emitRefConfidence NONE" -f
  [ "$status" == "0" ]
  [[ "$output" == *"--emitRefConfidence NONE --variant_index_type LINEAR --variant_index_parameter 128000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--emitRefConfidence, Value=NONE"* ]] 
  [[ "$output" != *"--emitRefConfidence GVCF"* ]]
}

@test "Run HTC with single extra-options: arg name is different" {
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "-ERC NONE" -f
  [ "$status" == "0" ]
  [[ "$output" == *"-ERC NONE --variant_index_type LINEAR --variant_index_parameter 128000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=-ERC, Value=NONE"* ]]
  [[ "$output" != *"--emitRefConfidence GVCF"* ]]  
}

@test "Run HTC with all extra-options" {
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--emitRefConfidence NONE --variant_index_type DYNAMIC_SEEK --variant_index_parameter 100000" -f
  [ "$status" == "0" ]
  [[ "$output" == *"--emitRefConfidence NONE --variant_index_parameter 100000 --variant_index_type DYNAMIC_SEEK"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--emitRefConfidence, Value=NONE"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--variant_index_parameter, Value=100000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--variant_index_type, Value=DYNAMIC_SEEK"* ]]
  [[ "$output" != *"--emitRefConfidence GVCF"* ]]
  [[ "$output" != *"--variant_index_type LINEAR"* ]]
  [[ "$output" != *"--variant_index_parameter 128000" ]]
}

@test "Run single boolean option" {
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--debug" -f
  [ "$status" == "0" ]
  [[ "$output" == *"--debug"* ]]
  [[ "$output" == *"--emitRefConfidence GVCF --variant_index_type LINEAR --variant_index_parameter 128000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--debug, Value="* ]]
}

@test "Run multiple option boolean+non-boolean with bool as the first option" {
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--debug --emitRefConfidence NONE --variant_index_parameter 100000" -f
  [ "$status" == "0" ]
  [[ "$output" == *"--debug --emitRefConfidence NONE --variant_index_parameter 100000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--debug, Value="* ]]
  [[ "$output" == *"Parsing one extra option: Key=--emitRefConfidence, Value=NONE"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--variant_index_parameter, Value=100000"* ]] 
}

@test "Run multiple option boolean+non-boolean with bool as the middle option" {
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--emitRefConfidence NONE --debug --variant_index_parameter 100000" -f
  [ "$status" == "0" ]
  [[ "$output" == *"--debug --emitRefConfidence NONE --variant_index_parameter 100000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--debug, Value="* ]]
  [[ "$output" == *"Parsing one extra option: Key=--emitRefConfidence, Value=NONE"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--variant_index_parameter, Value=100000"* ]]
}

@test "Run multiple option boolean+non-boolean with bool as the last option" {
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--emitRefConfidence NONE --variant_index_parameter 100000 --debug" -f
  [ "$status" == "0" ]
  [[ "$output" == *"--debug --emitRefConfidence NONE --variant_index_parameter 100000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--debug, Value="* ]]
  [[ "$output" == *"Parsing one extra option: Key=--emitRefConfidence, Value=NONE"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--variant_index_parameter, Value=100000"* ]]
}

@test "Run options by giving more than one --extra-opts parameter" {
  run fcs-genome htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--emitRefConfidence NONE --debug" --extra-options "--variant_index_parameter 100000 --variant_index_type DYNAMIC_SEEK" -f
  [ "$status" == "0" ]
  [[ "$output" == *"--debug --emitRefConfidence NONE --variant_index_parameter 100000 --variant_index_type DYNAMIC_SEEK"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--debug, Value="* ]]
  [[ "$output" == *"Parsing one extra option: Key=--emitRefConfidence, Value=NONE"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--variant_index_parameter, Value=100000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--variant_index_type, Value=DYNAMIC_SEEK"* ]]
}
