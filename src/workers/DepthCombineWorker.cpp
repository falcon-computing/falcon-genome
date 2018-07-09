#include <algorithm>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/lexical_cast.hpp>

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

          //if (flag_intervalCoverage_) {
          //    check_input(input_files_[i] + ".sample_interval_summary");
          //}
          //else if (flag_sampleSummary_) {
          //    check_input(input_files_[i] + ".sample_summary");
          //}
     }
}

void DepthCombineWorker::merge_outputs(std::string file_type) {
     std::map<std::string, std::vector<int> > InputData;
     std::string sampleName;
     std::vector<int> results;
     std::vector<int> temp_round;
     std::vector<std::string> temp;

     std::string header;

     for (int i = 0 ; i < sizeof(input_files_) ; i++){
        std::ifstream file((input_files_[i] + file_type).c_str());
        std::string value;
        getline(file,value);
        // Getting the header:
        if (i==0){
            header = value;
        };
        // Getting data for each sample:
        std::string grab_line;
        std::string sampleName;

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

   countfile << header << std::endl;
   for (auto elem : InputData){
        countfile << elem.first << "\t" ;
        for (auto datapoint : elem.second ){
             countfile << datapoint << '\t';
        }
        countfile << std::endl;

        if (file_type == ".sample_cumulative_coverage_counts"){
            std::ofstream normalized_file;
            normalized_file.open(output_file_ + ".sample_cumulative_coverage_proportions", std::ofstream::out | std::ofstream::app));
            double max_value = *max_element((elem.second).begin(), (elem.second).end());
            for (auto datapoint : elem.second ){
                 double normalized_value = datapoint/max_value;
                 if ( normalized_value < 0.01 ) normalized_value = 0.00;
	               normalized_file << std::setprecision(2) << normalized_value << '\t';
            }
            normalized_file << std::endl;
            normalized_file.close();
            normalized_file.clear();
        }


   }
   countfile.close(); countfile.clear();








}


void DepthCombineWorker::merge_interval_summary(std::string file_type) {
  std::ofstream fout;
  fout.open(output_file_+file_type, std::ofstream::app);
  for (int i = 0; i < input_files_.size(); i++) {
    std::string filename = input_files_[i] + file_type;
    std::ifstream fin;
    fin.open(filename);
    if (fin.is_open()) {
      std::string line;
      while (std::getline(fin, line)) {
        std::vector<std::string> each_line;
        if (boost::contains(filename, ".sample_interval_summary")) {
          if (boost::contains(filename, "part-00")) {
            fout << line << "\n";
          }
          else {
            if (!boost::starts_with(line, "Target")) {
              fout << line << "\n";
            }
          }
        }
      }
    }
  }
}

void DepthCombineWorker::merge_sample_summary(std::string file_type) {
  double total = 0;
  double mean_cov = 0;
  std::vector<double> contents;
  std::ofstream fout;
  fout.open(output_file_+file_type, std::ofstream::app);
  for (int i = 0; i < input_files_.size(); i++) {
    std::string filename = input_files_[i] + file_type;
    std::ifstream fin;
    fin.open(filename);sample_cumulative_coverage_counts
    if (fin.is_open()) {
      std::string line;
      while (std::getline(fin, line)) {
        std::vector<std::string> each_line;
        if (boost::contains(filename, ".sample_summary")) {
          if (!boost::starts_with(line, "sample_id") & !boost::starts_depth/part-10.cov.sample_interval_summarywith(line, "Total")) {
            std::string i;
            int itr = 0;
            std::istringstream iss(line);
            while (iss >> i) {
              if (itr != 0) {
                if (boost::contains(filename, "part-00")) {
                  if (itr == 1) {
                    total = boost::lexical_cast<double>(i);
                    contents.push_back(0);
                    contents[itr-1] = contents[itr-1]+total;
                  }
                  else if (itr == 6) {
                    contents.push_back(0);
                    contents[itr-1] = contents[itr-1] + (boost::lexical_cast<double>(i) * total / mean_cov);
                  }
                  else if ( itr == 3 || itr == 4 || itr == 5 ) {
                    contents.push_back(0);
                    contents[itr-1] = contents[itr-1] + (boost::lexical_cast<double>(i) * total / mean_cov);
                  }
                  else {
                    contents.push_back(0);
                    if (itr == 2) {
                      mean_cov = boost::lexical_cast<double>(i);
                    }
                    contents[itr-1] = contents[itr-1] + (total/boost::lexical_cast<double>(i));
                  }
                }
                else {
                  if (itr == 1) {
                    total = boost::lexical_cast<double>(i);
                    contents[itr-1] = contents[itr-1] + total;
                  }
                  else if (itr == 6) {
                    contents[itr-1] = contents[itr-1] + (boost::lexical_cast<double>(i) * total / mean_cov);
                  }
                  else if (itr == 3 || itr == 4 || itr == 5) {
                    contents[itr-1] = contents[itr-1] + (boost::lexical_cast<double>(i) * total / mean_cov);
                  }
                  else {
                    // total/mean
                    if (itr == 2) {
                      mean_cov = boost::lexical_cast<double>(i);
                    }
                    contents[itr-1] = contents[itr-1] + (total/boost::lexical_cast<double>(i));
                  }
                }
              }
              ++itr;
            }
          }
          else if (boost::starts_with(line, "sample_id") & boost::contains(filename, "part-00")) {
            fout << line << "\n";
          }
        }
      }
    }
  }

  if (file_type.compare(".sample_summary") == 0) {
    fout << output_file_ << "\t";
    for (int i = 0; i < contents.size(); i++) {
      if (i == 0) {
        fout << std::fixed << std::setprecision(0) << contents[i] << "\t";
      }
      else if (i == 2 || i == 3 || i == 4) {
        fout << std::fixed << std::setprecision(0) << contents[i]/contents[1] << "\t";
      }
      else if (i == 5) {
        fout << std::fixed << std::setprecision(2) << contents[i]/contents[1] << "\t";
      }
      else {
        fout << std::fixed << std::setprecision(2) << contents[0]/contents[i] << "\t";
      }
    }
  }
}

void DepthCombineWorker::merge_base_summary(std::string file_type) {
  std::ofstream fout;
  int itr=0;
  fout.open(output_file_+file_type, std::ofstream::app);
  namespace fs = boost::filesystem;
  std::string file_extension;
  file_extension = fs::file_size(output_file_);
  for (int i = 0; i < input_files_.size(); i++) {
    std::string filename = input_files_[i] + file_type;
    std::ifstream fin;
    fin.open(filename);
    if (fin.is_open()) {
      std::string line;
      while (std::getline(fin, line)) {
        if (!boost::contains(filename, file_extension+".sample_")) {
          if (boost::contains(filename, "part-00")) {
            fout << line << "\n";
          }
          else if (!boost::starts_with(line, "Locus")) {
            fout << line << "\n";
          }
        }
      }
    }
  }
}

void DepthCombineWorker::setup() {
  if (flag_intervalCoverage_) {
    merge_interval_summary(".sample_interval_summary");
    merge_gene_summary(".sample_gene_summary");
  }
  if (flag_sampleSummary_) {
    merge_sample_summary(".sample_summary");
  }
  if (flag_baseCoverage_) {
    merge_base_summary("");
  }
}
} // namespace fcsgenome
