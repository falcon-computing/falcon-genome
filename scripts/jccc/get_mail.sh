#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
#fetchmail --ssl --mda './mail-agent -f %F -s "icokus@ucla.edu" -i mail-agent.log'
fetchmail --ssl --mda ''$DIR'/mail-agent -f %F -s "lordwudee@gmail.com" -i '$DIR'/mail-agent.log' > /dev/null
