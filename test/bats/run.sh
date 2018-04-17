#!/bin/bash
source settings.bash

# Create directory
mkdir -p $output_dir

# Install BATS
mkdir -p "${DIR}/env"
mkdir -p "${DIR}/env/bats"
aws s3 sync s3://fcs-genome-data/falcon-genome-test/tools/BATS "${DIR}/env/bats"

chmod 777 "$BATS"
chmod 777 $DIR/env/bats/libexec/*
export PATH=$PATH:$DIR/env/bats/libexec

# Run bats tests
$BATS ${DIR}/cases/
