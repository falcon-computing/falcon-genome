#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/DOCWorker.h"

namespace fcsgenome {

DOCWorker::DOCWorker(std::string ref_path,
      std::string input_path,
      std::string output_path,
      bool &flag_f): Worker(1, 4),
  ref_path_(ref_path),
  input_path_(input_path)
{
  // check input/output files
  output_path_ = check_output(output_path, flag_f);
}

void DOCWorker::check() {
  ref_path_   = check_input(ref_path_);
  input_path_ = check_input(input_path_);
}

void DOCWorker::setup() {

  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx 8g "
      << "-jar " << get_config<std::string>("gatk_path") << " "
      << "-T DepthOfCoverage "
      << "-R " << ref_path_ << " "
      << "-I " << input_path_ << " "
      << "-omitBaseOutput "
      << "-omitLocusTable "
      << "-nt 24 "
      << "-ct 9 "
      << "-o " << output_path_ << " ";

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

} // namespace fcsgenome
