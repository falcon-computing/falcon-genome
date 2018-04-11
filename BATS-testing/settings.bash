#Reference
ref_dir=/pool/local/ref

ref_genome="${ref_dir}/human_g1k_v37.fasta"
db138_SNPs="${ref_dir}/dbsnp_138.b37.vcf"
g1000_indels="${ref_dir}/1000G_phase1.indels.b37.vcf"
g1000_gold_standard_indels="${ref_dir}/Mills_and_1000G_gold_standard.indels.b37.vcf"

#Paths
DIR=$(cd "$( dirname "${BASHSOURCE[0]}" )" && pwd)

test_BAM="/pool/storage/niveda/Results_validation/GATK-3.8/A15_sample/A15_sample_final_BAM.bam/"
output_dir="${DIR}/output"
BATS="${DIR}/env/bats/bin/bats"
