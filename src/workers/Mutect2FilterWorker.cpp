#include <boost/algorithm/string/replace.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/Mutect2FilterWorker.h"

namespace fcsgenome {

Mutect2FilterWorker::Mutect2FilterWorker(
    std::vector<std::string> intv_path,
    std::string input_path,
    std::string tumor_table,
    std::string output_path,
    std::vector<std::string> extra_opts,
    bool &flag_f,
    bool flag_gatk): Worker(1, get_config<int>("gatk.mutect2.nct", "gatk.nct"), extra_opts, "Generating Mutect2Filter VCF"),
  intv_path_(intv_path),     
  input_path_(input_path),
  tumor_table_(tumor_table),
  output_path_(output_path),
  flag_gatk_(flag_gatk)
{
  // check input/output files
  output_path_ = check_output(output_path, flag_f);
}

void Mutect2FilterWorker::check() {
  //intv_path_  = check_input(intv_path_);
   input_path_ = check_input(input_path_);
   if (!tumor_table_.empty()){
     tumor_table_ = check_input(tumor_table_);
   }
}

void Mutect2FilterWorker::setup() {
  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx" << get_config<int>("gatk.mutect2.memory", "gatk.memory") << "g " 
      << "-jar " << get_config<std::string>("gatk4_path") << " FilterMutectCalls "
      << "-V "   << input_path_   << " "  ;
  
  std::string test = input_path_;
  //std::replace(test.end()-3, test.end(), "vcf", "bed");  

  std::string ext[2] = {"bed", "list"};
  for (int k=0; k<2; k++){
    std::string target = get_fname_by_ext(input_path_, ext[k]);
    if (boost::filesystem::exists(target)){
      cmd << " -L " << target ;
    }
  };

  cmd << " -O " << output_path_  << " ";

  for (auto a: intv_path_){
   cmd << " -L " << a << " ";
  }

  cmd << " -isr INTERSECTION ";
  if (!tumor_table_.empty()){
    cmd  << " -contamination-table " << tumor_table_ << " ";
  }

  for (auto it = extra_opts_.begin(); it != extra_opts_.end(); it++) {
    cmd << it->first << " ";
    for ( auto vec_iter = it->second.begin(); vec_iter != it->second.end(); vec_iter++) {
       if (!(*vec_iter).empty() && vec_iter == it->second.begin()) {
          cmd << *vec_iter << " ";
        }
        else if (!(*vec_iter).empty()) {
                cmd << it->first << " " << *vec_iter << " ";
        }
    }
  }

  cmd_ = cmd.str();
  LOG(INFO) << cmd_;
}

} // namespace fcsgenome
