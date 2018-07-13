#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/UGWorker.h"

namespace fcsgenome {

UGWorker::UGWorker(std::string ref_path,
      std::string input_path,
      std::string intv_path,
      std::string output_path,
      std::vector<std::string> extra_opts,
      std::vector<std::string> &intv_list,
      bool &flag_f):
  Worker(1, get_config<int>("gatk.ug.nt"),extra_opts),
  ref_path_(ref_path),
  input_path_(input_path),
  intv_path_(intv_path),
  intv_list_(intv_list)
{
  output_path_ = check_output(output_path, flag_f);
}

void UGWorker::check() {
  ref_path_   = check_input(ref_path_);
  input_path_ = check_input(input_path_);
  intv_path_  = check_input(intv_path_);
}

void UGWorker::setup() {
  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx" << get_config<int>("gatk.ug.memory") << "g "
      << "-jar " << get_config<std::string>("gatk_path") << " "
      << "-T UnifiedGenotyper "
      << "-R " << ref_path_ << " "
      << "-L " << intv_path_ << " "
      << "-nt " << get_config<int>("gatk.ug.nt") << " "
      << "-I " << input_path_ << " "
      // secret option to fix index fopen issue
      << "--disable_auto_index_creation_and_locking_when_reading_rods "
      << "-o " << output_path_ << " ";

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

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome

