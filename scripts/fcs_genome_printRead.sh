#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -i|--input_base)
    input_base="$2"
    shift # past argument
    ;;
    -c|--chr_dir)
    chr_dir="$2"
    shift # past argument
    ;;
    -r|--rpt_dir)
    rpt_dir="$2"
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
  echo "USAGE: fcs_genome printread -i <input_base> -c <chr_dir> -r <rpt_dir> -o <output_dir>"
  exit 1;
fi

if [ -z $input_base ];then
  echo "input_base is not specified, please check the command"
  exit 1
fi

if [ -z $chr_dir ];then
  echo "chr_dir is not specified, please check the command"
  exit 1
fi

if [ -z $rpt_dir ];then
  rpt_dir=$output_dir/rpt
  create_dir $rpt_dir
  echo "rpt_dir is not set,will generate rpt in $rpt_dir"
fi

if [ -z $output ];then
  create_dir ${tmp_dir[2]}
  output=${tmp_dir[2]}
  echo "Output file name is not set, the output file is stored to "$output" as default"
fi

# Check the inputs

chr_list="$(seq 1 22) X Y MT"
for chr in $chr_list; do
  chr_bam=$chr_dir/${input_base}.bam.markdups.chr${chr}.bam
  if [ ! -f $chr_bam ];then 
    echo "Could not find the ${chr}.bam"
    exit 1
  fi
done

# Start the manager
declare -A pid_table

# Start manager
start_ts=$(date +%s)
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

# Start the jobs
for chr in $chr_list; do
    # The splited bams are in tmp_dir[1], the recalibrated bams should be in [2]
    chr_bam=${tmp_dir[1]}/${input_base}.bam.markdups.chr${chr}.bam
    chr_rpt=$rpt_dir/${input_base}.bam.markdups.bam.recalibration_report.grp
    chr_recal_bam=${tmp_dir[2]}/${input_base}.bam.recal.chr${chr}.bam

    $DIR/fcs-sh "$DIR/printReads.sh $chr_bam $chr_rpt $chr_recal_bam $chr" 2> $log_dir/printReads_chr${chr}.log &
    pid_table["$chr"]=$!
  done
  # Wait on all the tasks
  for pid in ${pid_table[@]}; do
    wait "${pid}"
  done
  end_ts=$(date +%s)

  for chr in $chr_list; do
    chr_bam=${tmp_dir[1]}/${input_base}.bam.markdups.chr${chr}.bam
    chr_recal_bam=${tmp_dir[2]}/${input_base}.bam.recal.chr${chr}.bam
    if [ ! -e ${chr_recal_bam}.done ]; then
      exit 4;
    fi
    rm ${chr_recal_bam}.done
    rm $chr_bam &
    rm ${chr_bam}.bai &
    rm ${chr_recal_bam}.bai &
    cp $chr_recal_bam $bam_dir
  done
  echo "PrintReads stage finishes in $((end_ts - start_ts))s"

# Stop manager
kill $manager_pid




