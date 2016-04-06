## This script generated gVCFs from merged.markdups.recal BAM files
## for a list of sampleIDs specified in a input file
## The required inputs: $1=file with N lines like "SampleID\n";$2=<SeqType>;
## The output files are .gvcf files per chr and their corresponding index file .idx
## It also generates a gVCF.log file per sample
##
## FILES USED / PACKAGES CALLED:
## ref_genome, GATK
## (c) L. Perez-Cano 02/20/2015
## (m) N. Kwok 09/09/2015
#############################################################################################

## Import all global paths and variables
source globals.sh

#### Check if required inputs are correctly specified when running the script #######
Usage="Usage: ./P6_Generate_gVCF_perchr.sh <list_SampleIDs_file> <SeqType>\nSeqType options: ATACSeq, Custom_Capture, Exome, Genome or RNAseq\n";
if [ $# -ne 2 ]; then
	echo "#########################################################";
	echo "ERROR: The number of parameters specified if not correct!"; 
	echo "#########################################################";
	echo -e $Usage;
	exit;
fi;
if [ "$2" != "Exome" ] && [ "$2" != "Genome" ] && [ "$2" != "Custom_Capture" ] && [ "$2" != "ATACSeq" ] && [ "$2" != "RNAseq" ]; then
        echo "#################################################";
        echo "ERROR: The SeqType $2 is not correct!";
        echo "#################################################";
        echo "SeqType options: ATACSeq, Custom_Capture, Exome, Genome or RNAseq";
        exit;
fi;
## The location of the input files:
input_dir="$ngs_raw_data_dir/$2";

date=`date | tr ' ' '_'`;

##### Check BAM files for all specified SampleIDs ###################
for SampleID in `cat $1`;do

	## Check if sample exists in NGS_raw_data repository
	if [ -d ${input_dir}/${SampleID} ];then

		## Check if the merged markdups recal BAM file exists
		if [ -f $input_dir/$SampleID/${SampleID}.merged.markdups.recal.bam ];then


			## Submit gVCF generation process per chr
			chr_list="1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,X,Y,MT";
			for chr in `echo $chr_list | tr ',' '\n'`;do
			echo "Initiated process of generating a gVCF file from $input_dir/$SampleID/${SampleID}.merged.markdups.recal.bam file for sample $SampleID $SeqType and chr $chr on $date" >$input_dir/$SampleID/Logs/gVCF_$chr.log;
			bam="$input_dir/$SampleID/${SampleID}.merged.markdups.recal.bam";
			echo "$java_call -Xmx2g -jar $gatk_call -T HaplotypeCaller -R $ref_genome -I $bam --emitRefConfidence GVCF --variant_index_type LINEAR --variant_index_parameter 128000 -L $chr -o $input_dir/$SampleID/${SampleID}_$chr.gvcf" >gvcf_TMP.sh;
			echo "COMMAND: $java_call -Xmx2g -jar $gatk_call -T HaplotypeCaller -R $ref_genome -I $bam --emitRefConfidence GVCF --variant_index_type LINEAR --variant_index_parameter 128000 -L $chr -o $input_dir/$SampleID/${SampleID}_$chr.gvcf" >>$input_dir/$SampleID/Logs/gVCF_$chr.log;
			chmod 775 gvcf_TMP.sh;
			construct_qsub gvcf_${SampleID}_$chr $input_dir/$SampleID/Logs/gVCF_$chr.log gvcf_TMP.sh
			#qsub -cwd -N gvcf_$SampleID_$chr -l h_vmem=8.5G,h_stack=8.5 -e $input_dir/$SampleID/Logs/gVCF_$chr.log -o $input_dir/$SampleID/Logs/gVCF_$chr.log -q long.q gvcf_TMP.sh;
			rm gvcf_TMP.sh;
			done;
		else
			echo "ERROR: Unable to find $input_dir/$SampleID/${SampleID}.merged.markdups.recal.bam";
		fi;
	else
		echo "ERROR: Unable to find ${input_dir}/${SampleID}";
	fi;	

done;
