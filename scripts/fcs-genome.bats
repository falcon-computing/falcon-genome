#!/curr/yaoh/prog/bats_install/bin/bats

fastq_dir=/merlin_fs/merlin1/hdd1/yaoh/fastq_short
fastq1=$fastq_dir/A15_100k_1.fastq
fastq2=$fastq_dir/A15_100k_2.fastq
tmp_dir[1]=/merlin_fs/merlin2/ssd1/yaoh/temp2
tmp_dir[2]=/merlin_fs/merlin2/ssd2/yaoh/temp2

# align
@test "align without input arg" {
   run fcs-genome al  
   [ "$status" -eq 1 ]
   [ "${lines[0]}" = "USAGE:" ]
   [ "${lines[1]}" = 'fcs-genome al|align \' ]
}

@test "align without reference specified" {
   skip
   run fcs-genome al -fq1 $fastq1 -fq2 $fastq2 -o output.bam -rg SEQ01 -sp SEQ01 -pl ILLUMINA -lb HUMsgR2AQDCAAPE -f 
   [ "$status" -eq 0 ]
   [ "${lines[0]}" = "[fcs-genome align] WARNING: Argument '-r' is not specified, \
use /space/scratch/genome/ref/factor4/human_g1k_v37.fasta by default" ]
}

@test "align without output specified" {
    skip
   run fcs-genome al -fq1 $fastq1 -fq2 $fastq2 -rg SEQ01 -sp SEQ01 -pl ILLUMINA -lb HUMsgR2AQDCAAPE -f 
   [ "$status" -eq 0 ]
   [ "${lines[0]}" = "[fcs-genome align] WARNING: Argument '-o' is not specified, \
use ${tmp_dir[1]}/A15_100k.bam by default" ]
}

@test "align without -fq1 specified" {
   run fcs-genome al -fq2 $fastq2 -o output.bam -rg SEQ01 -sp SEQ01 \
                     -pl ILLUMINA -lb HUMsgR2AQDCAAPE -f
   [ "$status" -eq 255 ]
   [ "${lines[0]}" = "[fcs-genome align] ERROR: Argument '-fq1' is missing, please specify." ]
   [ "${lines[2]}" = "USAGE:" ]
}


@test "align without -rg specified" {
   run fcs-genome al -fq1 $fastq1 -fq2 $fastq2 -o output.bam -sp SEQ01 \
                     -pl ILLUMINA -lb HUMsgR2AQDCAAPE -f
   [ "$status" -eq 255 ]
   [ "${lines[0]}" = "[fcs-genome align] ERROR: Argument '-rg' is missing, please specify." ]
   [ "${lines[2]}" = "USAGE:" ]
}


@test "align input file dont exist" {
   run fcs-genome al -fq1 $fastq_dir/file_not_exist -fq2 $fastq2 -o output.bam -rg SEQ01 \
                     -sp SEQ01 -pl ILLUMINA -lb HUMsgR2AQDCAAPE -f
   [ "$status" -eq 1 ]
   [ "${lines[1]}" = "[fcs-genome align] ERROR: Input file $fastq_dir/file_not_exist does not exist" ]
}


@test "Output file directory dont exist" {
   run fcs-genome al -fq1 $fastq1 -fq2 $fastq2 -o dontexist/output.bam -rg SEQ01 \
                     -sp SEQ01 -pl ILLUMINA -lb HUMsgR2AQDCAAPE -f
   [ "$status" -eq 1 ]
   [ "${lines[1]}" = "[fcs-genome align] ERROR: The directory dontexist does not exist" ]
}


@test "Input file directory dont exist" {
   run fcs-genome al -fq1 dontexist/fastq1 -fq2 $fastq2 -o output.bam -rg SEQ01 \
                     -sp SEQ01 -pl ILLUMINA -lb HUMsgR2AQDCAAPE -f
   [ "$status" -eq 1 ]
   [ "${lines[1]}" = "[fcs-genome align] ERROR: The directory dontexist does not exist" ]
}

@test "output file directory not writeable" {
   run fcs-genome al -fq1 $fastq1 -fq2 $fastq2 -o /output.bam -rg SEQ01 \
		     -sp SEQ01 -pl ILLUMINA -lb HUMsgR2AQDCAAPE -f
   [ "$status" -eq 1 ]
   [ "${lines[1]}" = "[fcs-genome align] ERROR: Cannot write to folder /" ]
}

# mark duplicate
@test "markdup without input arg" {
   run fcs-genome md  
   [ "$status" -eq 1 ]
   [ "${lines[0]}" = "USAGE:" ]
   [ "${lines[1]}" = 'fcs-genome md|markDup \' ]
}


@test "markdup input dont exist" {
   run fcs-genome md -i ${tmp_dir[1]}/file_not_exist  
   [ "$status" -eq 1 ]
   [ "${lines[0]}" = '[fcs-genome markDup] ERROR: Input file /merlin_fs/merlin2/ssd1/yaoh/temp2/file_not_exist does not exist' ]
}

@test "markdup output file directory dont exist" {
   run fcs-genome md -i /merlin_fs/merlin2/ssd1/yaoh/bams_al/A15_100k.bam -o dontexist/output.bam
   [ "$status" -eq 1 ]
   [ "${lines[0]}" = "[fcs-genome markDup] ERROR: The directory dontexist does not exist" ]
}

# baseRecal
@test "baseRecal without input arg" {
   run fcs-genome bqsr
   [ "$status" -eq 1 ]
   [ "${lines[0]}" = "USAGE:" ]
   [ "${lines[1]}" = 'fcs-genome baseRecal \' ]
}

@test "baseRecal without -i specified" {
   run fcs-genome bqsr -o output.rpt
   [ "$status" -eq 255 ]
   [ "${lines[0]}" = "[fcs-genome baseRecal] ERROR: Argument '-i' is missing, please specify." ]
   [ "${lines[1]}" = "[fcs-genome baseRecal] ERROR: Failed to parse input arguments" ]
}

@test "baseRecal output file directory dont exist" {
   run fcs-genome bqsr -i /merlin_fs/merlin2/ssd1/yaoh/bams_al/A15_100k.bam -knownSites /space/scratch/genome/ref/1000G_phase1.indels.b37.vcf \
                       -o dontexist/output.rpt
   [ "$status" -eq 1 ]
   [ "${lines[1]}" = "[fcs-genome baseRecal] ERROR: The directory dontexist does not exist" ]
}

@test "baseRecal input file dont exist" {
   run fcs-genome bqsr -i /merlin_fs/merlin2/ssd1/yaoh/bams_al/dontexist -knownSites /space/scratch/genome/ref/1000G_phase1.indels.b37.vcf \
                       -o output.rpt
   [ "$status" -eq 1 ]
   [ "${lines[1]}" = "[fcs-genome baseRecal] ERROR: Input file /merlin_fs/merlin2/ssd1/yaoh/bams_al/dontexist does not exist" ]
}

# printReads
@test "printReads without input arg" {
   run fcs-genome pr
   [ "$status" -eq 1 ]
   [ "${lines[0]}" = "USAGE:" ]
   [ "${lines[1]}" = 'fcs-genome printReads \' ]
}

@test "printReads without -bqsr specified" {
   run fcs-genome pr -i /merlin_fs/merlin2/ssd1/yaoh/bams_al/A15_100k.bam -o outputdir
   [ "$status" -eq 255 ]
   [ "${lines[1]}" = "[fcs-genome printReads] ERROR: Argument '-bqsr' is missing, please specify." ]
   [ "${lines[2]}" = "[fcs-genome printReads] ERROR: Failed to parse input arguments" ]
}

@test "printReads input bqsr dont exist" {
   run fcs-genome pr -i  /merlin_fs/merlin2/ssd1/yaoh/bams_al/A15_100k.bam \
                     -bqsr /merlin_fs/merlin2/ssd1/yaoh/rpt/dontexist
   [ "$status" -eq 1 ]
   [ "${lines[2]}" = "[fcs-genome printReads] ERROR: Input file /merlin_fs/merlin2/ssd1/yaoh/rpt/dontexist does not exist" ]
}

@test "printReads without -i specified" {
   run fcs-genome pr -bqsr /merlin_fs/merlin2/ssd1/yaoh/rpt/A15_100k.markdups.bam.recal.rpt \
                     -o outputdir
   [ "$status" -eq 255 ]
   [ "${lines[1]}" = "[fcs-genome printReads] ERROR: Argument '-i' is missing, please specify." ]
   [ "${lines[2]}" = "[fcs-genome printReads] ERROR: Failed to parse input arguments" ]
   
}

@test "printReads output file directory not writeable" {
   run fcs-genome pr -i  /merlin_fs/merlin2/ssd1/yaoh/bams_al/A15_100k.bam \
                     -bqsr /merlin_fs/merlin2/ssd1/yaoh/rpt/A15_100k.markdups.bam.recal.rpt \
                     -o /
   [ "$status" -eq 1 ]
   [ "${lines[1]}" = "[fcs-genome printReads] ERROR: Cannot write to folder /" ]
}

# HaplotypeCaller
@test "HaplotypeCaller without input arg" {
   run fcs-genome hptc
   [ "$status" -eq 1 ]
   [ "${lines[0]}" = "USAGE:" ]
   [ "${lines[1]}" = 'fcs-genome hptc|haplotypeCaller \' ]
}

@test "HaplotypeCaller without -i specified" {
   run fcs-genome hptc -o outputdir
   [ "$status" -eq 255 ]
   [ "${lines[0]}" = "[fcs-genome haplotypeCaller] ERROR: Argument '-i' is missing, please specify." ]
}

@test "HaplotypeCaller input dir does not contain complete set of chromosomes" {
   run fcs-genome hptc -i /merlin_fs/merlin2/ssd1/yaoh/A15_100k_false -o outputdir
   [ "$status" -eq 1 ]
   [ "${lines[1]}" = "[fcs-genome haplotypeCaller] ERROR: Input file /merlin_fs/merlin2/ssd1/yaoh/A15_100k_false/A15_100k.chr1.bam does not exist" ]
}

@test "HaplotypeCaller output dir's father directory dont exist" {
   run fcs-genome hptc -i /merlin_fs/merlin2/ssd1/yaoh/bams_pr/A15_100k -o dontexist/outputdir
   [ "$status" -eq 1 ]
   [ "${lines[1]}" = "[fcs-genome haplotypeCaller] ERROR: The directory dontexist does not exist" ]
}

@test "HaplotypeCaller output dir not writeble" {
   run fcs-genome hptc -i /merlin_fs/merlin2/ssd1/yaoh/bams_pr/A15_100k -o /
   [ "$status" -eq 1 ]
   [ "${lines[1]}" = "[fcs-genome haplotypeCaller] ERROR: Cannot write to folder /" ]
}

# wrapper
@test "gatk wrapper help command" {
   run fcs-genome gatk --help
   [ "$status" -eq 0 ]
   [ "${lines[6]}" = "usage: java -jar GenomeAnalysisTK.jar -T <analysis_type> [-args <arg_file>] [-I <input_file>] [--showFullBamList] [-rbs " ]
}

@test "gatk BaseRecalibrator with no args input" {
   run fcs-genome gatk -T BaseRecalibrator
   [ "$status" -eq 0 ]
   [ "${lines[6]}" = "usage: java -jar GenomeAnalysisTK.jar -T <analysis_type> [-args <arg_file>] [-I <input_file>] [--showFullBamList] [-rbs " ]
}

@test "gatk BaseRecalibrator with help command" {
   run fcs-genome gatk -T BaseRecalibrator --help
   [ "$status" -eq 0 ]
   [ "${lines[6]}" = "usage: java -jar GenomeAnalysisTK.jar -T <analysis_type> [-args <arg_file>] [-I <input_file>] [--showFullBamList] [-rbs " ]
}

@test "gatk PrintReads with no args input" {
   run fcs-genome gatk -T PrintReads
   [ "$status" -eq 0 ]
   [ "${lines[6]}" = "usage: java -jar GenomeAnalysisTK.jar -T <analysis_type> [-args <arg_file>] [-I <input_file>] [--showFullBamList] [-rbs " ]
}

@test "gatk PrintReads with help command" {
   run fcs-genome gatk -T PrintReads --help
   [ "$status" -eq 0 ]
   [ "${lines[6]}" = "usage: java -jar GenomeAnalysisTK.jar -T <analysis_type> [-args <arg_file>] [-I <input_file>] [--showFullBamList] [-rbs " ]
}

@test "gatk HaplotypeCaller with no args command" {
   run fcs-genome gatk -T HaplotypeCaller
   [ "$status" -eq 0 ]
   [ "${lines[6]}" = "usage: java -jar GenomeAnalysisTK.jar -T <analysis_type> [-args <arg_file>] [-I <input_file>] [--showFullBamList] [-rbs " ]
}

@test "gatk HaplotypeCaller with help command" {
   run fcs-genome gatk -T HaplotypeCaller --help
   [ "$status" -eq 0 ]
   [ "${lines[6]}" = "usage: java -jar GenomeAnalysisTK.jar -T <analysis_type> [-args <arg_file>] [-I <input_file>] [--showFullBamList] [-rbs " ]
}
