#include <glog/logging.h>
#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/VCFConcatWorker.h"

namespace fcsgenome {

VCFConcatWorker::VCFConcatWorker(
      std::vector<std::string> &input_files,
      std::string output_path,
      bool &flag_f): Worker(1, 1), input_files_(input_files)
{
  // check output files
  output_file_ = check_output(output_path, flag_f);
}

void VCFConcatWorker::check() {
  for (int i = 0; i < input_files_.size(); i++) {
    input_files_[i] = check_input(input_files_[i]);
  }
}

void VCFConcatWorker::setup() {
  // create cmd
  std::stringstream cmd;
  cmd << conf_bcftools_call << " concat " 
      << "-o " << output_file_ << " ";
  for (int i = 0; i < input_files_.size(); i++) {
    cmd << input_files_[i] << " ";
  }
  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

ZIPWorker::ZIPWorker(
      std::string input_path,
      std::string output_path,
      bool &flag_f): Worker(1, 1)
{
  input_file_  = input_path;
  output_file_ = check_output(output_path, flag_f);
}

void ZIPWorker::check() {
  DLOG(INFO) << "ZIPWorker check " << input_file_;
  input_file_ = check_input(input_file_);
}

void ZIPWorker::setup() {
  // create cmd
  std::stringstream cmd;
  cmd << conf_bgzip_call << " -c " 
      << input_file_ << " "
      << "> " << output_file_;
  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

TabixWorker::TabixWorker(std::string input_path): Worker(1, 1) {
  input_file_ = input_path;
}

void TabixWorker::check() {
  DLOG(INFO) << "TabixWorker check " << input_file_;
  input_file_ = check_input(input_file_);
}

void TabixWorker::setup() {
  // create cmd
  std::stringstream cmd;
  cmd << conf_tabix_call << " -p vcf " 
      << input_file_;
  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome
