#!/bin/bash
fetchmail --ssl --mda './mail-agent -f %F -s "cokus@ucla.edu" -i mail-agent.log'
