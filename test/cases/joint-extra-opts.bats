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
