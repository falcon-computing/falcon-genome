
#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source $DIR/globals.sh

while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -r|--ref)
    ref_fasta="$2"
    shift # past argument
    ;;
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
    -v|--verbose)
    verbose="$2"
    shift
    ;;
    -clean)
    clean_flag="$2"
    shift
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
  echo " USAGE: fcs_genome haplotypecaller -r <ref.fasta> -i <input_base> -c <chr_dir> -o <output_dir>"
  echo " The <input_base> argument is the basename of the input bams, shoud not contain suffixes"
  echo " The <chr_dir> argument is the directory to find the input bams"
  echo " The <output_dir> argument is the directory to put the output vcf results"
#  echo " The <verbose> argument is the verbose level of the run, verbose=0 means quiet \
#and no output, verbose=1 means output errors, verbose=2 means detailed information. By default it is set to 1"
#  echo " The clean option tells if the program should keep the input , if set to 1, we would \
#remove the input, by default it's set to 0 "
  exit 1;
fi

if [ -z $ref_fasta ];then
  ref_fasta=$ref_genome
  echo "The reference fasta is not specified, by default we would use $ref_fasta"
  echo "You should use -r <ref.fasta> to specify the reference"
fi

if [ -z $input_base ];then
  echo "input_base is not specified, please check the command"
  echo "USAGE: fcs_genome haplotypecaller -i <input_base> -c <chr_dir> -o <output_dir>"
  exit 1
fi

if [ -z $chr_dir ];then
  echo "chr_dir is not specified, please check the command"
  echo "USAGE: fcs_genome haplotypecaller -i <input_base> -c <chr_dir> -o <output_dir>"
  exit 1
fi

if [ -z $vcf_dir ];then
  vcf_dir=$output_dir/vcf
  create_dir $vcf_dir
  echo "Output directory is not set, the output file is stored to $vcf_dir as default"
fi

if [ -z $verbose ];then
  verbose=1;
  echo "In default verbose is 1"
  echo "If you want to set it, use the -v <verbose> option "
fi

if [ -z $chean_flag ];then
  clean_flag=0;
  echo "In default clean_flag is 0, the input would be kept"
  echo "If you want to set it, use the -clean option "
fi

# Check the inputs
chr_list="$(seq 1 22) X Y MT"
for chr in $chr_list; do
  chr_bam=$chr_dir/${input_base}.recal.chr${chr}.bam
  if [ ! -f $chr_bam ];then 
    >&2 echo "Could not find the $chr_bam, please check if you have pointed the right directory"
    exit 1
  fi
done
chr_vcf=$vcf_dir/${input_base}_chr1.gvcf
check_output $chr_vcf
rm $vcf_dir/${input_base}_chr*.gvcf>/dev/null

# Create the directorys
hptc_log_dir=$log_dir/hptc
hptc_manager_dir=$hptc_log_dir/manager
create_dir $log_dir
create_dir $hptc_log_dir
create_dir $hptc_manager_dir

# Start the manager
declare -A pid_table

# Start manager
start_ts=$(date +%s)
if [ -e "host_file" ]; then
  host_file=host_file
else
  host_file=$DIR/host_file
fi
echo "$DIR/manager/manager --v=1 --log_dir=$hptc_manager_dir/ $host_file"
$DIR/manager/manager --v=1 --log_dir=$hptc_manager_dir/ $host_file &
manager_pid=$!
sleep 1
if [[ ! $(ps -p "$manager_pid" -o comm=) =~ "manager" ]]; then
  echo "Cannot start manager, exiting"
  exit -1
fi
# Catch the interrupt option to stop manager
terminate(){
  echo "Caught interruption, cleaning up"
  # Stop manager
  kill "$manager_pid"
  echo "Stopped manager"
  # Stop stray
  for pid in ${pid_table[@]}; do
    ps -p $pid>/dev/null
    if [ "$?" == 0 ];then 
      kill "${pid}"
      echo "kill $pid"
    fi
  done
  # Check run dir
  for file in ${hptc_output_table[@]}; do
   if [ -e ${file}.java.pid ]; then
     hptc_java_pid=$(cat ${file}.java.pid)
     kill "$hptc_java_pid"
     rm ${file}.java.pid
     echo "Stopped remote java process $hptc_java_pid for $file"
   fi
   if [ -e ${file}.pid ]; then
     hptc_pid=$(cat ${file}.pid)
     kill "$hptc_pid"
     rm ${file}.pid
     echo "Stopped remote haplotypecaller process $hptc_pid for $file"
   fi
  done
  echo "Exiting"
  exit 1;
}

trap "terminate" 1 2 3 15

# Start the jobs


for chr in $chr_list; do
  chr_bam=$chr_dir/${input_base}.recal.chr${chr}.bam
  chr_vcf=$vcf_dir/${input_base}_chr${chr}.gvcf
  $DIR/fcs-sh "$DIR/haploTC.sh $chr $chr_bam $chr_vcf $ref_fasta" 2> $hptc_log_dir/haplotypeCaller_chr${chr}.log &
  pid_table["$chr"]=$!
  hptc_output_table["$chr"]=$chr_vcf
done

# Wait on all the tasks
for pid in ${pid_table[@]}; do
  wait "${pid}"
done
unset hptc_output_table

for chr in $chr_list; do
  chr_bam=$chr_dir/${input_base}.bam.recal.chr${chr}.bam
  chr_vcf=$vcf_dir/${input_base}_chr${chr}.gvcf
  if [ ! -e ${chr_vcf}.done ]; then
    exit 5;
  fi
  if [ $clean_flag == 1 ]; then
  rm ${chr_vcf}.done
  rm $chr_bam 
  fi
done
 
# Stop manager
kill $manager_pid
end_ts=$(date +%s)

echo "HaplotypeCaller stage finishes in $((end_ts - start_ts))s"
echo "Please find the results in $vcf_dir dir"
