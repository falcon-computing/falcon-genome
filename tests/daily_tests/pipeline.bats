#!/usr/bin/env bats

@test "Check for fcs-genome" {
  usage=$(fcs-genome | head -n 1)
  [ "$usage" == "Falcon Genome Analysis Toolkit v1.1.1-20-gfa092" ]
}

@test "Check pipeline script without arguments" {
  run $pipeline
  [ "$status" != "0" ]
  [ "${lines[0]}" == "USAGE: $pipeline [Fastq_file_path] [Platform] [Library] [Path_to_Output_dir]" ]
}

@test "Check pipeline script with an invalid argument for the FASTQ input" {
  run $pipeline $test_fastq/wrong Illumina Lib1 $output
  [ "$status" != "0" ]
}

@test "Check pipeline with small input FASTQ file" {
  run $pipeline $test_fastq/small Illumina Lib1 $output
  [ "$status" == "0" ]
}

