#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/Mutect2Worker.h"

namespace fcsgenome {

Mutect2Worker::Mutect2Worker(std::string ref_path,
      std::string intv_path,
      std::string input_path1,
      std::string input_path2,
      std::string output_path,
      std::string dbsnp_path,
      int  contig,
      bool &flag_f): Worker(1, get_config<int>("gatk.mutect2.nct")),
  ref_path_(ref_path),
  intv_path_(intv_path),
  input_path1_(input_path1),
  input_path2_(input_path2),
  dbsnp_path_(dbsnp_path)
{
  // check input/output files
  output_path_ = check_output(output_path, flag_f);
}

void Mutect2Worker::check() {
  ref_path_   = check_input(ref_path_);
  intv_path_  = check_input(intv_path_);
  input_path1_ = check_input(input_path1_);
  input_path2_ = check_input(input_path2_);
  dbsnp_path_ = check_input(dbsnp_path_);
}

void Mutect2Worker::setup() {

  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx" << get_config<int>("gatk.mutect2.memory") << "g "
      << "-jar " << get_config<std::string>("gatk_path") << " "
      << "-T MuTect2 "
      << "-R " << ref_path_ << " "
      << "-I:normal " << input_path1_ << " "
      << "-I:tumor " << input_path2_ << " ";

  cmd << "--variant_index_type LINEAR "
      << "--variant_index_parameter 128000 "
      << "-L " << intv_path_ << " "
      << "-nct " << get_config<int>("gatk.mutect2.nct") << " "
      << "-o " << output_path_ << " "
      << "--dbsnp " << dbsnp_path_ << " ";

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

} // namespace fcsgenome
                           
