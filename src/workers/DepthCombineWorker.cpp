#include <algorithm>
#include <bits/stdc++.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <math.h>
#include <sstream>
#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/DepthCombineWorker.h"

namespace fcsgenome {

DepthCombineWorker::DepthCombineWorker(
     std::vector<std::string> &input_files,
     std::string output_path,
     bool &flag_baseCoverage,
     bool &flag_intervalCoverage,
     bool &flag_sampleSummary,
     bool &flag_f): Worker(1, 1),
     input_files_(input_files),
     flag_baseCoverage_(flag_baseCoverage),
     flag_intervalCoverage_(flag_intervalCoverage),
     flag_sampleSummary_(flag_sampleSummary)
{
     // check output files
     output_file_ = check_output(output_path, flag_f);
}

void DepthCombineWorker::check() {
     for (int i = 0; i < input_files_.size(); i++) {
          check_input(input_files_[i]);
          check_input(input_files_[i] + ".sample_cumulative_coverage_counts");
          check_input(input_files_[i] + ".sample_cumulative_coverage_proportions");
          check_input(input_files_[i] + ".sample_interval_statistics");
          check_input(input_files_[i] + ".sample_gene_summary");
          check_input(input_files_[i] + ".sample_interval_summary");
          check_input(input_files_[i] + ".sample_summary");
          check_input(input_files_[i] + ".sample_statistics");
     }
}

void DepthCombineWorker::merge_outputs(std::string file_type) {
     std::map<std::string, std::vector<int> > InputData;
     std::string sampleName;
     std::vector<int> results;
     std::vector<int> temp_round;
     std::vector<std::string> temp;

     std::string header;
     for (int i = 0 ; i < input_files_.size() ; i++){
        DLOG(INFO) << "Merge File : " << input_files_[i] + file_type << std::endl;
        std::ifstream file((input_files_[i] + file_type).c_str());
        std::string value;
        getline(file,value);
        // Getting the header:
        if (i==0) header = value;

        // Getting data for each sample:
        std::string grab_line;
        int counter=0;
        if (i == 0){
            int number_of_fields = count( header.begin(),header.end(),'\t' );
            //std::cout << number_of_fields << std::endl;
            while (getline(file,grab_line)) {
                   boost::split(temp, grab_line, [](char c){return c == '\t';});
                   sampleName = temp[0];
                   temp.erase(temp.begin());
                   for (int k = 0; k < temp.size(); k++){
                        results.push_back(atoi(temp.at(k).c_str()));
                   }
                   temp.clear();
                   InputData.insert( std::pair<std::string,std::vector<int>>(sampleName,results) );
                   counter++;
            }
        } else {
            while (getline(file,grab_line)) {
                   boost::split(temp, grab_line, [](char c){return c == '\t';});
                   sampleName=temp[0];
                   temp.erase(temp.begin());
                   for (int m = 0; m < temp.size(); m++){
                        temp_round.push_back(atoi(temp.at(m).c_str()));
                   }
                   temp.clear();

   	               std::map<std::string, std::vector<int>>::iterator iter = InputData.find(sampleName);
                   if (iter != InputData.end()){
		                  results = operator+(results,temp_round);
		                  InputData[sampleName] = results;
                   }
                   temp_round.clear();
             }

        }  // if (i==0) DONE)

     } // End for loop

     std::ofstream countfile;
     if (file_type == ".sample_cumulative_coverage_counts"){
         countfile.open(output_file_ + ".sample_cumulative_coverage_counts");
     }
     if (file_type == ".sample_interval_statistics"){
         countfile.open(output_file_ + ".sample_interval_statistics");
     }
     if (file_type == ".sample_statistics"){
         countfile.open(output_file_ + ".sample_statistics");
     }

     // Inserting Header:
     countfile << header << std::endl;
     //Inserting Elements:
     for (auto elem : InputData){
          countfile << elem.first << "\t" ;
          for (auto datapoint : elem.second ){
               countfile << datapoint << '\t';
          }
          countfile << std::endl;

          // If coverage counts files are used, coverage proportions are computed:
          if (file_type == ".sample_cumulative_coverage_counts"){
              FILE *normalized_file;
              normalized_file = fopen((output_file_ + ".sample_cumulative_coverage_proportions").c_str(), "a+");
              fprintf(normalized_file, "%s\n", header.c_str());
              double max_value = *max_element((elem.second).begin(), (elem.second).end());
              for (auto datapoint : elem.second ){
                   double normalized_value = datapoint/max_value;
                   if ( normalized_value < 0.01 ) normalized_value = 0.00;
                   fprintf(normalized_file, "%.2f\t", normalized_value);
	                 //normalized_file << std::setprecision(2) << normalized_value << '\t';
              }
              fprintf(normalized_file, "\n");
              fclose(normalized_file);
          }

          if (file_type == ".sample_statistics"){
              FILE *summary_file;
              summary_file = fopen((output_file_ + ".sample_summary").c_str(), "a+");
              std::string stat_header = "sample_id\ttotal	mean\tgranular_third_quartile\t";
              stat_header = stat_header + "granular_median\tgranular_first_quartile\t\%_bases_above_15\n";

              // Computing Mean:
              double mean = 0.000;
              int cov_value = 0;
              int total_coverage15x = 0;
              int total_coverage = 0;
              int total = 0;
              for (auto datapoint : elem.second ){
                   if (cov_value > 14) total_coverage15x += datapoint;
                   total_coverage += cov_value * datapoint;
                   cov_value += 1;
                   total += datapoint;
              };

              double pct15x = 100*float(total_coverage15x)/float(total_coverage);
              mean = total_coverage/total;

              // Computing Third Quartile, Mean and First Quartile :
              int left, right, previous;

              // Third Quartile:
              int Q3 = round(3*total/4);
              int checkQ3 = 0;
              int cov_indexQ3 = 0;
              previous = 0;
              for (auto datapoint : elem.second ){
                   if (checkQ3 < Q3){
                       previous = checkQ3;
                       cov_indexQ3 += 1;
                       checkQ3 += datapoint;
                   } else {
                       left = checkQ3 - previous;
                       right = Q3 - checkQ3;
                       if (left < right) cov_indexQ3--;
                       break;
                   }
              }

              // Computing Median:
              int Q2 = round(total/2);
              int checkQ2 = 0;
              int cov_indexQ2 = 0;
              previous = 0;
              for (auto datapoint : elem.second ){
                   if (checkQ2 < Q2){
                       previous = checkQ2;
                       cov_indexQ2 += 1;
                       checkQ2 += datapoint;
                   } else {
                       left = checkQ2 - previous;
                       right = Q2 - checkQ2;
                       if (left < right) cov_indexQ2--;
                       break;
                   }
              }

              // Computing First Quartile:
              int Q1 = round(total/4);
              int checkQ1 = 0;
              int cov_indexQ1 = 0;
              previous = 0;
              for (auto datapoint : elem.second ){
                   if (checkQ1 < Q1){
                       previous = checkQ1;
                       cov_indexQ1 += 1;
                       checkQ1 += datapoint;
                   } else {
                       left = checkQ1 - previous;
                       right = Q1 - checkQ1;
                       if (left < right) cov_indexQ1--;
                       break;
                   }
              }
              //fprintf(summary_file, "%s\t%d\t%.2f\t%d\t%d\t%d\t%.2f\n",
              //        sampleName, total_coverage, mean, cov_indexQ3,
              //        cov_indexQ2, cov_indexQ1, pct15x);
              fprintf(summary_file, "%s\n", sampleName.c_str());
              fclose(summary_file);
       } // sample_summary file generated
   }
   countfile.close(); countfile.clear();
}

void DepthCombineWorker::concatenate_outputs(std::string file_type) {
   std::string temp = "";
   std::string fileCov;
   if (file_type == ".sample_summary"){
      // All parts-[0-9].cov will be concatenated:
      temp = file_type;
      file_type = ".cov";
      fileCov = output_file_ + file_type;
   }
   std::ofstream concatenated_file(fileCov, std::ios::out | std::ios::app);
   for (int i = 0 ; i < input_files_.size() ; i++){
        DLOG(INFO) << "Concatenating File : " << input_files_[i] + file_type << std::endl;
        std::ifstream inputFile((input_files_[i] + file_type).c_str());
        std::string value;
        if (i == 0){
            concatenated_file << inputFile.rdbuf();
        } else {
            std::string grab_line;
            getline(inputFile, grab_line);
            while(getline(inputFile, grab_line)){
                  concatenated_file << grab_line << std::endl;
            }
        }
   }
   concatenated_file.close(); concatenated_file.clear();
}

void DepthCombineWorker::setup() {
  if (flag_intervalCoverage_) {
      merge_outputs(".sample_cumulative_coverage_counts");
      merge_outputs(".sample_interval_statistics");
      concatenate_outputs(".sample_interval_summary");
      concatenate_outputs(".sample_gene_summary");
  }
  if (flag_sampleSummary_) {
      merge_outputs(".sample_statistics");
  }
  if (flag_baseCoverage_) {
      concatenate_outputs("");
  }
}

} // namespace fcsgenome
