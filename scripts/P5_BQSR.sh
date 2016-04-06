## This script recalibrates base quality scores in merged.markdups BAM files
## for a list of sampleIDs specified in a input file
## The required inputs: $1=file with N lines like "SampleID\n";$2=<SeqType>;
## The output file is a merged BAM file with marked dups and BQSR and the corresponding bai file per sample
## It also generates a BQSR.log file per sample
##
## FILES USED/PACKAGES CALLED:
## ref_genome, g1000_indels, g1000_gold_standard_indels, db138_SNPs, GATK, samtools
## (c) L. Perez-Cano 02/03/2015
## (m) N. Kwok 09/09/2015
#############################################################################################

## Import all global paths and variables
source globals.sh

#### Check if required inputs are correctly specified when running the script #######
Usage="Usage: ./P5_BQSR.sh <list_SampleIDs_file> <SeqType>\nSeqType options: ATACSeq, Custom_Capture, Exome, Genome or RNAseq\n";
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

		## Check if the merged markdups BAM file exists
		if [ -f $input_dir/$SampleID/${SampleID}.merged.markdups.bam ];then


			## Check if exists the corresponding Plots dir / create it
			if [ ! -d $input_dir/$SampleID/Plots ];then
				mkdir $input_dir/$SampleID/Plots;
			fi;

			## Submit BQSR 
			echo "Initiated process of BQSR in the BAM file for sample $SampleID $SeqType on $date" >$input_dir/$SampleID/Logs/BQSR.log;
			bam="$input_dir/$SampleID/${SampleID}.merged.markdups.bam";
			#echo "$java_call -d64 -Xmx2g -jar $gatk_call -T BaseRecalibrator -R $ref_genome -I $input_dir/$SampleID/${SampleID}.merged.markdups.bam -knownSites $g1000_indels -knownSites $g1000_gold_standard_indels -knownSites $db138_SNPs -o $input_dir/$SampleID/${SampleID}.recalibration_report.grp;$java_call -Xmx2g -jar $gatk_call -T PrintReads -R $ref_genome -I $input_dir/$SampleID/${SampleID}.merged.markdups.bam -BQSR $input_dir/$SampleID/${SampleID}.recalibration_report.grp -o $input_dir/$SampleID/${SampleID}.merged.markdups.recal.bam;$java_call -Xmx2g -jar $gatk_call -T BaseRecalibrator -R $ref_genome -I $input_dir/$SampleID/${SampleID}.merged.markdups.recal.bam -knownSites $g1000_indels -knownSites $g1000_gold_standard_indels -knownSites $db138_SNPs -o $input_dir/$SampleID/${SampleID}.postrecalibration_report.grp;$java_call -Xmx2g -jar $gatk_call -T AnalyzeCovariates -R $ref_genome -before $input_dir/$SampleID/${SampleID}.recalibration_report.grp -after $input_dir/$SampleID/${SampleID}.postrecalibration_report.grp -plots $input_dir/$SampleID/Plots/${SampleID}.recalibration_plot.pdf;$samtools_call index $input_dir/$SampleID/${SampleID}.merged.markdups.recal.bam" >bqsr_TMP.sh;
			#echo "COMMAND: $java_call -d64 -Xmx1024m -jar $gatk_call -T BaseRecalibrator -R $ref_genome -I $input_dir/$SampleID/${SampleID}.merged.markdups.bam -knownSites $g1000_indels -knownSites $g1000_gold_standard_indels -knownSites $db138_SNPs -o $input_dir/$SampleID/${SampleID}.recalibration_report.grp;$java_call -Xmx2g -jar $gatk_call -T PrintReads -R $ref_genome -I $input_dir/$SampleID/${SampleID}.merged.markdups.bam -BQSR $input_dir/$SampleID/${SampleID}.recalibration_report.grp -o $input_dir/$SampleID/${SampleID}.merged.markdups.recal.bam;$java_call -d64 -Xmx1024m -jar $gatk_call -T BaseRecalibrator -R $ref_genome -I $input_dir/$SampleID/${SampleID}.merged.markdups.recal.bam -knownSites $g1000_indels -knownSites $g1000_gold_standard_indels -knownSites $db138_SNPs -o $input_dir/$SampleID/${SampleID}.postrecalibration_report.grp;$java_call -Xmx2g -jar $gatk_call -T AnalyzeCovariates -R $ref_genome -before $input_dir/$SampleID/${SampleID}.recalibration_report.grp -after $input_dir/$SampleID/${SampleID}.postrecalibration_report.grp -plots $input_dir/$SampleID/Plots/${SampleID}.recalibration_plot.pdf;$samtools_call index $input_dir/$SampleID/${SampleID}.merged.markdups.recal.bam" >>$input_dir/$SampleID/Logs/BQSR.log;
			
			echo "$java_call -d64 -Xmx2g -jar $gatk_call -T AnalyzeCovariates -R $ref_genome -before $input_dir/$SampleID/${SampleID}.recalibration_report.grp -after $input_dir/$SampleID/${SampleID}.postrecalibration_report.grp -plots $input_dir/$SampleID/Plots/${SampleID}.recalibration_plot.pdf; $samtools_call index $input_dir/$SampleID/${SampleID}.merged.markdups.recal.bam" >bqsr_TMP.sh;
			chmod 775 bqsr_TMP.sh;
			
			construct_qsub recal_$SampleID $input_dir/$SampleID/Logs/BQSR.log bqsr_TMP.sh;
			#qsub -cwd -N recal_$SampleID -l h_vmem=23G,h_stack=23 -e $input_dir/$SampleID/Logs/BQSR.log -o $input_dir/$SampleID/Logs/BQSR.log -q long.q bqsr_TMP.sh;
			rm bqsr_TMP.sh;
		else
			echo "ERROR: Unable to find $input_dir/$SampleID/${SampleID}.merged.markdups.bam";
		fi;
	else
		echo "ERROR: Unable to find ${input_dir}/${SampleID}";
	fi;	

done;
