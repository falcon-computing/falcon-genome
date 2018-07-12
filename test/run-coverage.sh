#!/bin/bash
CURR_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $CURR_DIR/global.bash

source $FALCON_DIR/setup.sh

#Run results validation tests
while read id; do
  export id=$id
  $CURR_DIR/../bats/bats cases/depthOfCoverage.bats >> test.log
done <$data_list
