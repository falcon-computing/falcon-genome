## This script mark duplicates in merged BAM files
## for a list of sampleIDs specified in a input file
## The required inputs: $1=file with N lines like "SampleID\n";$2=<SeqType>;
## The output file is a merged BAM file with marked dups and the corresponding bai file per sample
## It also generates a Markdups.log file per sample
## (c) L. Perez-Cano 01/29/2015
#############################################################################################

## Import all global paths and variables
source globals.sh

#### Check if required inputs are correctly specified when running the script #######
Usage="Usage: ./P4_Markdups.sh <list_SampleIDs_file> <SeqType>\nSeqType options: ATACSeq, Custom_Capture, Exome, Genome or RNAseq\n";
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

## Format the date
date=`date | tr ' ' '_'`;

##### Check BAM files for all specified SampleIDs ###################
for SampleID in `cat $1`;do

	## Check if sample exists in NGS_raw_data repository
	if [ -d ${input_dir}/${SampleID} ];then

		## Check if the merged BAM file exists
		if [ -f $input_dir/$SampleID/${SampleID}.merged.bam ];then

			## Submit markdups 
			echo "Initiated process to mark duplicates in the BAM file for sample $SampleID $SeqType on $date" >$input_dir/$SampleID/Logs/Markdups.log;
			bam="$input_dir/$SampleID/${SampleID}.merged.bam";
			echo "java -XX:+UseSerialGC -Xmx512m -jar $picard_markdups_call TMP_DIR=tmp COMPRESSION_LEVEL=5 INPUT=$bam OUTPUT=$input_dir/$SampleID/${SampleID}.merged.markdups.bam METRICS_FILE=$input_dir/$SampleID/$SampleID.bam.dups_stats REMOVE_DUPLICATES=true ASSUME_SORTED=true VALIDATION_STRINGENCY=SILENT;$samtools_call index $input_dir/$SampleID/${SampleID}.merged.markdups.bam" >Markdups_TMP.sh;
			echo "COMMAND: java -XX:+UseSerialGC -Xmx512m -jar $picard_markdups_call TMP_DIR=tmp COMPRESSION_LEVEL=5 INPUT=$bam OUTPUT=$input_dir/$SampleID/${SampleID}.merged.markdups.bam METRICS_FILE=$input_dir/$SampleID/$SampleID.bam.dups_stats REMOVE_DUPLICATES=true ASSUME_SORTED=true VALIDATION_STRINGENCY=SILENT;$samtools_call index $input_dir/$SampleID/${SampleID}.merged.markdups.bam" >>$input_dir/$SampleID/Logs/Markdups.log;
			chmod 775 Markdups_TMP.sh;
			
			construct_qsub markdups_$SampleID $input_dir/$SampleID/Logs/Markdups.log Markdups_TMP.sh
			#qsub -cwd -N markdups_$SampleID -l h_vmem=8.625G,h_stack=8.625 -e $input_dir/$SampleID/Logs/Markdups.log -o $input_dir/$SampleID/Logs/Markdups.log -q long.q Markdups_TMP.sh;
			rm Markdups_TMP.sh;
		else
			echo "ERROR: Unable to find $input_dir/$SampleID/${SampleID}.merged.bam";
		fi;
	else
		echo "ERROR: Unable to find ${input_dir}/${SampleID}";
	fi;	

done;
