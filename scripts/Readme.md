##Falcon Genome Sequencing Pipeline

### Alignment
Perform pair-end alignment for input FASTQ files using `bwa mem` and `samtool sort`. 
The output is a sorted BAM file. 

#####Example
```
fcs-genome al|align \
    -r ref.fasta \
    -fq1 input_1.fastq \
    -fq2 input_2.fastq \
    -o output.bam \
    -ID RG_ID
    -SP sample_id
    -PL platform
    -LB library
```

### Mark Duplicate
Mark duplicated records off input BAM file using `picard MarkDuplicate`. The output 
is a BAM file and a duplicate statistic file with the name `output.bam.dups_stats`.

#####Example
```
fcs-genome md|markdup \
    -i input.bam \
    -o output.bam
```

### Base Recalibration
Perform recalibration of input BAM file using `GATK BaseRecalibrator` and produce a 
recalibration report for analysis. 

#####Example
```
fcs-genome bqsr|baseRecal \
    -r ref.fasta \
    -i input.bam \
    -knownSites dbsnp.vcf \
    -o output.rpt
```

### Print Reads
Apply the recalibrated score the the input BAM file using `GATK PrintReads`. 

#####Example
```
fcs-genome pr|printReads \
    -r ref.fasta \
    -i input.bam \
    -bqsr calibrate.rpt \
    -o recal_bam_dir
```

### Haploytype Caller
Perform variant calling using `GATK HaploytypeCaller`. The output BAM files are 
separate based on chromosomes and named as `output_chr[1-22 X Y MT].gvcf`, and 
put in a new directory named `output`

#####Example
```
fcs-genome haplotypeCaller \
    -r ref.fasta \
    -i sample_id \
    -c recal_bam_dir \
    -o vcf_dir
```
