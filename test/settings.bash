#Reference
ref_dir=/pool/local/ref/

ref_genome="${ref_dir}/human_g1k_v37.fasta"
db138_SNPs="${ref_dir}/dbsnp_138.b37.vcf"
g1000_indels="${ref_dir}/1000G_phase1.indels.b37.vcf"
g1000_gold_standard_indels="${ref_dir}/Mills_and_1000G_gold_standard.indels.b37.vcf"

#Paths
DIR=$(cd "$( dirname "${BASHSOURCE[0]}" )" && pwd)
work_dir="${DIR}/workdir"
output_dir="${DIR}/output"
mkdir -p $work_dir
mkdir -p $output_dir

cosmic=$work_dir/b37_cosmic_v54_120711.vcf

BATS="${DIR}/env/bats/bin/bats"
FCSBIN="${DIR}/../bin/fcs-genome"

test_fastq="${DIR}/daily/data"
output_unwritable="${DIR}/mutect2/output_unwritable"
wrong_ref="${DIR}/mutect2/"


S9_BAM=/pool/storage/niveda/Mutect2/concat/S9_BAM.bam/part-00.bam 
S10_BAM=/pool/storage/niveda/Mutect2/concat/S10_BAM.bam/part-00.bam

# Download fastq and baselines for input
input_dir="$work_dir/input"
mkdir -p $input_dir
#aws s3 cp s3://fcs-genome-data/data-suite/Performance-testing/daily/A15_sample_1.fastq.gz $input_dir
#aws s3 cp s3://fcs-genome-data/data-suite/Performance-testing/daily/A15_sample_2.fastq.gz $input_dir
#aws s3 cp --recursive s3://fcs-genome-data/Validation-baseline/GATK-3.8/A15_sample/ $input_dir

aws s3 sync s3://fcs-genome-data/ref/b37_cosmic_v54_120711.vcf $work_dir
aws s3 sync s3://fcs-genome-data/ref/b37_cosmic_v54_120711.vcf.idx $work_dir
