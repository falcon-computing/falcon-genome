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
      bool &flag_f): Worker(get_config<int>("gatk.bqsr.nprocs"), 1),
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
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx" << get_config<int>("gatk.bqsr.memory") << "g "
      << "-jar " << get_config<std::string>("gatk_path") << " "
      << "-T BaseRecalibrator "
      << "-R " << ref_path_ << " "
      << "-I " << input_path_ << " "
      << "-L " << intv_path_ << " "
      << "-nct " << get_config<int>("gatk.bqsr.nct") << " "
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
  cmd << get_config<std::string>("java_path") << " "
      << "-cp " << get_config<std::string>("gatk_path") << " "
      << "org.broadinstitute.gatk.tools.GatherBqsrReports ";
  for (int i = 0; i < input_files_.size(); i++) {
    cmd << "I=" << input_files_[i] << " ";
  }
  cmd << "O=" << output_file_;

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

PRWorker::PRWorker(std::string ref_path,
      std::string intv_path,
      std::string bqsr_path,
      std::string input_path,
      std::string output_path,
      int  contig,
      bool &flag_f): Worker(get_config<int>("gatk.ncontigs"), 1),
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
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx" << get_config<int>("gatk.pr.memory") << "g "
      << "-jar " << get_config<std::string>("gatk_path") << " "
      << "-T PrintReads "
      << "-R " << ref_path_ << " "
      << "-I " << input_path_ << " "
      << "-BQSR " << bqsr_path_ << " "
      << "-L " << intv_path_ << " "
      << "-nct " << get_config<int>("gatk.pr.nct") << " "
      << "-o " << output_path_ << " "
      << "1> /dev/null";

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome
