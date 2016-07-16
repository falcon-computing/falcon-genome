#!/curr/yaoh/prog/bats_install/bin/bats

@test "align without input arg" {
   run fcs-genome al  
   [ "$status" -eq 0 ]
   [ "${lines[0]}" = "USAGE:" ]
   [ "${lines[1]}" = 'fcs-genome al|align \' ]
}

fastq_dir=/merlin_fs/merlin1/hdd1/yaoh/fastq_short
fastq1=$fastq_dir/A15_100k_1.fastq
fastq2=$fastq_dir/A15_100k_2.fastq

@test "align without reference specified" {
   skip
   run fcs-genome al -fq1 $fastq1 -fq2 $fastq2 -o output.bam -rg SEQ01 -sp SEQ01 -pl ILLUMINA -lb HUMsgR2AQDCAAPE -f 
   [ "$status" -eq 0 ]
   [ "${lines[0]}" = "[fcs-genome align] WARNING: Argument '-r' is not specified, \
use /space/scratch/genome/ref/factor4/human_g1k_v37.fasta by default" ]
}

@test "align without -fq1 specified" {
   run fcs-genome al -fq2 $fastq2 -o output.bam -rg SEQ01 -sp SEQ01 \
                     -pl ILLUMINA -lb HUMsgR2AQDCAAPE -f
   [ "$status" -eq 255 ]
   [ "${lines[0]}" = "[fcs-genome align] ERROR: Argument '-fq1' is missing, please specify." ]
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

@test "markdup without input arg" {
   run fcs-genome md  
   [ "$status" -eq 0 ]
   [ "${lines[0]}" = "USAGE:" ]
   [ "${lines[1]}" = 'fcs-genome md|markDup \' ]
}


