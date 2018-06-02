#include <iostream>
#include <fstream>
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
      bool &flag_f): Worker(1, 1), input_files_(input_files), flag_baseCoverage_(flag_baseCoverage), flag_intervalCoverage_(flag_intervalCoverage), flag_sampleSummary_(flag_sampleSummary)
{
  // check output files
  output_file_ = check_output(output_path, flag_f);
}

void DepthCombineWorker::check() {
  for (int i = 0; i < input_files_.size(); i++) {
    if (flag_intervalCoverage_) {
      //input_files_[i] = check_input(input_files_[i] + ".sample_interval_summary");
    }
    else if (flag_sampleSummary_) {
      check_input(input_files_[i] + ".sample_summary");
    }
  }
}

void DepthCombineWorker::merge(std::string file_type) {
  int total;
  std::vector<float> contents;
  std::ofstream fout;
  fout.open(output_file_+file_type, std::ofstream::app);
  
  for (int i = 0; i < input_files_.size(); i++) {
    std::string filename = input_files_[i] + file_type;
    std::ifstream fin;
    fin.open(filename);
    
    if (fin.is_open()) {
      DLOG(INFO) << filename;
      std::string line;

      while (std::getline(fin, line)) {
        if (boost::contains(filename, ".sample_interval_summary") | boost::contains(filename, ".sample_gene_summary")) {
          if (boost::contains(filename, "part-00")) {
            DLOG(INFO) << line;
            fout << line << "\n";
          }
          else {
            if (!boost::starts_with(line, "Target") & !boost::starts_with(line, "Gene")) {
              DLOG(INFO) << line;
              fout << line << "\n";
            }
          }
        }
        else if (boost::contains(filename, ".sample_summary")) {
          if (!boost::starts_with(line, "sample_id") & !boost::starts_with(line, "Total")) {
            std::string i;
            int itr=0;
            std::istringstream iss(line);
            while (iss >> i) {
              if (itr != 0) {
                DLOG(INFO) << itr;
                DLOG(INFO) << filename;
                if (boost::contains(filename, "part-00")) {
                  if (itr == 1) {
                    total = strtof((i).c_str(),0);
                    contents.push_back(total);
                  }
                  else {
                    contents.push_back((strtof((i).c_str(),0))*total);
                  }
                }
                else {
                  if (itr == 1) {
                    total = strtof((i).c_str(),0);
                    contents[itr-1] = contents[itr-1] + total;
                  }
                  else {
                    contents[itr-1] += strtof((i).c_str(),0) * total;
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
  for (int i = 0; i < contents.size(); i++) {
    if (i == 0) {
      DLOG(INFO) << contents[i];
      fout  << contents[i] << "\t";
    }
    else {
      DLOG(INFO) << ("%.2f",contents[i]/contents[0]);
      fout << ("%.2f",contents[i]/contents[0]) << "\t";
    }
  }
}

void DepthCombineWorker::setup() {
  if (flag_intervalCoverage_) {
    merge(".sample_interval_summary");
  }
  if (flag_sampleSummary_) {
    merge(".sample_summary");
  }
}
} // namespace fcsgenome
