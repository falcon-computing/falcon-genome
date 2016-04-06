## This script checks if BAM files are aparently succssfully generated
## for a list of sampleIDs specified in a input file
## The required inputs: $1=file with N lines like "SampleID\n";$2=<SeqType>;$3=<Output_file>
## Its outputs a Check_BAMs.log file that specifies un/successful(0/1) processes per sample
## (c) L. Perez-Cano 11/20/2014
## (m) L. Perez-Cano 01/29/2015
#############################################################################################

## Import all global paths and variables
source globals.sh

#### Check if required inputs are correctly specified when running the script #######
Usage="Usage: ./P2_Check_BAM_files.sh <list_SampleIDs_file> <SeqType>\nSeqType options: ATACSeq, Custom_Capture, Exome, Genome or RNAseq\n";
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
input_dir="$ngs_raw_data_dir/$2"

## Other input variables:
date=`date | tr ' ' '_'`;

##### Check BAM files for all specified SampleIDs ###################
for SampleID in `cat $1`;do

	## Check if sample exists in NGS_raw_data repository
	if [ -d ${input_dir}/${SampleID} ];then

		## Check if Sample_RunDetails_Unique log file exists
		if [ -f $input_dir/$SampleID/Logs/Sample_RunDetails_Unique ];then

			echo -n "" >$input_dir/$SampleID/Logs/Check_BAMs.log;
			num_RGids=`cat $input_dir/$SampleID/Logs/Sample_RunDetails_Unique | cut -d '|' -f6 | sed "s/RGid//g" | sort -k1 | uniq | wc -l`;
			num_okBAMs=0;
			for RGid in `cat $input_dir/$SampleID/Logs/Sample_RunDetails_Unique | cut -d '|' -f6 | sed "s/RGid//g" | sort -k1 | uniq`;do
			printf "BAMfile_satus_for_sample: ${SampleID}_${RGid} " | tee -a $input_dir/$SampleID/Logs/Check_BAMs.log;

			## Check if the BAM file exists:
			if [ -f $input_dir/$SampleID/${SampleID}_${RGid}.bam ];then

					## Check if the BAM file looks good:
					($samtools_call view -H $input_dir/$SampleID/${SampleID}_${RGid}.bam >tmp.out) 2>err;
					rm tmp.out;	
					if [ `grep "^\[bam_header_read] EOF marker is absent. The input is probably truncated." err | wc -l` -eq 0 ]; then
						printf "1\n" | tee -a $input_dir/$SampleID/Logs/Check_BAMs.log;
						num_okBAMs=`expr $num_okBAMs + 1`;
					else
						printf "0\n" | tee -a $input_dir/$SampleID/Logs/Check_BAMs.log;
					fi;
			else
				echo "ERROR: Unable to find $input_dir/$SampleID/${SampleID}_${RGid}.bam";
			fi;	
			done;
			if [ $num_okBAMs == $num_RGids ];then
				echo "All_BAM_files_generated? 1" | tee -a $input_dir/$SampleID/Logs/Check_BAMs.log;
			else
				echo "All_BAM_files_generated? 0" | tee -a $input_dir/$SampleID/Logs/Check_BAMs.log;
			fi;		
		else
			echo "ERROR: Unable to find $input_dir/$SampleID/Logs/Sample_RunDetails_Unique file";
		fi;
	else
		echo "ERROR: Unable to find ${input_dir}/${SampleID} sample";
	fi;	
done;

