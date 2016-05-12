server="bmap" # either "hoffman" or "bmap"

if [ $server = "hoffman" ]; then
# NGS Home Directory
ngs_dir="/u/project/eeskin/geschwind/NGS"

# Reference Genome Info
ref_genome_dir="$ngs_dir/reference_genome"
ref_genome="$ref_genome_dir/hs37d5_normalized.fa"
db138_SNPs="$ref_genome_dir/dbsnp_138.b37.vcf"
g1000_indels="$ref_genome_dir/1000G_phase1.indels.b37.vcf"
g1000_gold_standard_indels="$ref_genome_dir/Mills_and_1000G_gold_standard.indels.b37.vcf"
hapmap="$ref_genome_dir/hapmap_3.3.b37.vcf"
g1000_omni2_5="$ref_genome_dir/1000G_omni2.5.b37.vcf"

# Directories
ngs_raw_data_dir="$ngs_dir/NGS_raw_data"
ngs_code_dir="$ngs_dir/NGS_code"
ngs_sample_info_dir="$ngs_dir/NGS_sample_info"

# Binary application calls
local_apps_dir="/u/local/apps"
geschwind_programs_dir="/u/nobackup/dhg/Software"
anaconda_python_call="$geschwind_programs_dir/anaconda/bin/python"
bamtools_call="$geschwind_programs_dir/bamtools/bin/bamtools"
bedtools_call="$local_apps_dir/bedtools/2.23.0/gcc-4.4.7/bin/bedtools"
bedtools_multi_intersect_call="$local_apps_dir/bedtools/2.23.0/gcc-4.4.7/bin/multiIntersectBed"
bwa_call="$local_apps_dir/bwa/current/bwa"
fastqc_call="/u/home/n/nkwok/FastQC/fastqc"
gatk_call="$local_apps_dir/gatk/3.3-0/GenomeAnalysisTK.jar"
picard_markdups_call="$local_apps_dir/picard-tools/current/MarkDuplicates.jar"
java_call="/usr/bin/java"
samtools_call="$geschwind_programs_dir/samtools-1.2/samtools"

# .bed files
QC_supplemental_files_dir="$ngs_code_dir/QC_supplemental_input_files"
genome_no_gaps_bedfile="$QC_supplemental_files_dir/genome.noGapregions.noChr.bed"
genome_exon_bedfile="$QC_supplemental_files_dir/gencode.v19.annotation.protein-coding.exons.noChr.bed"
exome_no_gaps_bedfile="$QC_supplemental_files_dir/SeqCap_EZ_Exome_v2_noCHR.bed"
exome_exon_bedfile="$QC_supplemental_files_dir/Exome_GencodeExon_Overlap_noCHR.bed"


function construct_qsub {
	qsub_call="qsub -cwd -N $1 -l h_data=2G,h_rt=08:00:00,highp -pe shared 8 -e $2 -o $2"

	if [ $4 ]; then
		qsub_call="$qsub_call -hold_jid $4"
	fi
	qsub_call="$qsub_call $3"
	eval $qsub_call
}

fi

if [ $server = "bmap" ]; then

# Home Directory
ngs_dir="/ifs/collabs/geschwind/NGS"

# Reference Genome Info
ref_genome_dir="$ngs_dir/reference_genome"
ref_genome="$ref_genome_dir/hs37d5_normalized.fa"
db138_SNPs="$ref_genome_dir/dbsnp_138.b37.vcf"
g1000_indels="$ref_genome_dir/1000G_phase1.indels.b37.vcf"
g1000_gold_standard_indels="$ref_genome_dir/Mills_and_1000G_gold_standard.indels.b37.vcf"
hapmap="$ref_genome_dir/hapmap_3.3.b37.vcf"
g1000_omni2_5="$ref_genome_dir/1000G_omni2.5.b37.vcf"

# Directories
ngs_raw_data_dir="$ngs_dir/NGS_raw_data"
ngs_code_dir="$ngs_dir/NGS_code"
ngs_sample_info_dir="$ngs_dir/NGS_sample_info"
## JUST FOR TESTING
ngs_raw_data_dir="/ifshome/nkwok/scripts"
ngs_sample_info_dir="/ifshome/nkwok/scripts/sample_info"
# Binary application calls
geschwind_programs_dir="/ifs/collabs/geschwind/programs"
anaconda_python_call="/ifshome/nkwok/anaconda/bin/python"
bamtools_call="$geschwind_programs_dir/bamtools/bamtools/bin/bamtools"
bedtools_call="$geschwind_programs_dir/bedtools/bedtools2/bin/bedtools"
bedtools_multi_intersect_call="$geschwind_programs_dir/bedtools/bedtools2/bin/multiIntersectBed"
bwa_call="$geschwind_programs_dir/bwa/bwa-0.7.10/bwa"
fastqc_call="$geschwind_programs_dir/fastqc/fastqc/fastqc"
gatk_call="$geschwind_programs_dir/gatk/GenomeAnalysisTK.jar"
picard_markdups_call="$geschwind_programs_dir/picard/picard-tools-1.119/MarkDuplicates.jar"
java_call="/usr/local/java/jdk1.7.0_45/bin/java"
samtools_call="/usr/local/loniWorkflows/Bioinformatics/samtools-0.1.19/samtools"

 # .bed files
 QC_supplemental_files_dir="$ngs_code_dir/QC_supplemental_input_files"
 genome_no_gaps_bedfile="$QC_supplemental_files_dir/genome.noGapregions.noChr.bed"
 genome_exon_bedfile="$QC_supplemental_files_dir/gencode.v19.annotation.protein-coding.exons.noChr.bed"
 exome_no_gaps_bedfile="$QC_supplemental_files_dir/SeqCap_EZ_Exome_v2_noCHR.bed"
 exome_exon_bedfile="$QC_supplemental_files_dir/Exome_GencodeExon_Overlap_noCHR.bed"

function construct_qsub {
	qsub_call="qsub -cwd -N $1 -l h_vmem=8.625G,h_stack=8.625 -e $2 -o $2 -q long.q"
	if [ $4 ]; then
		qsub_call="$qsub_call -hold_jid $4"
	fi
	qsub_call="$qsub_call $3"
	eval $qsub_call
}

fi
