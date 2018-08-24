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
      std::vector<std::string> &dbsnp_path,
      std::vector<std::string> &cosmic_path,
      std::string &germline_path,
      int  contig,
      bool &flag_f,
      bool flag_gatk): Worker(1, get_config<int>("gatk.mutect2.nct", "gatk.nct"), extra_opts),
  ref_path_(ref_path),
  intv_path_(intv_path),
  normal_path_(normal_path),
  tumor_path_(tumor_path),
  dbsnp_path_(dbsnp_path),
  cosmic_path_(cosmic_path),
  germline_path_(germline_path),
  flag_gatk_(flag_gatk)
{
  // check input/output files
  output_path_ = check_output(output_path, flag_f);
}

void Mutect2Worker::check() {
  ref_path_   = check_input(ref_path_);
  intv_path_  = check_input(intv_path_);
  normal_path_ = check_input(normal_path_);
  tumor_path_ = check_input(tumor_path_);
  for(int i = 0; i < dbsnp_path_.size(); i++) {
    dbsnp_path_[i] = check_input(dbsnp_path_[i]);
  }
  for(int j = 0; j < cosmic_path_.size(); j++) {
    cosmic_path_[j] = check_input(cosmic_path_[j]);
  }
}

void Mutect2Worker::setup() {

  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx" << get_config<int>("gatk.mutect2.memory", "gatk.memory") << "g ";

  if (flag_gatk_ || get_config<bool>("use_gatk4") ) {
      cmd << "-jar " << get_config<std::string>("gatk4_path") << " Mutect2 ";
  }
  else{
      cmd << "-jar " << get_config<std::string>("gatk_path") << " -T MuTect2 ";
  }

  cmd << "-R " << ref_path_ << " ";

  if (flag_gatk_ || get_config<bool>("use_gatk4") ) {
      cmd << "-I " << normal_path_ << " "
          << "-I " << tumor_path_ << " "
          << "-normal " << " normal "
          << "-tumor "  << " tumor "
          << "--germline_resource " << germline_path_ ;
  }
  else{
      cmd << "-I:normal " << normal_path_ << " "
          << "-I:tumor " << tumor_path_   << " ";

      if (!extra_opts_.count("--variant_index_type")) {
         cmd << "--variant_index_type LINEAR ";
      }

      if (!extra_opts_.count("--variant_index_parameter")) {
         cmd << "--variant_index_parameter 128000 ";
      }

      cmd << "-nct " << get_config<int>("gatk.mutect2.nct", "gatk.nct") << " "
          << "-o " << output_path_ << " ";

      for (int i = 0; i < dbsnp_path_.size(); i++) {
          cmd << "--dbsnp " << dbsnp_path_[i] << " ";
      }
      for (int j = 0; j < cosmic_path_.size(); j++) {
          cmd << "--cosmic " << cosmic_path_[j] << " ";
      }

  } // End checking GATK version

  cmd << "-L " << intv_path_ << " -isr INTERSECTION ";

  for (auto it = extra_opts_.begin(); it != extra_opts_.end(); it++) {
      cmd << it->first << " ";
      for ( auto vec_iter = it->second.begin(); vec_iter != it->second.end(); vec_iter++) {
         if (!(*vec_iter).empty() && vec_iter == it->second.begin()) {
            cmd << *vec_iter << " ";
          }
          else if (!(*vec_iter).empty()) {
                  cmd << it->first << " " << *vec_iter << " ";
          }
      }
  }

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

} // namespace fcsgenome
