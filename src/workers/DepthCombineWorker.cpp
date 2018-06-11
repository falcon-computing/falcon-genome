#include <iomanip>
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
  double total = 0;
  double mean_cov = 0;
  std::vector<double> contents;
  std::ofstream fout;
  fout.open(output_file_+file_type, std::ofstream::app);
  for (int i = 0; i < input_files_.size(); i++) {
    std::string filename = input_files_[i] + file_type;
    std::ifstream fin;
    fin.open(filename);
    
    if (fin.is_open()) { 
      std::string line;

      while (std::getline(fin, line)) {
        
        if (boost::contains(filename, ".sample_interval_summary") | boost::contains(filename, ".sample_gene_summary")) {
          if (boost::contains(filename, "part-00")) { 
            fout << line << "\n";
          }
          else {
            if (!boost::starts_with(line, "Target") & !boost::starts_with(line, "Gene")) {    
                fout << line << "\n";
            }  
          }
        }
        else if (boost::contains(filename, ".sample_summary")) {
          if (!boost::starts_with(line, "sample_id") & !boost::starts_with(line, "Total")) { 
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
                    contents[itr-1] = contents[itr-1] + (boost::lexical_cast<double>(i));
                  }
                  else {
                    contents.push_back(0);
                    if (itr == 2) {
                      mean_cov = boost::lexical_cast<double>(i);
                    }
                    //contents[itr-1] = contents[itr-1] + (boost::lexical_cast<double>(i) * total);
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
                  else if ( itr == 3 || itr == 4 || itr == 5 ) {
                    DLOG(INFO) << itr << " " << (boost::lexical_cast<double>(i)) << "\n";
                    contents[itr-1] = contents[itr-1] + (boost::lexical_cast<double>(i));
                  }
                  else {
                    //contents[itr-1] = contents[itr-1] + (boost::lexical_cast<double>(i) * total);
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
      DLOG(INFO) << std::fixed << std::setprecision(2) << contents[i] << "\t";
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
 
void DepthCombineWorker::setup() {
  if (flag_intervalCoverage_) {
    merge(".sample_interval_summary");
    merge(".sample_gene_summary");
  }
  if (flag_sampleSummary_) {
    merge(".sample_summary");
  }
}
} // namespace fcsgenome
