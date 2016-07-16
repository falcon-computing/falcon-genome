#####################################################################
## This script provide some wrapper function if gatk and picard
#####################################################################
#!/bin/bash

# Import global variables
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

# Prevent this script to be running alone
if [[ $0 != ${BASH_SOURCE[0]} ]]; then
  # Script is sourced by another shell
  cmd_name=`basename $0 2> /dev/null`
  if [[ "$cmd_name" != "fcs-genome" ]]; then
    log_error "This script should be started by 'fcs-genome'"
    return 1
  fi
else
  # Script is executed directly
  log_error "This script should be started by 'fcs-genome'"
  exit 1
fi

command=$1
stage_name=$command

counter=1

filter_args(){
  for arg in $@; do
    if [ $arg == "-T" -o $arg == $filter_arg -o $arg == "gatk" ];then
       args=$args
    else
       args="$args $arg"
    fi
  done
}

if [ $command == "gatk" ]; then
  # Get the gatk command, the while loop detects the -T incase user dont put -T toolname as the first arg
  while [ $counter -lt $# ]; do
    eval "str=\$$counter"
    if [ $str == "-T" ];then
      counter=$[$counter+1]
      eval "str=\$$counter"
      gatk_command=$str
      break
    else
      counter=$[$counter+1]
    fi
  done
  log_debug "gatk_command is $gatk_command" 

  # Pass the command
  case $gatk_command in
  BaseRecalibrator|PrintReads|HaplotypeCaller)
    # Filter the duplicate command
    shift
    for arg in $@; do
      if [ $arg == "-T" -o $arg == $gatk_command ];then
         args=$args
      else
         args="$args $arg"
      fi
    done
    # Start fcs-genome baseRecal
    log_info "Using fcs-genome $gatk_command instead"
    log_debug "$args"
    $DIR/../fcs-genome $gatk_command "$args"
    if [ "$?" -ne 0 ]; then 
      exit $?
    fi
    ;;
  *)
    shift
    $JAVA -jar $GATK $@
    if [ "$?" -ne 0 ]; then 
      log_error "$gatk_command failed"
      exit 1
    fi
    ;; 
  esac
elif [ $command == "picard" ]; then
  # Get the picard command
  picard_command=$2
  if [ $picard_command == "MarkDuplicates" ];then    
    # Detect the command
    while [[ $# -gt 0 ]];do
      key="$1"
      case $key in
      INPUT=*)
        markdup_input=${key#*INPUT=}
        #echo "$markdup_input"
        ;;
      OUTPUT=*)
        markdup_output=${key#*OUTPUT=}
        ;;
      esac
      shift
    done
    $DIR/../fcs-genome md -i $markdup_input -o $markdup_output
    if [ "$?" -ne 0 ]; then 
      exit $?
    fi
  else
    shift
    $JAVA -jar $PICARD $@
    if [ "$?" -ne 0 ]; then 
      log_error "$gatk_command failed"
      exit 1
    fi
  fi
else 
  log_debug "Never here."
fi
