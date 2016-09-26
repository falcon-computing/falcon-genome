#include <glog/logging.h>
#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/BQSRWorker.h"

namespace fcsgenome {

BQSRWorker::BQSRWorker(std::string ref_path,
      std::vector<std::string> &known_sites,
      std::string intv_path,
      std::string input_path,
      std::string output_path,
      int  contig,
      bool &flag_f): Worker(conf_gatk_ncontigs, 1),
  ref_path_(ref_path),
  intv_path_(intv_path),
  input_path_(input_path),
  known_sites_(known_sites)
{
  output_path_ = check_output(output_path, flag_f);
}

void BQSRWorker::check() {
  ref_path_   = check_input(ref_path_);
  intv_path_  = check_input(intv_path_);
  input_path_ = check_input(input_path_);

  for (int i = 0; i < known_sites_.size(); i++) {
    known_sites_[i] = check_input(known_sites_[i]);
  }
}

void BQSRWorker::setup() {
  // create cmd
  std::stringstream cmd;
  cmd << conf_java_call << " "
      << "-Xmx" << conf_bqsr_memory << "g "
      << "-jar " << conf_gatk_path << " "
      << "-T BaseRecalibrator "
      << "-R " << ref_path_ << " "
      << "-I " << input_path_ << " "
      << "-L " << intv_path_ << " "
      << "-nct " << conf_bqsr_nct << " "
      << "-o " << output_path_ << " ";

  //for (int i = 0; i < input_paths.size(); i++) {
  //  cmd << "-I " << input_paths[i] << " ";
  //}
  for (int i = 0; i < known_sites_.size(); i++) {
    cmd << "-knownSites " << known_sites_[i] << " ";
  }

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

BQSRGatherWorker::BQSRGatherWorker(std::vector<std::string> &input_files,
    std::string output_file, bool &flag_f): Worker(1, 1),
  input_files_(input_files)
{
  output_file_ = check_output(output_file, flag_f);
}

void BQSRGatherWorker::check() {
  for (int i = 0; i < input_files_.size(); i++) {
    input_files_[i] = check_input(input_files_[i]);
  }
}

void BQSRGatherWorker::setup() {
  // create cmd
  std::stringstream cmd;
  cmd << conf_java_call << " "
      << "-cp " << conf_gatk_path << " "
      << "org.broadinstitute.gatk.tools.GatherBqsrReports ";
  for (int i = 0; i < input_files_.size(); i++) {
    cmd << "I=" << input_files_[i] << " ";
  }
  cmd << "O=" << output_file_;

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome
