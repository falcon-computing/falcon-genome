# fcs-genome pipeline

## Description
   Variant calling pipeline for germline and somatic mutations adopting GATK's Best Practices along with Falcon's FPGA acceleration techniques to significantly improve performance.

## Help
To get the list of commands available in the pipeline, type in:
```
fcs-genome 
```

## Commands and Options
### Name
   align
### Description
   Maps pair-ended FASTQ sequences against a large reference genome sequence. The resulting BAM file is sorted, with duplicates marked. 
### Options
| Command | Description |
| --- | --- |
| -h [--help] | print help messages |
| -f [--force] | overwrite output files if they exist |
| -r [--ref] arg | reference genome path |
| -1 [--fastq1] arg | input pair-end fastq file |
| -2 [--fastq2] arg | input pair-end fastq file |
| -o [--output] arg | output BAM file |
| -R [--rg] arg | read group ID ('ID' in BAM header) |
| -S [--sp] arg | sample ID ('SM' in BAM header) |
| -P [--pl] arg | platform ID ('PL' in BAM header) |
| -L [--lb] arg | library ID ('LB' in BAM header) |
| -l [--align-only] | skip mark duplicates |
### Example
```
fcs-genome align \
  --ref [Path to reference genome] \
  --fastq1 [Path to fastq1] \
  --fastq2 [Path to fastq2] \
  --output [Output BAM file] \
  --rg [Read Group ID] \
  --sp [Sample ID] \
  --pl [Platform ID] \
  --lb [Library ID]
```

2.
### Name
   markdup
### Description
   MarkDuplicates tags duplicate reads in a BAM file. 
### Options
| Command | Description |
| --- | --- |
| -h [--help] | print help messages |
| -f [--force] | overwrite output files if they exist |
| -i [--input] arg | input file |
| -o [--output] arg | output file |
### Example
```
fcs-genome markdup \
  --input [Output BAM file of align] \
  --output [Output duplicates marked BAM file]
```
NOTE: In case the option of --align-only is used and the merging of sorted BAM files and Mark Duplicates is to be done seperately, each partitioned fastq file is taken as an input for alignment. The result is a parent folder containing sub-folders with the same name as the read group, which is unique for each partioned aligned, BAM file. This parent folder is then taken as an input for mark duplicates.
### Example
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
3. 
### Name
   baserecal
### Description
   Base Quality Score Recalibration (BQSR) gives per-base score estimates of errors caused by sequencing machines.
### Options
| Command | Description |
| --- | --- |
| -h [--help] | print help messages |
| -f [--force] | overwrite ouput files, if they exist |
| -r [--ref] arg | reference genome path |
| -i [--input] arg | input BAM file or dir |
| -o [--output] arg | output BQSR file |
| -K [--knownSites] arg | known sites for base recalibration |
### Example   
```
fcs-genome baserecal \
  --ref [Path to reference genome] \
  --input [Output BAM file of markdup] \
  --output [Output BQSR file] \
  --knownSites [known sites argument]
```

4.
### Name
   printreads
### Description
   PrintReads is a tool that manipulates BAM files. It takes the output of BQSR to result in recalibrated BAM files.
### Option
| Command | Description |
| --- | --- |
| -h [--help] | print help messages |
| -f [--force] | overwrite output files if they exist |
| -r [--ref] arg | reference genome path |
| -b [--bqsr] arg | input BQSR file |
| -i [--input] arg | input BAM file or dir |
| -o [--output] arg | output BAM files |
### Example
```
fcs-genome printreads \
  --ref [Path to reference genome] \
  --bqsr [Output BQSR file of baserecal] \
  --input [Output BAM file of markdup] \
  --output [Output recalibrated BAM file]
```

5.
### Name
   bqsr
### Description
   Command that implements BQSR followed by PrintReads.
### Options
| Command | Description |
| --- | --- |
| -h [--help] | print help messages |
| -f [--force] | overwrite output files if they exist |
| -f [--ref] arg | reference genome path |
| -b [-bqsr] arg | output BQSR file (if left blank, no file will be produced) |
| -i [--input] arg | input BAM file or dir |
| -o [--output] arg | output directory of BAM files |
| -K [--knownSites] arg | known sites for base recalibration |
### Example   
```
fcs-genome bqsr \
  --ref [Path to reference genome] \
  --input [Output BAM file of markdup] \
  --output [Output recalibrated BAM file] \
  --knownSites [known sites argument]
```

6. 
### Name
   htc
### Description
   Haplotype Caller calls variants, producing a genomic VCF (gVCF) file. To get a VCF file as output, include the option --
produce-vcf.
### Options
| Command | Description |
| --- | --- |
| -h [--help] | print help messages |
| -f [--force] | overwrite output files if they exist |
| -r [--ref] arg | reference genome path |
| -i [--input] arg | input BAM file or dir |
| -o [--output] arg | output gvcf file |
| -s [--skip-concat] | produce a set of gvcf files instead of one |
### Example
```
fcs-genome htc \
  --ref [Path to reference genome] \
  --input [Output recalibrated BAM file of bqsr] \
  --output [Output gvcf file]
```

