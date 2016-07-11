##Falcon Genome Sequencing Pipeline

### Alignment
Perform pair-end alignment for input FASTQ files using `bwa mem` and `samtool sort`. 
The output is a sorted BAM file. 

#####Example
```
fcs-genome align \
    -r ref.fasta \
    -fq1 input_1.fastq \
    -fq2 input_2.fastq \
    -o output.bam \
    -R "@RG\tID:rg_id\tSM:sample_id\tPL:platform\tLB:library"
```

### Mark Duplicate
Mark duplicated records off input BAM file using `picard MarkDuplicate`. The output 
is a BAM file and a duplicate statistic file with the name `output.bam.dups_stats`.

#####Example
```
fcs-genome markDuplicate \
    -i input.bam \
    -o output.bam
```

### Base Recalibration
Perform recalibration of input BAM file using `GATK BaseRecalibrator` and produce a 
recalibration report for analysis. 

#####Example
```
fcs-genome baseRecal \
    -r ref.fasta \
    -i input.bam \
    -knownSites dbsnp.vcf \
    -o output.rpt
```

### Print Reads
Apply the recalibrated score the the input BAM file using `GATK PrintReads`. 

#####Example
```
fcs-genome printReads \
    -r ref.fasta \
    -i input.bam \
    -bqsr calibrate.rpt \
    -o output.bam
```

### Haploytype Caller
Perform variant calling using `GATK HaploytypeCaller`. The output BAM files are 
separate based on chromosomes and named as `output_chr[1-22 X Y MT].gvcf`, and 
put in a new directory named `output`

#####Example
```
fcs-genome haplotypeCaller \
    -r ref.fasta \
    -i input.bam \
    -o output
```
