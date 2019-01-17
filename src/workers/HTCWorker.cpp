#include <iostream>
#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/HTCWorker.h"

namespace fcsgenome {

HTCWorker::HTCWorker(std::string ref_path,
      std::vector<std::string> intv_paths,
      std::vector<std::string> input_paths,
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
  input_paths_(input_paths)
{
  // check input/output files
  output_path_ = check_output(output_path, flag_f);
}

void HTCWorker::check() {
  namespace fs = boost::filesystem;

  ref_path_   = check_input(ref_path_);

  // check user input for intv.bed first
  for (int i = 0; i < intv_paths_.size(); i++) {
    intv_paths_[i]  = check_input(intv_paths_[i]);
  }

  // check input paths
  bool found_bam_intv = false;
  for (int i = 0; i < input_paths_.size(); i++) {
    // check BAM input (*.bam)
    input_paths_[i] = check_input(input_paths_[i]);

    // check BAM index (*.bai | *.bam.bai)
    if (!fs::exists(input_paths_[i] + ".bai") &&
        !fs::exists(get_fname_by_ext(input_paths_[i], "bai"))) {
      DLOG(INFO) << input_paths_[i] + ".bai" << ": " << fs::exists(input_paths_[i] + ".bai");
      DLOG(INFO) << get_fname_by_ext(input_paths_[i], "bai") << ": " << fs::exists(get_fname_by_ext(input_paths_[i], "bai"));
      throw fileNotFound("cannot find BAM index for " + input_paths_[i]);
    }
    
    // check if interval for bam input exists (pre-generated)
    // here we make the assumption the intv have a pre-determined 
    // filename pattern, like part-00.bam and part-00.bed
    std::string intv = get_fname_by_ext(input_paths_[i], "bed");
    if (fs::exists(intv)) {
      intv_paths_.push_back(intv);
      DLOG(INFO) << "Using interval " << intv << " that comes with input "
                 << input_paths_[i];
      found_bam_intv = true;
    }
  }
  if (!found_bam_intv) {
    // if no interval comes with the input, use pre-defined ones
    std::string intv_dir = contig_intv_dir();
    std::string intv = get_contig_fname(intv_dir, contig_, "list", "intv");
    intv = check_input(intv);

    intv_paths_.push_back(intv);
    DLOG(INFO) << "Using interval " << intv << " generated for contig " << contig_;
  }
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

  cmd << "-R " << ref_path_   << " ";

  for (auto path : input_paths_) {
    cmd << "-I " << path << " ";
  }
  for (auto path : intv_paths_) {
    cmd << "-L " << path  << " ";
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
