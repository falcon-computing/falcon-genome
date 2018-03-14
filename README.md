# fcs-genome pipeline  
##Contents

## Overview
The fcs-genome pipeline allows for variant calling for both germline and somatic mutations adopting GATK's Best Practices along with Falcon's FPGA acceleration techniques to significantly improve performance. The general pattern of the pipeline workflow starts with raw sequence reads, and proceeds to obtain a filtered set of variants that can be annotated for further analysis.

Like the GATK Best Practices, the workflow follows two phases of analysis:
1. Data pre-processing - The raw sequence reads in the FASTQ format are converted to BAM files through alignment to the reference using a BWA-like tool. The BAM files are made analysis-ready through the application of the fcs-genome version of correctional tools such as IndelRealigner and Base Recalibrator to account for biases.
2. Variant Calling - Variant calls, either germline or somatic, are made using the tools equivalent to GATK's Haplotype Caller and Mutect2 respectively. This produces files in either GVCF/VCF format.

## Quick Start 
### Installation
#### Software Prerequisites

#### System Setup
+ Software for Falcon Genomics is installed in ```/usr/local/falcon/```
+ System information must be stored in ```/usr/local/falcon/fcs-genome.conf```
+ Paths to reference, known sites and input FASTQ files are required as parameters for the pipeline

### Example
#### HTC pipeline
This pipeline starts with raw sequence data in the FASTQ format as input, aligns the sequences to the reference sequence, marks duplicate reads in the aligned BAM file, recalibrates reads based on per-base quality score based on sequencing machine biases, and performs germline variant calling.
```
#Export fcs-genome and other required tools to the PATH
source /usr/bin/falcon/setup.sh 

#Define input paths
ref_dir=

ref_genome=$ref_dir/human_g1k_v37.fasta
db138_SNPs=$ref_dir/dbsnp_138.b37.vcf  #Optional
g1000_indels=$ref_dir/1000G_phase1.indels.b37.vcf  #Optional
g1000_gold_standard_indels=$ref_dir/Mills_and_1000G_gold_standard.indels.b37.vcf #Optional

fastq_dir=
sample_id=
platform= Illumina
output_dir=

# Alignment to reference
# Input=FASTQ, OUTPUT= BAM. Command fcs-genome align also sorts the aligned BAM file and marks duplicates unless --align-only is specified as a parameter
fcs-genome align \
        --ref $ref_genome \
        --fastq1 ${fastq_dir}/${sample_id}_1.fastq.gz \
        --fastq2 ${fastq_dir}/${sample_id}_2.fastq.gz \
        --output $output_dir/${sample_id}_marked.bam \
        --rg ${sample_id} --sp ${sample_id} --pl $platform --lb ${sample_id}

# Base Recalibration
# Input=BAM file with duplicates marked, OUTPUT=Recalibrated BAM file. Command fcs-genome bqsr performs GATK's Base Recalibration and Print Reads in a single command.
fcs-genome bqsr \
        --ref $ref_genome \
        --input $output_dir/${sample_id}_marked.bam \
        --output $output_dir/${sample_id}_recalibrated.bam \
        --knownSites $db138_SNPs \
        --knownSites $g1000_indels \
        --knownSites $g1000_gold_standard_indels   

# Haplotype Caller
# Input=Recalibrated BAM, OUTPUT=VCF file. Command fcs-genome htc performs germline variant calling with default output format as GVCF. A VCF file is produced when parameter --produce-vcf is specified.
fcs-genome htc \
        --ref $ref_genome \
        --input $output_dir/${sample_id}_recalibrated.bam \
        --output ${sample_id}.vcf --produce-vcf
```

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
| -r [--ref] arg | reference genome path |
| -i [--input] arg | input BAM file or dir |
| -o [--output] arg | output gvcf file |
| -s [--skip-concat] | produce a set of gvcf files instead of one |

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
| - f [--force] | overwrite output files if they exist |
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
The Genome Analysis Toolkit- which handles and processes genomic data from any organism, with any level of ploidy is the standard for SNP and indel indentification for DNA and RNAseq data. 

## Multiple FASTQ files as Input for Alignment only
#### Description
In case the option of --align-only is used and the merging of sorted BAM files and Mark Duplicates is to be done seperately, each partitioned fastq file is taken as an input for alignment. The result is a parent folder containing sub-folders with the same name as the read group, which is unique for each partioned aligned, BAM file. This parent folder is then taken as an input for mark duplicates.
#### Example
```
for i in $(seq 0 $((num_groups - 1))); do 
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

fcs-genome markDup \
    -i ${tmp_dir}/$sample_id \
    -o ${tmp_dir}/${sample_id}.bam \
    -f 2>> $log_file 
```

