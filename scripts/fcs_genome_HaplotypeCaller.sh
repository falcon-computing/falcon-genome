
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
    -o|--output)
    vcf_dir="$2"
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

if [ ! -z $help_req ];then
  echo "USAGE: fcs_genome haplotypecaller -i <input_base> -c <chr_dir> -r -o <output_dir>"
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

if [ -z $vcf_dir ];then
  vcf_dir=$output_dir/vcf
  create_dir $vcf_dir
  echo "Output file name is not set, the output file is stored to $vcf_dir as default"
fi

# Check the inputs
chr_list="$(seq 1 22) X Y MT"
for chr in $chr_list; do
  chr_bam=$chr_dir/${input_base}.bam.recal.chr${chr}.bam
  if [ ! -f $chr_bam ];then 
    echo "Could not find the $chr_bam"
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
  chr_bam=$chr_dir/${input_base}.bam.recal.chr${chr}.bam
  chr_vcf=$vcf_dir/${input_base}_chr${chr}.gvcf
  $DIR/fcs-sh "$DIR/haploTC.sh $chr $chr_bam $chr_vcf" & #2> $log_dir/haplotypeCaller_chr${chr}.log &
  pid_table["$chr"]=$!
done

# Wait on all the tasks
for pid in ${pid_table[@]}; do
  wait "${pid}"
done

for chr in $chr_list; do
  chr_bam=$chr_dir/${input_base}.bam.recal.chr${chr}.bam
  chr_vcf=$vcf_dir/${input_base}_chr${chr}.gvcf
  if [ ! -e ${chr_vcf}.done ]; then
    exit 5;
  fi
  rm ${chr_vcf}.done
  rm $chr_bam 
done
 
# Stop manager
kill $manager_pid
