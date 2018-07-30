#include <iostream>
#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/HTCWorker.h"

namespace fcsgenome {

HTCWorker::HTCWorker(std::string ref_path,
      std::string intv_path,
      std::string input_path,
      std::string output_path,
      std::vector<std::string> extra_opts,
      std::vector<std::string> &intv_list,
      int  contig,
      bool flag_vcf,
      bool &flag_f,
      bool flag_gatk):
  Worker(1, get_config<int>("gatk.htc.nct", "gatk.nct"), get_config<bool>("use_gatk4"),extra_opts),
  produce_vcf_(flag_vcf),
  flag_gatk_(flag_gatk),
  ref_path_(ref_path),
  intv_path_(intv_path),
  input_path_(input_path),
  intv_list_(intv_list)
{
  // check input/output files
  output_path_ = check_output(output_path, flag_f);
}

void HTCWorker::check() {
  ref_path_   = check_input(ref_path_);
  intv_path_  = check_input(intv_path_);
  input_path_ = check_input(input_path_);
}

void HTCWorker::setup() {

  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx" << get_config<int>("gatk.htc.memory", "gatk.memory") << "g ";

  if (flag_gatk_ || get_config<bool>("use_gatk4") ) {
      cmd << "-jar " << get_config<std::string>("gatk4_path") << " HaplotypeCaller ";
  } else {
      cmd << "-jar " << get_config<std::string>("gatk_path") << " -T HaplotypeCaller ";
  }

  cmd << "-R " << ref_path_ << " "
      << "-I " << input_path_ << " ";

  for (int i = 0; i < intv_list_.size(); i++) {
    cmd << "-L " << intv_list_[i] << " ";
  }
  if (intv_list_.size() > 0 ) {
    cmd << "-isr INTERSECTION ";
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

  if (flag_gatk_ || get_config<bool>("use_gatk4")){
     cmd << "-L " << intv_path_ << " "
         << "-O " << output_path_ << " --native-pair-hmm-threads=1 ";
     cmd << "1> /dev/null";
  } else{
     if (!produce_vcf_) {
        if (!extra_opts_.count("--emitRefConfidence") && !extra_opts_.count("-ERC")) {
        // if the user has not specified the same arg in extra options, use our default values
           cmd << "--emitRefConfidence GVCF ";
        }
     }
     if (!extra_opts_.count("--variant_index_type")) {
        cmd << "--variant_index_type LINEAR ";
     }
     if (!extra_opts_.count("--variant_index_parameter")) {
        cmd << "--variant_index_parameter 128000 ";
     }

     cmd << "-L " << intv_path_ << " "
         << "-nct " << get_config<int>("gatk.htc.nct", "gatk.nct") << " "
         << "-o " << output_path_ << " ";

     cmd << "1> /dev/null";
  }

  cmd_ = cmd.str();
  LOG(INFO) << cmd_;
}

} // namespace fcsgenome
