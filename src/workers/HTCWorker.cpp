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
      std::vector<std::string> extra_opts,
      int  contig,
      bool flag_vcf,
      bool &flag_f): 
  Worker(1, get_config<int>("gatk.htc.nct", "gatk.nct"), extra_opts),
  produce_vcf_(flag_vcf),
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
      << "-Xmx" << get_config<int>("gatk.htc.memory", "gatk.memory") << "g "
      << "-jar " << get_config<std::string>("gatk_path") << " "
      << "-T HaplotypeCaller "
      << "-R " << ref_path_ << " "
      << "-I " << input_path_ << " ";
  std::map<std::string, std::string>::iterator it;
    if (!produce_vcf_) {
      it = extra_opts_.find("--emitRefConfidence");
      if(it != extra_opts_.end()) {
        cmd << it->first << " " << it->second << " ";
        extra_opts_.erase(it);
      }
      else {
        cmd << "--emitRefConfidence GVCF ";
      }
    }  
    
    it = extra_opts_.find("--variant_index_type");
    if (it != extra_opts_.end()) {
      cmd << it->first << " " << it->second << " ";
      extra_opts_.erase(it);
    }
    else {
      cmd << "--variant_index_type LINEAR ";
    }
    
    it = extra_opts_.find("--variant_index_parameter");
    if (it != extra_opts_.end()) {
      cmd << it->first << " " << it->second << " ";
      extra_opts_.erase(it);
    }
    else {
      cmd << "--variant_index_parameter 128000 ";
    }
 
  cmd << "-L " << intv_path_ << " "
      << "-nct " << get_config<int>("gatk.htc.nct", "gatk.nct") << " "
      << "-o " << output_path_ << " ";
  for (it = extra_opts_.begin(); it != extra_opts_.end(); it++) {
    cmd << it->first << " " << it->second << " ";
  } 
  cmd << "1> /dev/null";

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

} // namespace fcsgenome
