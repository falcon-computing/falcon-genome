# fcs-genome pipeline
Variant calling pipeline for germline and somatic mutations

## Help
To get the list of commands available in the pipeline, type in:
```
fcs-genome 
```
To get help for any of the commands, type in:
```
fcs-genome [command] --help
```

## Pipeline Workflow
1. align: 
> Maps pair-ended FASTQ sequences against a large reference genome sequence. 
> The resulting BAM file is sorted, with duplicates marked. 
> To skip the sort and mark duplicates, use the option --align-only. 
Command:
```
fcs-genome align --ref [Path to reference genome] --fastq1 [Path to fastq1] --fastq2 [Path to fastq2] --output [Output BAM file] --rg [Read Group ID] --sp [Sample ID] --pl [Platform ID] --lb [Library ID]
```

2. markdup:
> MarkDuplicates tags duplicate reads in a BAM file. 
Command:
```
fcs-genome markdup --input [Output BAM file of align] --output [Output duplicates marked BAM file]
```

3. baserecal:
> Base Quality Score Recalibration (BQSR) gives per-base score estimates of errors caused by sequencing machines.
Command:
```
fcs-genome baserecal --ref [Path to reference genome] --input [Output BAM file of markdup] --output [Output BQSR file] --knownSites [known sites argument]
```

4. printreads:
> PrintReads is a tool that manipulates BAM files. It takes the output of BQSR to result in recalibrated BAM files.
Command:
```
fcs-genome printreads --ref [Path to reference genome] --bqsr [Output BQSR file of baserecal] --input [Output BAM file of markdup] --output [Output recalibrated BAM file]
```

5. bqsr:
> Command that implements BQSR followed by PrintReads.
Command:
```
fcs-genome bqsr --ref [Path to reference genome] --input [Output BAM file of markdup] --output [Output recalibrated BAM file] --knownSites [known sites argument]
```

6. htc:
> Haplotype Caller calls variants, producing a genomic VCF (gVCF) file. To get a VCF file as output, include the option --
produce-vcf.
Command:
```
fcs-genome htc --ref [Path to reference genome] --input [Output recalibrated BAM file of bqsr] --output [Output gvcf file]
```

