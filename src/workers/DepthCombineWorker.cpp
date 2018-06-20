#include <algorithm>
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
      check_input(input_files_[i] + ".sample_interval_summary");
    }
    else if (flag_sampleSummary_) {
      check_input(input_files_[i] + ".sample_summary");
    }
  }
}


void DepthCombineWorker::merge_gene_summary(std::string file_type) {
  double total = 0;
  double mean_cov = 0;
  std::vector<int> repeat_pos;
  std::vector<std::vector<std::string> > matrix;
  std::ofstream fout;
  int itr=0;
  fout.open(output_file_+file_type, std::ofstream::app);
  for (int i = 0; i < input_files_.size(); i++) {
    std::string filename = input_files_[i] + file_type;
    std::ifstream fin;
    fin.open(filename);
    
    if (fin.is_open()) { 
      std::string line;
      while (std::getline(fin, line)) {
        int col=0;
        std::vector<std::string> each_line;
        if (boost::contains(filename, ".sample_gene_summary")) {
          if (!boost::starts_with(line, "Gene")) {
            itr++;
            std::string each_str;
            std::istringstream iss(line);
            int bool_val=0;
            int pos;
            while (iss >> each_str) {  
              if (itr != 1) {
                if (col == 0) {
                  for (int j = 0; j < matrix.size(); j++) {
                    if (matrix[j][0].compare(each_str) == 0) { 
                      pos=j;
                      if(std::find(repeat_pos.begin(), repeat_pos.end(), j) !=repeat_pos.end()) {
                        for (int vect_iter=0; vect_iter < repeat_pos.size(); vect_iter++) {  
                        }                      
                      }
                      else {
                        repeat_pos.push_back(j);
                        mean_cov = boost::lexical_cast<double>(matrix[j][2]);
                        matrix[j][2] = boost::lexical_cast<std::string>(boost::lexical_cast<double>(matrix[j][1])/boost::lexical_cast<double>(matrix[j][2]));
                        mean_cov = boost::lexical_cast<double>(matrix[j][4]);
                        matrix[j][4] = boost::lexical_cast<std::string>(boost::lexical_cast<double>(matrix[j][1])/boost::lexical_cast<double>(matrix[j][4]));
                        matrix[j][8] = boost::lexical_cast<std::string>(boost::lexical_cast<double>(matrix[j][8]) * boost::lexical_cast<double>(matrix[j][1]) / mean_cov); 
                      }
                      bool_val=1;
                      break;
                    }
                  }
                }
              }   
              if (bool_val == 0) {
                if (col == 0) {
                  matrix.push_back(std::vector<std::string>());     
                }
                matrix[itr-1].push_back(each_str);  
              }
              else {   
                if (col == 1 || col == 3) {
                  total = boost::lexical_cast<double>(each_str); 
                  matrix[pos][col] = boost::lexical_cast<std::string>((boost::lexical_cast<double>(matrix[pos][col])) + (boost::lexical_cast<double>(each_str))); 
                  if (col == 1) { 
                    itr--;
                  } 
                }
                else if (col == 2 || col == 4) {
                  mean_cov = boost::lexical_cast<double>(each_str);   
                  matrix[pos][col] = boost::lexical_cast<std::string>(boost::lexical_cast<double>(matrix[pos][col]) + (total/mean_cov));
                }
                else if (col == 5 || col == 6 || col == 7) {
                  //matrix[pos][col] = boost::lexical_cast<std::string>(boost::lexical_cast<double>(matrix[pos][col]) + (boost::lexical_cast<double>(each_str) * total));
                  matrix[pos][col] = boost::lexical_cast<std::string>(boost::lexical_cast<double>(matrix[pos][col]) + (boost::lexical_cast<double>(each_str) * total / mean_cov));
                }
                else if (col == 8) {
                  matrix[pos][col] = boost::lexical_cast<std::string>(boost::lexical_cast<double>(matrix[pos][col]) + (boost::lexical_cast<double>(each_str) * total / mean_cov )); 
                } 
              }         
              col++;
            } 
          }
        }      
      }
    }
  }
  for (int vect_iter=0; vect_iter < repeat_pos.size(); vect_iter++) {
   int col=0;
   double sum_ratio; 
   while (col < 9) {
    if (col == 2 || col == 4) {
      sum_ratio = boost::lexical_cast<double>(matrix[repeat_pos[vect_iter]][col]);
      matrix[repeat_pos[vect_iter]][col]=boost::lexical_cast<std::string>(boost::lexical_cast<double>(matrix[repeat_pos[vect_iter]][1])/boost::lexical_cast<double>(matrix[repeat_pos[vect_iter]][col]));
    }
    else if (col == 5 || col == 6 || col == 7) {
      //matrix[repeat_pos[vect_iter]][col]=boost::lexical_cast<std::string>(boost::lexical_cast<double>(matrix[repeat_pos[vect_iter]][col])/boost::lexical_cast<double>(matrix[repeat_pos[vect_iter]][1]));
      matrix[repeat_pos[vect_iter]][col]=boost::lexical_cast<std::string>(boost::lexical_cast<double>(matrix[repeat_pos[vect_iter]][col])/sum_ratio);
    }
    else if (col == 8) {
      matrix[repeat_pos[vect_iter]][col]=boost::lexical_cast<std::string>(boost::lexical_cast<double>(matrix[repeat_pos[vect_iter]][col])/sum_ratio);
    } 
    col++;
  }
 }
  if (file_type.compare(".sample_gene_summary") == 0) {
    for (int i = 0; i < matrix.size(); i++) {
      for (int j = 0; j < matrix[i].size(); j++) {
        fout << matrix[i][j] << "\t";
      }
      fout << "\n";
    }
  }
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
    fin.open(filename);
    if (fin.is_open()) { 
      std::string line;
      while (std::getline(fin, line)) {
        std::vector<std::string> each_line;
        if (boost::contains(filename, ".sample_summary")) {
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
