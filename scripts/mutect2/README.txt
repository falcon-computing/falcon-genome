############### STEPS FOR SOMATIC VARIANT CALLING ###############

#For both normal.fastq and tumor.fastq:
  #Align to reference using BWA-MEM
  #Mark duplicates 
  #Recalibrate Bases
#to get BAM files- normal.recal.bam and tumor.recal.bam

#SNV call using Mutect2 to get vcf files- normal.vcf and normal_tumor.vcf
  java -jar /curr/diwu/tools/gatk-3.6/GenomeAnalysisTK.jar \
    -T MuTect2 \
    -R /pool/local/ref/human_g1k_v37.fasta \
    -I:tumor /pool/storage/diwu/annovar/LB-2907-TumorDNA.recal.bam/part-01.bam \
    -I:normal /pool/storage/diwu/annovar/LB-2907-BloodDNA.recal.bam/part-01.bam \
    --dbsnp /pool/local/ref/dbsnp_138.b37.vcf \
    #[--cosmic COSMIC.vcf] \
    -L /pool/local/ref/Refeq_Exon_Intervals_Gene.interval_list \
    -o mutect.vcf

#Filter variants to remove likely germline variants
  grep -v "REJECT" mutect.vcf > mutect_somatic.vcf

#Annotation using Annovar



