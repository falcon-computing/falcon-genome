#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/GenotypeGVCFsWorker.h"

namespace fcsgenome {

GenotypeGVCFsWorker::GenotypeGVCFsWorker(
    std::string ref_path,
    std::string input_path,
    std::string output_path,
    bool &flag_f): Worker(1, 1),
  ref_path_(ref_path),
  input_path_(input_path)
{
  output_path_ = check_output(output_path, flag_f);
}

void GenotypeGVCFsWorker::check() {
  ref_path_   = check_input(ref_path_);
  input_path_ = check_input(input_path_);
}

void GenotypeGVCFsWorker::setup() {
  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx" << get_config<int>("gatk.genotype.memory") << "g "
      << "-jar " << get_config<std::string>("gatk_path") << " "
      << "-T GenotypeGVCFs "
      << "-R " << ref_path_ << " "
      << "--variant " << input_path_ << " "
      // secret option to fix index fopen issue
      << "--disable_auto_index_creation_and_locking_when_reading_rods "
      << "-o " << output_path_;

  cmd_ = cmd.str();
}
} // namespace fcsgenome
