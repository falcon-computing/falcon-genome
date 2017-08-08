#!/bin/bash

source settings.bash
source $fcs_genome

# Create directory
mkdir $test_fastq
mkdir $output

# Download small FASTQ files from s3
aws s3 cp s3://fcs-genome-data/fastq/Tests/ "$test_fastq" --recursive

# Run bats tests
$BATS daily_tests/commands.bats
