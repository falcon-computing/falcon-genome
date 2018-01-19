# fcs-genome pipeline
  
## Description
Variant calling pipeline for germline mutations adopting GATK's Best Practices along with Falcon's FPGA acceleration techniques to significantly improve performance.
## Synopsis
```
fcs-genome align -r ref.fasta -1 input_1.fastq -2 input_2.fastq -o aln.sorted.bam \
  --rg RG_ID --sp sample_id --pl platform --lb library 

fcs-genome markdup -i aln.sorted.bam -o aln.marked.bam 

fcs-genome indel -r ref.fasta -i aln.sorted.bam -o indel.bam

fcs-genome bqsr -r ref.fasta -i indel.bam -o recal.bam

fcs-genome baserecal -r ref.fasta -i indel.bam -o recalibration_report.grp 

fcs-genome printreads -r ref.fasta -b recalibration_report.grp -i indel.bam -o recal.bam 

fcs-genome htc -r ref.fasta -i recal.bam -o final.gvcf

fcs-genome joint -r ref.fasta -i final.gvcf -o final.vcf 

fcs-genome ug -r ref.fasta -i recal.bam -o final.vcf

fcs-genome gatk -T analysisType 
```

## Case Example: Same Sample Data Stored in Multiple FASTQ files
#### Description
In cases where the same sample is distributed over multiple lanes in the sequencing machine- thereby having different read group names- multiple FASTQ files will correspond to that sample. In such cases, alignment for each paired-end sequence data is run in parallel, with the results stored in a parent directory and each file named by the convention part-n. This entire directory is then taken as input for MarkDuplicates, resulting in a combined, sorted BAM file with the duplicates marked.  
#### Example
```
for i in $(seq 0 $((num_groups - 1))); do 
  #Each paired sequence data, represented by fastq_1 and fastq_2 are aligned iteratively
  #The output is stored in the directory $tmp_dir/$sample_id with the file name part-n
  
  fastq_1=${fastq_files[$(($i * 2))]}           
  fastq_2=${fastq_files[$(($i * 2 + 1))]}

  read_group=`echo $(basename $fastq_1) | sed 's/\_R1.*//'`
  library=$sample_id

  fcs-genome align \
      -1 $fastq_1 \
      -2 $fastq_2 \
      -o $tmp_dir/$sample_id \
      -r $ref_genome \
      --align-only \
      -S $sample_id \
      -R $read_group \
      -L $library \
      -P Illumina \
      -f 2>> $log_file
done

#Each alignment file, following the part-n naming convention, is taken as input for MarkDuplicates
#The output is a combined, sorted BAM file- $sample_id.bam

fcs-genome markDup \
    -i ${tmp_dir}/$sample_id \
    -o ${tmp_dir}/${sample_id}.bam \
    -f 2>> $log_file 
```

## Commands and Options
### Alignment
```
fcs-genome align <options>
```
#### Description
Equivalent to BWA-MEM, this command maps pair-ended FASTQ sequences against a large reference genome sequence. The resulting BAM file is sorted, with duplicates marked. 
#### Options
| Command | Description |
| --- | --- |
| -h [--help] | print help messages |
| -f [--force] | overwrite output files if they exist |
| -O [--extra-options] | the "[key] [value]" arguments from the user is used as arguments for GATK |
| -r [--ref] arg | reference genome path |
| -1 [--fastq1] arg | input pair-end fastq file |
| -2 [--fastq2] arg | input pair-end fastq file |
| -o [--output] arg | output BAM file (if --align-only is set, the output will be a directory of BAM files |
| -R [--rg] arg | read group ID ('ID' in BAM header) |
| -S [--sp] arg | sample ID ('SM' in BAM header) |
| -P [--pl] arg | platform ID ('PL' in BAM header) |
| -L [--lb] arg | library ID ('LB' in BAM header) |
| -l [--align-only] | skip mark duplicates |

---
### Mark Duplicates 
```
fcs-genome markdup <options>
```
#### Description
Equivalent to Picard's MarkDuplicates, this tool tags duplicate reads in a BAM file. Duplicate reads refer to those that originate in a single fragment of DNA.
#### Options
| Command | Description |
| --- | --- |
| -h [--help] | print help messages |
| -f [--force] | overwrite output files if they exist |
| -O [--extra-options] | the "[key] [value]" arguments from the user is used as arguments for GATK |
| -i [--input] arg | input file |
| -o [--output] arg | output file |

---
### Indel Realignment
```
fcs-genome indel <options>
```
#### Description
Equivalent to GATK IndelRealigner. This command takes a BAM file as an input and performs local realignment of reads. Presence of  insertions or deletions in the genome compared to the reference genome may be the cause of mismatches in the alignment. To prevent these from being mistaken as SNP's, this step is done.
#### Options
| Command | Description |
| --- | --- |
| -h [--help] | print help messages |
| -f [--force] | overwrite output files if they exist |
| -O [--extra-options] | the "[key] [value]" arguments from the user is used as arguments for GATK |
| -r [--ref] arg | reference genome path |
| -i [--input] arg | input BAM file or dir |
| -o [--output] arg | output directory of BAM files |
| -K [--known] arg | known indels for realignment|

---
### Base Recalibration + Print Reads
```
fcs-genome bqsr <options>
```
#### Description
The equivalent of GATK's BaseRecalibrator followed by GATK's PrintReads, this command implements Base Quality Score Recalibration (BQSR) and outputs the result in recalibrated BAM files.
#### Options
| Command | Description |
| --- | --- |
| -h [--help] | print help messages |
| -f [--force] | overwrite output files if they exist |
| -O [--extra-options] | the "[key] [value]" arguments from the user is used as arguments for GATK |
| -r [--ref] arg | reference genome path |
| -b [-bqsr] arg | output BQSR file (if left blank, no file will be produced) |
| -i [--input] arg | input BAM file or dir |
| -o [--output] arg | output directory of BAM files |
| -K [--knownSites] arg | known sites for base recalibration |

---
### Base Recalibration 
```
fcs-genome baserecal <options>
```
#### Description
This equivalent of GATK's BaseRecalibrator gives per-base score estimates of errors caused by sequencing machines. Taking an input of BAM files containing data that requires recalibration, the output file is a table generated based on user-specified covariates such as read group and reported quality score.  
#### Options
| Command | Description |
| --- | --- |
| -h [--help] | print help messages |
| -f [--force] | overwrite ouput files, if they exist |
| -O [--extra-options] | the "[key] [value]" arguments from the user is used as arguments for GATK |
| -r [--ref] arg | reference genome path |
| -i [--input] arg | input BAM file or dir |
| -o [--output] arg | output BQSR file |
| -K [--knownSites] arg | known sites for base recalibration |

---
### Print Reads
```
fcs-genome printreads <options>
```
#### Description
Equivalent to GATK's PrintReads, this tool manipulates BAM files. It takes the output of BQSR and one or more BAM files to result in processed and recalibrated BAM files.
#### Option
| Command | Description |
| --- | --- |
| -h [--help] | print help messages |
| -f [--force] | overwrite output files if they exist |
| -O [--extra-options] | the "[key] [value]" arguments from the user is used as arguments for GATK |
| -r [--ref] arg | reference genome path |
| -b [--bqsr] arg | input BQSR file |
| -i [--input] arg | input BAM file or dir |
| -o [--output] arg | output BAM files |

---
### Haplotype Caller
```
fcs-genome htc <options>
```
#### Description
Equivalent to GATK's Haplotype Caller, this tool calls germline SNPs and indels through local de-novo assembly of haplotypes in regions that show variation from reference, producing a genomic VCF (gVCF) file. To get a VCF file as output, include the option --produce-vcf.
#### Options
| Command | Description |
| --- | --- |
| -h [--help] | print help messages |
| -f [--force] | overwrite output files if they exist |
| -O [--extra-options] | the "[key] [value]" arguments from the user is used as arguments for GATK |
| -r [--ref] arg | reference genome path |
| -i [--input] arg | input BAM file or dir |
| -o [--output] arg | output GVCF file |
| -v [--produce-vcf] | produce VCF files from Haplotype Caller instead of GVCF |

---
### Joint Genotyping
``` 
fcs-genome joint <options>
```
#### Description
Equivalent of GATK's GenotypeGVCFs, this tool takes in gVCF files as input. The files are then merged, re-genotyped and re-annotated resulting in a combined, genotyped VCF file.
#### Options
| Command | Description |
| --- | --- |
| -h [--help] | print help messages |
| -f [--force] | overwrite output files if they exist |
| -O [--extra-options] | the "[key] [value]" arguments from the user is used as arguments for GATK |
| -r [--ref] arg | reference genome path |
| -i [--input-dir] arg | input dir containing [sample_id].gvcf.gz files |
| -o [--output] arg | output vcf.gz file(s) |
| -c [--combine-only] | combine GVCFs only and skip genotyping |
| -g [--skip-combine] | genotype GVCFs only and skip combining (for single sample) |

---
### Unified Genotyper
```
fcs-genome ug <options>
```
#### Description 
Equivalent to GATK's UnifiedGenotyper, this tool is also used to perform SNP and indel calling, taking in read data from BAM files as an input and producing raw, unfiltered VCF files as output.
#### Options
| Command | Description |
| --- | --- |
| -h [--help] | print help messages |
| -f [--force] | overwrite output files if they exist |
| -O [--extra-options] | the "[key] [value]" arguments from the user is used as arguments for GATK |
| -r [--ref] arg | reference genome path |
| -i [--input] arg | input BAM file or dir |
| -o [--output] arg | output vcf file (if --skip-concat is set, the output will be a directory of vcf files) |
| -s [--skip-concat] | produce a set of vcf files instead of one |

---
### GATK
```
fcs-genome gatk <options>
```
#### Description
The Genome Analysis Toolkit developed by the Broad Institute - which handles and processes genomic data from any organism, with any level of ploidy, is the standard for SNP and indel indentification for DNA and RNAseq data. Apart from variant discovery, genotyping and ensuring data quality assurance is also done. GATK provides a Best Practices workflow for variant discovery that ensures the most accurate results.



