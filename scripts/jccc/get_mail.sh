#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
#fetchmail --ssl --mda './mail-agent -f %F -s "icokus@ucla.edu" -i mail-agent.log'
fetchmail --ssl --mda ''$DIR'/mail-agent -f %F -s "cokus@ucla.edu" -i '$DIR'/log/mail-agent.log -o '$DIR'/log/mails' > /dev/null
