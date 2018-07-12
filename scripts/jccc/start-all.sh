#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/settings.sh
source $DIR/common.sh

start_manager "compute-manager" $result_dir
start_manager "demux-manager" $fastq_dir
start_manager "ssheet-manager" $run_dir
start_manager "rsync-manager" $run_dir
