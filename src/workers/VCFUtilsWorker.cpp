#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/VCFUtilsWorker.h"

namespace fcsgenome {

VCFConcatWorker::VCFConcatWorker(
      std::vector<std::string> &input_files,
      std::string output_path,
      bool &flag_a,
      bool &flag_f): Worker(1, 1), input_files_(input_files), flag_a_(flag_a)
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
  if(flag_a_) {
    cmd << get_config<std::string>("bcftools_path") << " concat -a " 
        << "-o " << output_file_ << " ";
  }
  else {
    cmd << get_config<std::string>("bcftools_path") << " concat " 
        << "-o " << output_file_ << " ";
  }
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
  cmd << get_config<std::string>("bcftools_path") << " norm "
      << "-m +any " 
      << "-O z " 
      << "-o " << output_file_ << " "
      << input_file_;

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

void TabixWorker::check() {
  path_ = check_input(path_);
}

void TabixWorker::setup() {
  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("tabix_path") << " -p vcf " 
      << path_;
  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

void VCFSortWorker::check() {
  path_ = check_input(path_);
}

void VCFSortWorker::setup() {
  std::stringstream cmd;
  cmd << "("
      << "grep -v \"^[^#;]\" " << path_ << " && "
      << "grep \"^[^#;]\" " << path_ << " | sort -V"
      << ") > " << path_ << ".sort";
  cmd << ";";
  cmd << "mv " << path_ << ".sort " << path_;

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome
