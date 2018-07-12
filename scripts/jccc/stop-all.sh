#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/settings.sh
source $DIR/common.sh

stop_manager "rsync-manager"
stop_manager "ssheet-manager"
stop_manager "demux-manager"
stop_manager "compute-manager"
