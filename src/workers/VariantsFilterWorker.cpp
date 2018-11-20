#include <iostream>
#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/VariantsFilterWorker.h"

namespace fcsgenome {

VariantsFilterWorker::VariantsFilterWorker(std::string ref_path,
      std::string intv_path,
      std::string input_path,
      std::string output_path,
      std::vector<std::string> extra_opts,
      int  contig,
      std::string filtering_par,
      std::string filter_name,
      bool &flag_f,
      bool flag_gatk):
  Worker(1, get_config<int>("gatk.htc.nct", "gatk.nct"), extra_opts, "VariantsFilter"),
  flag_gatk_(flag_gatk),
  ref_path_(ref_path),
  intv_path_(intv_path),
  input_path_(input_path),
  filtering_par_(filtering_par),
  filter_name_(filter_name)
{
  // check input/output files
  output_path_ = check_output(output_path, flag_f);
}

void VariantsFilterWorker::check() {
  ref_path_   = check_input(ref_path_);
  intv_path_  = check_input(intv_path_);
  input_path_ = check_input(input_path_);
}

void VariantsFilterWorker::setup() {

  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx" << get_config<int>("gatk.htc.memory", "gatk.memory") << "g ";

  if (flag_gatk_ || get_config<bool>("use_gatk4") ) {
      cmd << "-jar " << get_config<std::string>("gatk4_path") << " VariantFiltration ";
  } else {
      cmd << "-jar " << get_config<std::string>("gatk_path") << " -T VariantFiltration ";
  }

  cmd << "-V " << input_path_ << " "
      << "-L " << intv_path_  << " "
      << " -isr INTERSECTION ";
 
  if (flag_gatk_ || get_config<bool>("use_gatk4") ) {
    cmd << "-O " << output_path_  << " ";
   } else {
    cmd << "-o " << output_path_ << " ";
  }

  for (auto it = extra_opts_.begin(); it != extra_opts_.end(); it++) {
    cmd << it->first << " ";
    for( auto vec_iter = it->second.begin(); vec_iter != it->second.end(); vec_iter++) {
      if (!(*vec_iter).empty() && vec_iter == it->second.begin()) {
        cmd << *vec_iter << " ";
      }
      else if (!(*vec_iter).empty()) {
        cmd << it->first << " " << *vec_iter << " ";
      }
    }
  }

  cmd << "1> /dev/null";
  cmd_ = cmd.str();

  DLOG(INFO) << cmd_;
}

} // namespace fcsgenome
