#include <iostream>
#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/DepthWorker.h"

namespace fcsgenome {

DepthWorker::DepthWorker(std::string ref_path,
      std::string intv_paths,
      std::string input_path,
      std::string output_path,
      std::string geneList_paths,
      int depthCutoff,
      std::vector<std::string> extra_opts,
      int  contig,
      bool &flag_f,
      bool &flag_baseCoverage,
      bool &flag_intervalCoverage,
      bool &flag_sampleSummary): Worker(1, get_config<int>("gatk.depth.nct", "gatk.nct"), extra_opts),
  ref_path_(ref_path),
  intv_paths_(intv_paths),
  input_path_(input_path),
  geneList_paths_(geneList_paths),
  depthCutoff_(depthCutoff),
  flag_baseCoverage_(flag_baseCoverage),
  flag_intervalCoverage_(flag_intervalCoverage),
  flag_sampleSummary_(flag_sampleSummary)
{
  // check input/output files
  output_path_ = check_output(output_path, flag_f);
}

void DepthWorker::check() {
  ref_path_   = check_input(ref_path_);
  //input_path_ = check_input(input_path_);
  intv_paths_  = check_input(intv_paths_);
  geneList_paths_  = check_input(geneList_paths_);
}

void DepthWorker::setup() {

  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx" << get_config<int>("gatk.depth.memory", "gatk.memory") << "g "
      << "-jar " << get_config<std::string>("gatk_path") << " "
      << "-T DepthOfCoverage "
      << "-R " << ref_path_ << " "
      << "-I " << input_path_ << " "
      << "-L " << intv_paths_ << " "
      << "-geneList " << geneList_paths_ << " ";

  for (auto it = extra_opts_.begin(); it != extra_opts_.end(); it++) {
    cmd << it-> first << " ";
    if (!it->second.empty()) {
      cmd << it->second << " ";
    }
  }

  cmd << "-nt " << get_config<int>("gatk.depth.nct", "gatk.nct") << " "
      << "-o " << output_path_ << " "
      << "-ct " << depthCutoff_ << " ";

  if (geneList_paths_.size() > 0 ) {
     cmd << "-isr INTERSECTION ";
  }

  if(!flag_baseCoverage_)
     cmd << "-omitBaseOutput ";
  if(!flag_sampleSummary_)
     cmd << "-omitSampleSummary ";

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

} // namespace fcsgenome
