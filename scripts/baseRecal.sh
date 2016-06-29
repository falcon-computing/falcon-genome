#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

if [[ $# -lt 2 ]]; then
  echo "USAGE: $0 <input.bam> <output.grp>"
  exit 1;
fi

input=$1
output=$2

check_input $input
check_output $output

export PATH=$DIR:$PATH

$JAVA -Djava.io.tmpdir=/tmp -jar ${GATK_QUEUE} \
    -S $DIR/BaseRecalQueue.scala \
    -R $ref_genome \
    -I $input \
    -knownSites $g1000_indels \
    -knownSites $g1000_gold_standard_indels \
    -knownSites $db138_SNPs \
    -o $output \
    -jobRunner ParallelShell \
    -maxConcurrentRun 32 \
    -scatterCount 32 \
    -run
