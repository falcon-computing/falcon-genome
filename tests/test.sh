#!/bin/bash

source daily_tests/test_helper.bash
source /curr/software/falcon-genome/latest/setup.sh

# Download small FASTQ files from s3

mkdir $test_fastq
aws s3 cp s3://fcs-genome-data/fastq/Tests/ "$test_fastq" --recursive

# Run bats tests

$BATS daily_tests/
