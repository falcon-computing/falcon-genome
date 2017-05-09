#!/usr/local/bin/perl
use strict;
use warnings;

#Input file is the exome coverage file. Columns: CHR  EXOME_START  EXOME_STOP  CHR  BAM_START  BAM_STOP  COVERAGE
#Average coverage output files are generated for:
#	1) Location-wise for the exon Columns: CHR  START  STOP  TOTAL_COV  COV>=4  >=10  >=20  >=50  >=100  
#	2) For the whole sample  Columns: SAMPLE  AVG_COV  COV>=4  >=10  >=20  >=50  >=100    	 

open(my $INPUTFILE, "<", "Blood_codingcov.bed");
open(my $MYOUTFILE, ">", "Blood_coveragevalues.txt");
print $MYOUTFILE "CHR\tSTART\tSTOP\tTOTAL_COV\t>=4\t>=10\t>=20\t>=50\t>=100\tNR\n";
open(my $MYOUTFILE2, ">", "Blood_BloodCoverage.csv");
print $MYOUTFILE2 "SAMPLE,AVG_COV,>=4,>=10,>=20,>=50,>=100\n";

my $total=0;
my $total_cov=0;
my $total_cov4=0;
my $total_cov10=0;
my $total_cov20=0;
my $total_cov50=0;
my $total_cov100=0;
my $count=0;
my $prev_chr;
my $prev_start;
my $prev_stop;
my $bool=0;
my $multiple;
my $condition="false";

my $csv_total;
my $csv_total_cov;
my $csv_total_cov4;
my $csv_total_cov10;
my $csv_total_cov20;
my $csv_total_cov50;
my $csv_total_cov100;
        
while(my $line= <$INPUTFILE>)
{
	chomp($line);
	my @file= split "\t", $line;
	$count++;
	#Checks are made to check for identical chr,exome_start and exome_stop in order to compute average values location wise
	if($count!=1 && $file[0] eq $prev_chr && $file[1]==$prev_start && $file[2]==$prev_stop)
	{
		$bool=1;
		if($condition eq "true")
		{
			next;
		}
		if($file[5]>=$file[2])
		{
			$condition="true";
		}
	}	
	elsif($count==1)  
	{
		$bool=1;
		if($file[5]>=$file[2])
		{
			$condition="true";
		}
	}
	else
	{
		#Location-wise data is printed one at a time in the output file and total values are set back to zero for the new location.
		#Overall sample values are continously added
			print $MYOUTFILE $prev_chr,"\t",$prev_start,"\t",$prev_stop,"\t",$total_cov,"\t",$total_cov4,"\t",$total_cov10,"\t",$total_cov20,"\t",$total_cov50,"\t",$total_cov100,"\t",$total,"\n";
			$csv_total+=$total;		
			$total=0;
			$csv_total_cov+=$total_cov;
			$total_cov=0;
			$csv_total_cov4+=$total_cov4;
			$total_cov4=0;
			$csv_total_cov10+=$total_cov10;
			$total_cov10=0;
			$csv_total_cov20+=$total_cov20;
			$total_cov20=0;
			$csv_total_cov50+=$total_cov50;
			$total_cov50=0;
			$csv_total_cov100+=$total_cov100;
			$total_cov100=0;
			$condition="false";
	}
	#Exact overlaps must be considered and regions preceding and following target exonic regions are not considered.
	#Further, $multiple is a variable that makes sure that every position is considered. In some cases bam_stop-bam_stop > 1
	my $subt_stop=$file[5]<$file[2]?$file[5]:$file[2];
        my $subt_start=$file[4]<$file[1]?$file[1]:$file[4];
        $multiple=$subt_stop-$subt_start;
        $total+=$multiple;
        $total_cov+=$multiple*$file[6];
        if($file[6]>=4)
        {
        	$total_cov4+=$multiple;
        }
        if($file[6]>=10)
        {
                $total_cov10+=$multiple;
        }
        if($file[6]>=20)
        {
                $total_cov20+=$multiple;
        }
        if($file[6]>=50)
        {
        	$total_cov50+=$multiple;
       	}
        if($file[6]>=100)
        {
        	$total_cov100+=$multiple;
        }
        $bool=0;
	$prev_chr=$file[0];
	$prev_start=$file[1];
	$prev_stop=$file[2];
}
#if($condition eq "false")
#{
	print $MYOUTFILE $prev_chr,"\t",$prev_start,"\t",$prev_stop,"\t",$total_cov,"\t",$total_cov4,"\t",$total_cov10,"\t",$total_cov20,"\t",$total_cov50,"\t",$total_cov100,"\t",$total,"\n";
#}
#Average values for the entire sample are written into an output file
	print $MYOUTFILE2 "Blood",",",($csv_total_cov/$csv_total),",",($csv_total_cov4/$csv_total),",",($csv_total_cov10/$csv_total),",",($csv_total_cov20/$csv_total),",",($csv_total_cov50/$csv_total),",",($csv_total_cov100/$csv_total),"\n";


	
		
