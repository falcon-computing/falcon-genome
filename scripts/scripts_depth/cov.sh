#!/bin/bash

for file in $(ls /pool/storage/diwu/annovar/LB-2907-BloodDNA.recal.bam/)
do
	if [[ $file =~ bam$ ]];then
	 	samtools view -b -F 0x400 /pool/storage/diwu/annovar/LB-2907-BloodDNA.recal.bam/"$file" | /curr/niveda/bedtools2/bin/bedtools genomecov -ibam stdin -bga >> Blood_coverage.bed
	fi
done	
/curr/niveda/bedtools2/bin/bedtools intersect -loj -a /curr/niveda/coverage/FCS/hg19_coding_merge.bed -b Blood_coverage.bed > Blood_codingcov.bed
perl cov_calculate.pl
python cov_graph.py
 

