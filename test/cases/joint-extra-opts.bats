#!/usr/bin/env bats

ref_genome=/curr/niveda/ref/human_g1k_v37.fasta
output_vcf=/curr/niveda/dnaseq/test/temp/output.vcf
input_gvcf=/curr/niveda/tests/joint/small.gvcf.gz
dbsnp=/curr/niveda/ref/dbsnp_138.b37.vcf

@test "Run joint with no extra-options: default run" {
  skip
  run /curr/niveda/dnaseq/bin/fcs-genome joint --ref $ref_genome -i $input_gvcf -o $output_vcf -f
  echo "${output}"
  [ "$status" == "0" ]
}

@test "Run joint with extra-options" {
  run /curr/niveda/dnaseq/bin/fcs-genome joint --ref $ref_genome -i $input_gvcf -o $output_vcf -O "--dbsnp $dbsnp -A MappingQualityRankSumTest" -f
  echo "${output}"
  [ "$status" == "0" ]
  [[ "$output" == *"Parsing one extra option: Key=-A, Value=MappingQualityRankSumTest"* ]]
  [[ "$output" == *"Parsing one extra option: Key=--dbsnp, Value=$dbsnp"* ]]
} 
