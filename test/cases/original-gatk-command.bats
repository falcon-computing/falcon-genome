#!/usr/bin/env bats

@test "Using single quotes: successful run" {
  run fcs-genome gatk -T VariantFiltration -R /pool/local/ref/human_g1k_v37.fasta -V /pool/storage/niveda/Results_validation/GATK-3.8/A15_sample.vcf.gz --filterExpression 'QD < 2.0 && DP <4 && QUAL <30 && FS >10 && ReadPosRankSum < -8.0' --filterName LowQual -o /curr/niveda/temp/small.filter.temp
  [ "$status" == "0" ]
}

@test "Using double quotes: successful run" {
  run fcs-genome gatk -T VariantFiltration -R /pool/local/ref/human_g1k_v37.fasta -V /pool/storage/niveda/Results_validation/GATK-3.8/A15_sample.vcf.gz --filterExpression "QD < 2.0 && DP <4 && QUAL <30 && FS >10 && ReadPosRankSum < -8.0" --filterName LowQual -o /curr/niveda/temp/small.filter.temp
  [ "$status" == "0" ]
}

@test "No quotes: Fail" {
  run fcs-genome gatk -T VariantFiltration -R /pool/local/ref/human_g1k_v37.fasta -V /pool/storage/niveda/Results_validation/GATK-3.8/A15_sample.vcf.gz --filterExpression QD < 2.0 && DP <4 && QUAL <30 && FS >10 && ReadPosRankSum < -8.0 --filterName LowQual -o /curr/niveda/temp/small.filter.temp
  [ "$status" != "0" ]
}
