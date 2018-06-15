#!/usr/bin/env bats

load "${BATS_TEST_DIRNAME}"/../settings.bash

@test "Run joint with no extra-options: default run" {
  run ${FCSBIN} joint --ref $ref_genome -i $input_gvcf -o $output_vcf -f
  [ "$status" == "0" ]
}

@test "Run joint with extra-options" {
  run ${FCSBIN} joint --ref $ref_genome -i $input_gvcf -o $output_vcf -O "--dbsnp $db138_SNPs -stand_call_conf 50" -f
  [ "$status" == "0" ]
  [[ "$output" == *"--dbsnp $db138_SNPs -stand_call_conf 50"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--dbsnp, Value=$dbsnp"* ]]
  [[ "$output" == *"Parsing one extra option: Key=-stand_call_conf, Value=50"* ]]
}

@test "Run joint with multiple extra-options of the same key" {
  run ${FCSBIN} joint --ref $ref_genome -i $input_gvcf -o $output_vcf -O "--dbsnp $db138_SNPs -stand_call_conf 50 -A ReadPosRankSumTest -A MappingQualityRankSumTest" -f
  [ "$status" == "0" ]
  [[ "$output" == *"--dbsnp $db138_SNPs -A ReadPosRankSumTest -A MappingQualityRankSumTest -stand_call_conf 50"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--dbsnp, Value=$dbsnp"* ]]
  [[ "$output" == *"Parsing one extra option: Key=-stand_call_conf, Value=50"* ]]
  [[ "$output" == *"Parsing one extra option: Key=-A, Value=MappingQualityRankSumTest"* ]]
  [[ "$output" == *"Parsing one extra option: Key=-A, Value=ReadPosRankSumTest"* ]]
}

@test "Run HTC with duplicate argument to overwrite default" {
  run ${FCSBIN} htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--emitRefConfidence NONE" -f
  [ "$status" == "0" ]
  [[ "$output" == *"--emitRefConfidence NONE --variant_index_type LINEAR --variant_index_parameter 128000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--emitRefConfidence, Value=NONE"* ]] 
  [[ "$output" != *"--emitRefConfidence GVCF"* ]]
}

@test "Run extra-option having boolean value" { 
  run ${FCSBIN} htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--debug" -f
  [ "$status" == "0" ]
  [[ "$output" == *" --debug "* ]]
  [[ "$output" == *"--emitRefConfidence GVCF --variant_index_type LINEAR --variant_index_parameter 128000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--debug, Value="* ]]
}

@test "Run extra-option having multiple boolean and non-boolean value" {
  run ${FCSBIN} htc --ref $ref_genome --input $test_BAM --output $output_dir/A15_sample.vcf --extra-options "--debug --emitRefConfidence NONE --variant_index_parameter 100000" --extra-options "--variant_index_type DYNAMIC_SEEK -edr" -f
  [ "$status" == "0" ]
  [[ "$output" == *"--debug --emitRefConfidence NONE --variant_index_parameter 100000 --variant_index_type DYNAMIC_SEEK -edr"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--debug, Value="* ]]
  [[ "$output" == *"Parsing one extra option: Key=--emitRefConfidence, Value=NONE"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--variant_index_parameter, Value=100000"* ]]
  [[ "$output" == *"Parsing one extra option: Key=-edr, Value="* ]]
  [[ "$output" == *"Parsing one extra option: Key=--variant_index_type, Value=DYNAMIC_SEEK"* ]] 
}
