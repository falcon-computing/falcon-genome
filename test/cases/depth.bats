#!/usr/bin/env bats

load "${BATS_TEST_DIRNAME}"/../settings.bash

@test "Run DepthOfCoverage with missing arg geneList: error" {
  run fcs-genome depth --ref /pool/local/ref/human_g1k_v37.fasta --input /pool/storage/niveda/Results_validation/GATK-3.8/A15_sample_final_BAM.bam/ --output /pool/storage/niveda/Results_validation/temp_output/A15.csv -f
  [ "$status" != "0" ]
  [[ "$output" == *"ERROR: Missing argument '--geneList'"* ]]
}

@test "Run DepthOfCoverage with depthCutoff=30: " {
  run fcs-genome depth --ref /pool/local/ref/human_g1k_v37.fasta --input /pool/storage/niveda/Results_validation/GATK-3.8/A15_sample_final_BAM.bam/ --output /pool/storage/niveda/Results_validation/temp_output/A15.csv --geneList /pool/local/ref/RefSeq_Exon_Intervals_Gene.interval_list --depthCutoff 30 -f
  echo "$output"
  [ "$status" == "0" ]
  [[ "$output" == *"-ct 30"* ]]
}

@test "Run DepthOfCoverage with baseCoverage" {
  run fcs-genome depth --ref /pool/local/ref/human_g1k_v37.fasta --input /pool/storage/niveda/Results_validation/GATK-3.8/A15_sample_final_BAM.bam/ --output /pool/storage/niveda/Results_validation/temp_output/A15.csv --geneList /pool/local/ref/RefSeq_Exon_Intervals_Gene.interval_list --baseCoverage -f
  echo "$output"
  [ "$status" == "0" ]
  [[ "$output" != *"-omitBaseOutput"* ]]
  [[ "$output" == *"-omitIntevals"* ]]
  [[ "$output" == *"-omitSampleSummary"* ]]
}

@test "Run DepthOfCoverage with intervalCoverage" {
  run fcs-genome depth --ref /pool/local/ref/human_g1k_v37.fasta --input /pool/storage/niveda/Results_validation/GATK-3.8/A15_sample_final_BAM.bam/ --output /pool/storage/niveda/Results_validation/temp_output/A15.csv --geneList /pool/local/ref/RefSeq_Exon_Intervals_Gene.interval_list --intervalCoverage -f
  echo "$output"
  [ "$status" == "0" ]
  [[ "$output" == *"-omitBaseOutput"* ]]
  [[ "$output" != *"-omitIntevals"* ]]
  [[ "$output" == *"-omitSampleSummary"* ]]
}

@test "Run DepthOfCoverage with sampleSummary" {
  run fcs-genome depth --ref /pool/local/ref/human_g1k_v37.fasta --input /pool/storage/niveda/Results_validation/GATK-3.8/A15_sample_final_BAM.bam/ --output /pool/storage/niveda/Results_validation/temp_output/A15.csv --geneList /pool/local/ref/RefSeq_Exon_Intervals_Gene.interval_list --sampleSummary -f
  echo "$output"
  [ "$status" == "0" ]
  [[ "$output" == *"-omitBaseOutput"* ]]
  [[ "$output" == *"-omitIntevals"* ]]
  [[ "$output" != *"-omitSampleSummary"* ]]
}

@test "Run DepthOfCoverage with all optional arguments" {
  run fcs-genome depth --ref /pool/local/ref/human_g1k_v37.fasta --input /pool/storage/niveda/Results_validation/GATK-3.8/A15_sample_final_BAM.bam/ --output /pool/storage/niveda/Results_validation/temp_output/A15.csv --geneList /pool/local/ref/RefSeq_Exon_Intervals_Gene.interval_list --depthCutoff 30 --baseCoverage --intervalCoverage --sampleSummary -f
  echo "$output"
  [ "$status" == "0" ]
  [[ "$output" != *"-omitBaseOutput"* ]]
  [[ "$output" != *"-omitIntevals"* ]]
  [[ "$output" != *"-omitSampleSummary"* ]]
}

@test "Run DepthOfCoverage with no optional arguments" {
  run fcs-genome depth --ref /pool/local/ref/human_g1k_v37.fasta --input /pool/storage/niveda/Results_validation/GATK-3.8/A15_sample_final_BAM.bam/ --output /pool/storage/niveda/Results_validation/temp_output/A15.csv --geneList /pool/local/ref/RefSeq_Exon_Intervals_Gene.interval_list -f
  echo "$output"
  [ "$status" == "0" ]
  [[ "$output" == *"-omitBaseOutput"* ]]
  [[ "$output" == *"-omitIntevals"* ]]
  [[ "$output" == *"-omitSampleSummary"* ]]
}
