#!/usr/bin/env bats
load ../global

helper_run() {
  #"Run depthofCoverage" {
  local -r id="$1"

  run fcs-genome depth \
    --ref ${ref_genome} \
    --input ${input_bam}/$id/${id}_final_BAM.bam \
    --output $workdir/$id \
    --geneList $geneList \
    --intervalCoverage --sampleSummary -f
 
  [ "$status" -eq 0 ]
}

helper_compareSummary() {
  #"Compare DOC summary file against baseline" {
  local -r id="$1"
  baseline_file="$baseline/${id}_bams.list.sample_summary"
  run compare_depthDiff "$baseline_file" "$id"
 
  echo "${output}"
  [ "$status" -eq 0 ]
}

@test "Run depth of coverage: $id" {
  helper_run "$id"
}

@test "Compare DOC summary file against baseline"  {
  helper_compareSummary "$id"
}
