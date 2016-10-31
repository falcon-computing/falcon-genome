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
      int  contig,
      bool &flag_f): Worker(1, get_config<int>("gatk.htc.nct")),
  ref_path_(ref_path),
  intv_path_(intv_path),
  input_path_(input_path)
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
      << "-Xmx" << get_config<int>("gatk.htc.memory") << "g "
      << "-jar " << get_config<std::string>("gatk_path") << " "
      << "-T HaplotypeCaller "
      << "-R " << ref_path_ << " "
      << "-I " << input_path_ << " "
      << "--emitRefConfidence GVCF "
      << "--variant_index_type LINEAR "
      << "--variant_index_parameter 128000 "
      << "-L " << intv_path_ << " "
      << "-nct " << get_config<int>("gatk.htc.nct") << " "
      << "-o " << output_path_ << " ";

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

} // namespace fcsgenome
