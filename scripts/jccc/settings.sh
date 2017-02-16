
samplesheet_loc=wudee@hoffman2.idre.ucla.edu:/u/nobackup/lixuser/shared
remote_fastq_loc=wudee@hoffman2.idre.ucla.edu:/u/nobackup/lixuser/shared/fastq

tmp_dir=/genome_fs/local/temp
ref_dir=/genome_fs/local/ref
ref_genome=$ref_dir/human_g1k_v37.fasta
db138_SNPs=$ref_dir/dbsnp_138.b37.vcf
g1000_indels=$ref_dir/1000G_phase1.indels.b37.vcf
g1000_gold_standard_indels=$ref_dir/Mills_and_1000G_gold_standard.indels.b37.vcf

log_dir=$(pwd)/log
run_dir=/genome_fs/space-a/run
fastq_dir=/genome_fs/local/run
