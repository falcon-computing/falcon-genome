## This script generates BAM files for sampleIDs specified in a input file
## The required inputs: $1=file with N lines like "SampleID\n";$2=<SeqType>
## BAM files are created in NGS_raw_data/SeqType/SampleID
## A Make_bam_$SampleID_$lane.log is created per sample+lane in NGS_raw_data/SeqType/SampleID/Logs/
## (c) L. Perez-Cano 11/06/2014
## (m) L. Perez-Cano 01/28/2015
#############################################################################################

## Import all global paths and  variables
source globals.sh

## The reference genome:
#ref_genome="/ifs/collabs/geschwind/NGS/reference_genome/hs37d5.fa";

#### Check if required inputs are correctly specified when running the script #######
Usage="Usage: ./P1_Generate_BAM_files.sh <list_SampleIDs_file> <SeqType>\nSeqType options: ATACSeq, Custom_Capture, Exome, Genome or RNAseq\n";
if [ $# -ne 2 ]; then
	echo "#########################################################";
	echo "ERROR: The number of parameters specified if not correct!"; 
	echo "#########################################################";
	echo -e $Usage; 
	exit;
fi;
if [ "$2" != "Exome" ] && [ "$2" != "Genome" ] && [ "$2" != "Custom_Capture" ] && [ "$2" != "ATACSeq" ] && [ "$2" != "RNAseq" ]; then
	echo "#################################################";
	echo "ERROR: The Seqtype $2 is not correct!";
	echo "#################################################";
	echo "SeqType options: ATACSeq, Custom_Capture, Exome, Genome or RNAseq";
	exit;
fi;

## The location of the input files:
input_dir="$ngs_raw_data_dir/$2";

## Other input variables:
#bwa_call="/ifs/collabs/geschwind/programs/bwa/bwa-0.7.10/bwa";
#samtools_call="/usr/local/loniWorkflows/Bioinformatics/samtools-0.1.19/samtools";
date=`date | sed "s/  / /g" | tr ' ' '_'`;

##### Submit processes to generate BAM files for all specified SampleIDs ###################
for SampleID in `cat $1`;do

	## Check if sample exists in NGS_raw_data repository
	if [ -d ${input_dir}/${SampleID} ];then

		## Check if Sample_RunDetails_all_fastqs log file exists
		if [ -f $input_dir/$SampleID/Logs/Sample_RunDetails_Unique ];then

			for RGid in `cat $input_dir/$SampleID/Logs/Sample_RunDetails_Unique | cut -d '|' -f6 | sed "s/RGid//g"`;do
			echo "Initiated process to generate the BAM file on $date:" >$input_dir/$SampleID/Logs/Make_bam_${SampleID}_$RGid.log;
			#Check if Check_BAMs.log already exists and if the BAM file was already correctly generated
			existingBAM=0;
			if [ -f $input_dir/$SampleID/Logs/Check_BAMs.log ];then
				if [ `grep ${SampleID}_$RGid $input_dir/$SampleID/Logs/Check_BAMs.log | awk {'print $3'}` -eq "1" ];then
					existingBAM=1;
				fi;
			fi;
			if [ $existingBAM -eq 0 ]; then
			#Check if RGinfo and fastq files exists for that sample+lane
			RGinfo1="$input_dir/$SampleID/Logs/${SampleID}_R1_${RGid}_RGinfo.txt";
			RGinfo2="$input_dir/$SampleID/Logs/${SampleID}_R2_${RGid}_RGinfo.txt";
			fastq1="$input_dir/$SampleID/${SampleID}_All_R1_${RGid}.fastq.gz";
			fastq2="$input_dir/$SampleID/${SampleID}_All_R2_${RGid}.fastq.gz";
			if [ -f $fastq1 ] && [ -f $fastq2 ] && [ -f $RGinfo1 ] && [ -f $RGinfo2 ];then

				## Assign values to variables required for adding the read group
				RG_ID=`head -1 $RGinfo1 | awk -F'\t' {'print $1'}`;
				platform=`head -1 $RGinfo1 | awk -F'\t' {'print $2'}`;
				library=`head -1 $RGinfo1 | awk -F'\t' {'print $3'}`;
				#unit=`awk -F'\t' {'print $4'} $RGinfo1`;

				## Submit mapping processes
				echo "$bwa_call mem -M -t 8 -R '@RG\tID:$RG_ID\tSM:$SampleID\tPL:$platform\tLB:$library' $ref_genome $fastq1 $fastq2 | $samtools_call view -S -b -u - | $samtools_call sort - $input_dir/$SampleID/${SampleID}_$RGid" >generate_BAM_file_TMP.sh;
				echo "COMMAND: $bwa_call mem -M -t 8 -R '@RG\tID:$RG_ID\tSM:$SampleID\tPL:$platform\tLB:$library' $ref_genome $fastq1 $fastq2 | $samtools_call view -S -b -u - | $samtools_call sort - $input_dir/$SampleID/${SampleID}_$RGid" >>$input_dir/$SampleID/Logs/Make_bam_${SampleID}_$RGid.log;
				chmod 775 generate_BAM_file_TMP.sh;
				
				construct_qsub bwa-mem_${SampleID}_$RGid $input_dir/$SampleID/Logs/Make_bam_${SampleID}_$RGid generate_BAM_file_TMP.sh;
				#qsub -cwd -N bwa-mem_${SampleID}_$RGid -l h_vmem=23G,h_stack=23 -e $input_dir/$SampleID/Logs/Make_bam_${SampleID}_$RGid.log -o $input_dir/$SampleID/Logs/Make_bam_${SampleID}_$RGid.log -q long.q generate_BAM_file_TMP.sh;
				rm generate_BAM_file_TMP.sh;
			else
				echo "##### ERROR: Unable to find $fastq1 or $fast2 or $RGinfo1 or $RGinfo2 ########";
			fi;
			fi;
			done;
		else
			echo "#### ERROR: Unable to find $input_dir/$SampleID/Logs/Sample_RunDetails_all_fastqs log file ####";
		fi;
	else
		echo "#### ERROR: Sample $SampleID does not exist in $input_dir ####";
	fi;	
done;
