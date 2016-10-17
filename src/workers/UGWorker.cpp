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
      bool &flag_f):
  Worker(get_config<int>("gatk.ug.nprocs"), 1),
  ref_path_(ref_path),
  input_path_(input_path),
  intv_path_(intv_path)
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
      << "-o " << output_path_ << " ";
  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome
