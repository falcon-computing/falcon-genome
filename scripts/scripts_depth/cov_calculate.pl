#!/usr/local/bin/perl
use strict;
use warnings;

#Input file is the exome coverage file. Columns: CHR  EXOME_START  EXOME_STOP  CHR  BAM_START  BAM_STOP  COVERAGE
#Average coverage output files are generated for:
#  1) Location-wise for the exon Columns: CHR  START  STOP  TOTAL_COV  COV>=4  >=10  >=20  >=50  >=100  
#  2) For the whole sample  Columns: SAMPLE  AVG_COV  COV>=4  >=10  >=20  >=50  >=100       
my $sample=$ARGV[0];
open(my $INPUTFILE, "<", "$sample"."_codingcov.bed");
open(my $MYOUTFILE, ">", "$sample"."_coveragevalues.txt");
print $MYOUTFILE "CHR\tSTART\tSTOP\tTOTAL_COV\t>=0\t>=5\t>=10\t>=15\t>=20\t>=25\t>=30\t>=35\t>=40\t>=45\t>=50\t>=55\t>=60\t>=65\t>=70\t>=75\t>=80\t>=85\t>=90\t>=95\t>=100\tNR\n";
open(my $MYOUTFILE2, ">", "$sample"."_"."$sample"."Coverage.csv");
print $MYOUTFILE2 "SAMPLE,AVG_COV,>=0,>=5,>=10,>=15,>=20,>=25,>=30,>=35,>=40,>=45,>=50,>=55,>=60,>=65,>=70,>=75,>=80,>=85,>=90,>=95,>=100\n";

my $total=0;
my $total_cov=0;
my @total_cov;
my $count=0;
my $prev_chr;
my $prev_start;
my $prev_stop;
my $bool=0;
my $multiple;
my $condition="false";

my $csv_total;
my $csv_total_cov;
my @csv_total_cov;
        
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
      print $MYOUTFILE $prev_chr,"\t",$prev_start,"\t",$prev_stop,"\t",$total_cov,"\t";
      for(my $i=0;$i<=100;$i=$i+5)
      {
      	$total_cov[$i]= defined $total_cov[$i] ? $total_cov[$i] : 0;
      	print $MYOUTFILE $total_cov[$i],"\t";
      }
      print $MYOUTFILE "\n";	
      $csv_total+=$total;		
      $total=0;
      for(my $i=0; $i<scalar(@total_cov); $i++)
      {
      	$csv_total_cov[$i] = defined $csv_total_cov[$i] ? $csv_total_cov[$i] : 0;
      	$total_cov[$i]= defined $total_cov[$i] ? $total_cov[$i] : 0;
      	$csv_total_cov[$i]+=$total_cov[$i];
      }

      @total_cov=();
      $condition="false";
  }
  
  #Exact overlaps must be considered and regions preceding and following target exonic regions are not considered.
  #Further, $multiple is a variable that makes sure that every position is considered. In some cases bam_stop-bam_stop > 1
  my $subt_stop=$file[5]<$file[2]?$file[5]:$file[2];
        my $subt_start=$file[4]<$file[1]?$file[1]:$file[4];
        $multiple=$subt_stop-$subt_start;
        $total+=$multiple;
        $total_cov+=$multiple*$file[6];
  my $end=$file[6]>100?100:$file[6];
  for(my $i=0; $i<=$end; $i++)
  {
    $total_cov[$i]+=$multiple;
  }

  $bool=0;
  $prev_chr=$file[0];
  $prev_start=$file[1];
  $prev_stop=$file[2];
}
print $MYOUTFILE $prev_chr,"\t",$prev_start,"\t",$prev_stop,"\t",$total_cov,"\t";
for(my $i=0;$i<=100;$i=$i+5)
{
  $total_cov[$i]= defined $total_cov[$i] ? $total_cov[$i] : 0;
  print $MYOUTFILE $total_cov[$i],"\t";
}
print $MYOUTFILE "\n";

#Average values for the entire sample are written into an output file
  print $MYOUTFILE2 "$sample",",";
  for(my $i=0;$i<=100;$i=$i+5)
  {
    $csv_total_cov[$i] = defined $csv_total_cov[$i] ? $csv_total_cov[$i] : 0;
    print $MYOUTFILE2 ($csv_total_cov[$i]/$csv_total),",";
  }
   print $MYOUTFILE2 "\n";    
