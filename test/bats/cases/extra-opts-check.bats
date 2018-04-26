#!/usr/bin/env bats

load "${BATS_TEST_DIRNAME}"/../settings.bash

@test "Run HTC with no extra-options: default parameters are run" {
  run $EXE htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf -f
  [[ "$output" == *"--emitRefConfidence GVCF --variant_index_type LINEAR --variant_index_parameter 128000"* ]]
}

@test "Run HTC with single extra-options: user specified arg is run; default for every other option" {
  run $EXE htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--emitRefConfidence NONE" -f
  [[ "$output" == *"--emitRefConfidence NONE --variant_index_type LINEAR --variant_index_parameter 128000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--emitRefConfidence, Value=NONE"* ]] 
  [[ "$output" != *"--emitRefConfidence GVCF"* ]]
}

@test "Run HTC with single extra-options: arg name is different" {
  run $EXE htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "-ERC NONE" -f
  [[ "$output" == *"-ERC NONE --variant_index_type LINEAR --variant_index_parameter 128000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=-ERC, Value=NONE"* ]]
  [[ "$output" != *"--emitRefConfidence GVCF"* ]]  
}

@test "Run HTC with all extra-options" {
  run $EXE htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--emitRefConfidence NONE --variant_index_type DYNAMIC_SEEK --variant_index_parameter 100000" -f
  [[ "$output" == *"--emitRefConfidence NONE --variant_index_parameter 100000 --variant_index_type DYNAMIC_SEEK"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--emitRefConfidence, Value=NONE"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--variant_index_parameter, Value=100000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--variant_index_type, Value=DYNAMIC_SEEK"* ]]
  [[ "$output" != *"--emitRefConfidence GVCF"* ]]
  [[ "$output" != *"--variant_index_type LINEAR"* ]]
  [[ "$output" != *"--variant_index_parameter 128000" ]]
}

@test "Run single boolean option" {
  run $EXE htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--debug" -f
  [[ "$output" == *"--debug"* ]]
  [[ "$output" == *"--emitRefConfidence GVCF --variant_index_type LINEAR --variant_index_parameter 128000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--debug, Value="* ]]
}

@test "Run multiple option boolean+non-boolean with bool as the first option" {
  run $EXE htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--debug --emitRefConfidence NONE --variant_index_parameter 100000" -f
  [[ "$output" == *"--debug --emitRefConfidence NONE --variant_index_parameter 100000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--debug, Value="* ]]
  [[ "$output" == *"Parsing one extra option: Key=--emitRefConfidence, Value=NONE"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--variant_index_parameter, Value=100000"* ]] 
}

@test "Run multiple option boolean+non-boolean with bool as the middle option" {
  run $EXE htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--emitRefConfidence NONE --debug --variant_index_parameter 100000" -f
  [[ "$output" == *"--debug --emitRefConfidence NONE --variant_index_parameter 100000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--debug, Value="* ]]
  [[ "$output" == *"Parsing one extra option: Key=--emitRefConfidence, Value=NONE"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--variant_index_parameter, Value=100000"* ]]
}

@test "Run multiple option boolean+non-boolean with bool as the last option" {
  run $EXE htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--emitRefConfidence NONE --variant_index_parameter 100000 --debug" -f
  [[ "$output" == *"--debug --emitRefConfidence NONE --variant_index_parameter 100000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--debug, Value="* ]]
  [[ "$output" == *"Parsing one extra option: Key=--emitRefConfidence, Value=NONE"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--variant_index_parameter, Value=100000"* ]]
}

@test "Run options by giving more than one --extra-opts parameter" {
  run $EXE htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--emitRefConfidence NONE --debug" --extra-options "--variant_index_parameter 100000 --variant_index_type DYNAMIC_SEEK" -f
  [[ "$output" == *"--debug --emitRefConfidence NONE --variant_index_parameter 100000 --variant_index_type DYNAMIC_SEEK"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--debug, Value="* ]]
  [[ "$output" == *"Parsing one extra option: Key=--emitRefConfidence, Value=NONE"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--variant_index_parameter, Value=100000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--variant_index_type, Value=DYNAMIC_SEEK"* ]]
}
