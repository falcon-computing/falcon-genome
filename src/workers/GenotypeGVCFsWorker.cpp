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
    std::vector<std::string> extra_opts,
    bool &flag_f,
    bool flag_gatk): Worker(1, 1, extra_opts, "GenotypeGVCF"),
  ref_path_(ref_path),
  input_path_(input_path),
  flag_gatk_(flag_gatk)
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
      << "-Xmx" << get_config<int>("gatk.genotype.memory") << "g ";

  if (flag_gatk_ || get_config<bool>("use_gatk4")) {
    cmd << "-jar " << get_config<std::string>("gatk4_path") << " "
        << " GenotypeGVCFs "
        << " -R " << ref_path_ << " " 
        << " -V gendb://" << input_path_ << " -G StandardAnnotation " 
        << " -O " << output_path_ ; 
  }
  else{
    cmd << "-jar " << get_config<std::string>("gatk_path") << " "
        << "-T GenotypeGVCFs "
        << "-R " << ref_path_ << " "
        << "--variant " << input_path_ << " "
        // secret option to fix index fopen issue
        << "--disable_auto_index_creation_and_locking_when_reading_rods "
        << "-o " << output_path_ << " ";
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

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;

}
} // namespace fcsgenome
