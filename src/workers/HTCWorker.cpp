#include <iostream>
#include <string>
#include <vector>

#include "fcs-genome/BamInput.h"
#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/HTCWorker.h"

namespace fcsgenome {

HTCWorker::HTCWorker(std::string ref_path,
      std::vector<std::string> intv_paths,
      //std::vector<std::string> input_paths,
      std::string input_paths,
      std::string output_path,
      std::vector<std::string> extra_opts,
      int contig,
      bool flag_vcf,
      bool &flag_f,
      bool flag_gatk):
  Worker(1, get_config<int>("gatk.htc.nct", "gatk.nct"), 
      extra_opts, 
      "HaplotypeCaller"),
  contig_(contig),
  produce_vcf_(flag_vcf),
  flag_gatk_(flag_gatk),
  ref_path_(ref_path),
  intv_paths_(intv_paths),
  input_paths_(input_paths),
  output_path_(output_path)
{
  output_path_ = check_output(output_path, flag_f);
}

void HTCWorker::check() {
  ref_path_  = check_input(ref_path_);
  if (intv_paths_.size()>0){  
    for (auto path : intv_paths_){
       path = check_input(path);
    }
  }
  BamInputInfo data_ = input_paths_.getInfo();
  data_ = input_paths_.merge_bed(contig_);
  data_.bam_name = check_input(data_.bam_name); 
}

void HTCWorker::setup() {
  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx" << get_config<int>("gatk.htc.memory", "gatk.memory") << "g ";

  if (flag_gatk_ || get_config<bool>("use_gatk4") ) {
    cmd << "-jar " << get_config<std::string>("gatk4_path") << " HaplotypeCaller ";
  } else {
    cmd << "-jar " << get_config<std::string>("gatk_path") << " -T HaplotypeCaller ";
  }

  cmd << " -R " << ref_path_    << " ";

  cmd << input_paths_.get_gatk_args(contig_);

  for (auto path: intv_paths_){
    cmd <<  " -L " << path << " ";
  }

  cmd << " -isr INTERSECTION ";

  for (auto option : extra_opts_) {
    if (option.second.size() == 0) {
      cmd << option.first << " ";
    }
    else {
      for (auto val : option.second) {
        if (!val.empty()) {
          cmd << option.first << " " << val << " ";
        }
      }
    }
  }

  if (flag_gatk_ || get_config<bool>("use_gatk4")){
    cmd << "-O " << output_path_  << " "
        << " --native-pair-hmm-threads=" << get_config<int>("gatk.htc.nct", "gatk.nct") << " ";
    if (!produce_vcf_){
      cmd << "--emit-ref-confidence=GVCF" << " ";
    } else {
     // This is the DEFAULT:
     cmd << "--emit-ref-confidence=NONE" << " " ;
    }
  } else{
     if (!produce_vcf_) {
       if (!extra_opts_.count("--emitRefConfidence") && !extra_opts_.count("-ERC")) {
         // if the user has not specified the same arg in extra options, use our default values
         cmd << "--emitRefConfidence GVCF ";
       }
     }
     if (!extra_opts_.count("--variant_index_type")) {
       cmd << "--variant_index_type LINEAR ";
     }
     if (!extra_opts_.count("--variant_index_parameter")) {
       cmd << "--variant_index_parameter 128000 ";
     }
     cmd << "-nct " << get_config<int>("gatk.htc.nct", "gatk.nct") << " "
         << "-o " << output_path_ << " ";
  }

  cmd << "1> /dev/null";
  cmd_ = cmd.str();

  DLOG(INFO) << cmd_;
}

} // namespace fcsgenome
