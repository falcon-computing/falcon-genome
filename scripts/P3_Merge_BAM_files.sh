## This script merges all the BAM files generated per sample
## for a list of sampleIDs specified in a input file
## The required inputs: $1=file with N lines like "SampleID\n";$2=<SeqType>;
## The output file is a merged BAM file per sample
## It also generates a Merge_BAM_files.log file per sample
## (c) L. Perez-Cano 01/29/2015
#############################################################################################

## Import all global paths and variables
source globals.sh

#### Check if required inputs are correctly specified when running the script #######
Usage="Usage: ./P3_Merge_BAM_files.sh <list_SampleIDs_file> <SeqType>\nSeqType options: ATACSeq, Custom_Capture, Exome, Genome or RNAseq\n";
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
#input_dir="/ifs/collabs/geschwind/NGS/NGS_raw_data/$2";
input_dir="$ngs_raw_data_dir/$2"

## Date formatting
date=`date | tr ' ' '_'`;

##### Check BAM files for all specified SampleIDs ###################
for SampleID in `cat $1`;do

	## Check if sample exists in NGS_raw_data repository
	if [ -d ${input_dir}/${SampleID} ];then

		## Check if Check_BAMs.log file exists
		if [ -f $input_dir/$SampleID/Logs/Check_BAMs.log ];then

			## Check if all BAM files for the sample are ok and merge
			if [ `grep All_BAM_files_generated? $input_dir/$SampleID/Logs/Check_BAMs.log | awk {'print $2'}` -eq 1 ];then
				echo "Initiated process to merge all the BAM files for sample $SampleID on $date:" >$input_dir/$SampleID/Logs/mergeBAMs.log;
				echo -n "" >$input_dir/$SampleID/Logs/list_BAMs_tomerge.txt;
				for bam in `grep "BAMfile_satus_for_sample:" $input_dir/$SampleID/Logs/Check_BAMs.log | awk {'print $2'}`;do echo $input_dir/$SampleID/$bam.bam >>$input_dir/$SampleID/Logs/list_BAMs_tomerge.txt;done;
				echo "$bamtools_call merge -list $input_dir/$SampleID/Logs/list_BAMs_tomerge.txt -out $input_dir/$SampleID/$SampleID.merged.bam" >mergeBAM_temp.sh;
				echo "COMMAND: $bamtools_call merge -list $input_dir/$SampleID/Logs/list_BAMs_tomerge.txt -out $input_dir/$SampleID/$SampleID.merged.bam" >>$input_dir/$SampleID/Logs/mergeBAMs.log;
				chmod 775 mergeBAM_temp.sh;
				
				construct_qsub mergeBAMs_$SampleID $input_dir/$SampleID/Logs/mergeBAMs.log mergeBAM_temp.sh
				#qsub -cwd -N mergeBAMs_$SampleID -l h_vmem=5.75G,h_stack=5.75 -e $input_dir/$SampleID/Logs/mergeBAMs.log -o $input_dir/$SampleID/Logs/mergeBAMs.log -q long.q mergeBAM_temp.sh;
				rm mergeBAM_temp.sh;
			else
				echo "ERROR: Not all the BAM files for sample $SampleID are ready to be merged" >>$input_dir/$SampleID/Logs/mergeBAMs.log;
			fi;
		else
			echo "ERROR: Unable to find $input_dir/$SampleID/Logs/Check_BAMs.log file; you must run P2_Check_BAM_files.sh before merging";
		fi;
	else
		echo "ERROR: Unable to find sample ${input_dir}/${SampleID}";
	fi;	
done;
