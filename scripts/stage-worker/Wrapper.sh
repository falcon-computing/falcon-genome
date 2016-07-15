#####################################################################
## This script provide some wrapper function if gatk and picard
#####################################################################
#!/bin/bash

# Import global variables
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/../globals.sh
source $DIR/common.sh

stage_name=wrapper

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
if [ $command == "gatk" ];then
   # Get the gatk command, the while loop detects the -T incase user dont put -T toolname as the first arg
   while [ $counter -lt $# ] ; do
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
   echo  "gatk_command is $gatk_command" 
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
     echo "$args"
     fcs-genome $gatk_command "$args"
   ;;
   *)
     shift
     $JAVA -jar $GATK $@
   ;; 
   esac
else 
   # Get the picard command
   picard_command=$2
   if [ $picard_command == MarkDuplicates ];then    
     # Detect the command
     while [[ $# -gt 0 ]];do
        key="$1"
        case $key in
        INPUT=*)
          markdup_input=${$key#*INPUT=}
        ;;
        OUTPUT=*)
          markdup_output=${$key#*OUTPUT=}
        ;;
        esac
        shift
      done
      fcs-genome -i $markdup_input -o markdup_output
    else
      shift
      $JAVA -jar $PICARD $@
    fi
fi
