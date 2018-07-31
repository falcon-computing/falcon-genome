#include <iostream>
#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/Mutect2Worker.h"

namespace fcsgenome {

Mutect2Worker::Mutect2Worker(std::string ref_path,
      std::string intv_path,
      std::string normal_path,
      std::string tumor_path,
      std::string output_path,
      std::vector<std::string> extra_opts,
      std::string &dbsnp_path,
      std::string &cosmic_path,
      std::string &intv_list,
      int  contig,
      bool &flag_f): Worker(1, get_config<int>("gatk.mutect2.nct", "gatk.nct"), extra_opts),
  ref_path_(ref_path),
  intv_path_(intv_path),
  normal_path_(normal_path),
  tumor_path_(tumor_path),
  dbsnp_path_(dbsnp_path),
  cosmic_path_(cosmic_path),
  intv_list_(intv_list)
{
  // check input/output files
  output_path_ = check_output(output_path, flag_f);
}

void Mutect2Worker::check() {
  ref_path_   = check_input(ref_path_);
  tumor_path_ = check_input(tumor_path_);
  if (!intv_path_.empty())   intv_path_   = check_input(intv_path_);
  if (!normal_path_.empty()) normal_path_ = check_input(normal_path_);
  if (!dbsnp_path_.empty())  dbsnp_path_  = check_input(dbsnp_path_);
  if (!cosmic_path_.empty()) cosmic_path_ = check_input(cosmic_path_);
}

void Mutect2Worker::setup() {

  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx" << get_config<int>("gatk.mutect2.memory", "gatk.memory") << "g "
      << "-jar " << get_config<std::string>("gatk_path") << " "
      << "-T MuTect2 "
      << "-R " << ref_path_ << " "
      << "-I:tumor " << tumor_path_ << " ";

  if (!normal_path_.empty()){
      cmd << "-I:normal " << normal_path_ << " ";
  } else {
      cmd << "--artifact_detection_mode " << " ";
  }

  if (!intv_path_.empty()){
      cmd << "-L " << intv_path_ << " ";
  }

  if (!intv_list_.empty()){
      cmd << "-L " << intv_list_ << " ";
  }

  if (!dbsnp_path_.empty()){
      cmd << "--dbsnp " << dbsnp_path_ << " ";
  }

  if (!cosmic_path_.empty()){
      cmd << "--cosmic " << cosmic_path_ << " ";
  }

  cmd << "-nct " << get_config<int>("gatk.mutect2.nct", "gatk.nct") << " "
      << "-o " << output_path_ << " ";

  if (intv_list_.size() > 0 ) {
    cmd << "-isr INTERSECTION ";
  }

  for (auto it = extra_opts_.begin(); it != extra_opts_.end(); it++) {
    cmd << it->first << " ";
    for( auto vec_iter = it->second.begin(); vec_iter != it->second.end(); vec_iter++) {
      if (!(*vec_iter).empty() && vec_iter == it->second.begin()) {
        cmd << *vec_iter << " ";
      }
      else if (!(*vec_iter).empty()) {
        cmd << it->first << " " << *vec_iter << " ";
      }
    }
  }

  if(!extra_opts_.count("--variant_index_type")) {
    cmd << "--variant_index_type LINEAR ";
  }

  if (!extra_opts_.count("--variant_index_parameter")) {
    cmd << "--variant_index_parameter 128000 ";
  }

  cmd_ = cmd.str();
  LOG(INFO) << cmd_;
}

} // namespace fcsgenome
