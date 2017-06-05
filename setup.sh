#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )


export PATH=$DIR/bin:$PATH
export PATH=$DIR/tools/bin:$PATH
export PATH=$DIR/tools/rtg-tools:$PATH

export LD_LIBRARY_PATH=$DIR/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/curr/diwu/tools/htslib-1.3.1
