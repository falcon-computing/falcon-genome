#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/settings.sh
source $DIR/common.sh

start_manager "demux-manager"
start_manager "ssheet-manager"
start_manager "rsync-manager"
