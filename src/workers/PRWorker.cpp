#include <glog/logging.h>
#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/PRWorker.h"

namespace fcsgenome {

PRWorker::PRWorker(std::string ref_path,
      std::string intv_path,
      std::string bqsr_path,
      std::string input_path,
      std::string output_path,
      int  contig,
      bool &flag_f): Worker(conf_gatk_ncontigs, 1),
  ref_path_(ref_path),
  intv_path_(intv_path),
  bqsr_path_(bqsr_path),
  input_path_(input_path)
{
  // check output files
  output_path_ = check_output(output_path, flag_f);
}

void PRWorker::check() {
  ref_path_    = check_input(ref_path_);
  intv_path_   = check_input(intv_path_);
  bqsr_path_   = check_input(bqsr_path_);
  input_path_  = check_input(input_path_);

  DLOG(INFO) << "intv is " << intv_path_;
  DLOG(INFO) << "output is " << output_path_;
}

void PRWorker::setup() {
  // create cmd
  std::stringstream cmd;
  cmd << conf_java_call << " "
      << "-Xmx" << conf_pr_memory << "g "
      << "-jar " << conf_gatk_path << " "
      << "-T PrintReads "
      << "-R " << ref_path_ << " "
      << "-I " << input_path_ << " "
      << "-BQSR " << bqsr_path_ << " "
      << "-L " << intv_path_ << " "
      << "-nct " << conf_pr_nct << " "
      << "-o " << output_path_ << " "
      << "1> /dev/null";

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

} // namespace fcsgenome
