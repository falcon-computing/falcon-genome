#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -i|--input)
    input="$2"
    shift # past argument
    ;;
    -o|--output)
    output="$2"
    shift # past argument
    ;;
    -h|--help)
    help_req=YES
    ;;
    *)
            # unknown option
    ;;
esac
shift # past argument or value
done

# Check the command
if [ ! -z $help_req ];then
  echo "USAGE: fcs_genome bqsr -i <input> "
  exit 1;
fi

if [ -z $input ];then
  echo "The input arg is missing, please check the cmd"
  exit 1;
fi

if [ -z $output ];then
  rpt_dir=$output_dir/rpt
  create_dir $rpt_dir
  output=$rpt_dir/$(basename $input).recalibration_report.grp
  echo "The output dir is not specified, would put the result to $output"
fi

check_input $input
check_output $output

# Table storing all the pids for tasks within one stage
declare -A pid_table

# Start manager
if [ -e "host_file" ]; then
  host_file=host_file
else
  host_file=$DIR/host_file
fi
echo "$DIR/manager/manager --v=1 --log_dir=. $host_file"
$DIR/manager/manager --v=1 --log_dir=. $host_file &
manager_pid=$!
sleep 1
if [[ ! $(ps -p "$manager_pid" -o comm=) =~ "manager" ]]; then
  echo "Cannot start manager, exiting"
  exit -1
fi


export PATH=$DIR:$PATH

$JAVA -Djava.io.tmpdir=/tmp -jar ${GATK_QUEUE} \
    -S $DIR/BaseRecalQueue.scala \
    -R $ref_genome \
    -I $input \
    -knownSites $g1000_indels \
    -knownSites $g1000_gold_standard_indels \
    -knownSites $db138_SNPs \
    -o $output \
    -jobRunner ParallelShell \
    -maxConcurrentRun 32 \
    -scatterCount 32 \
    -run




# Stop manager
kill $manager_pid
