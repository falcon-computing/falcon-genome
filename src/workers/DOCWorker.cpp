#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/DOCWorker.h"

namespace fcsgenome {

DOCWorker::DOCWorker(std::string ref_path,
      std::string input_path,
      std::string output_path,
      std::string geneList,
      std::string intervalList,
      int depthCutoff,
      bool &flag_f,
      bool &flag_baseCoverage,
      bool &flag_intervalCoverage,
      bool &flag_sampleSummary) : Worker(1, 4),
  ref_path_(ref_path),
  input_path_(input_path),
  geneList_(geneList),
  intervalList_(intervalList),
  depthCutoff_(depthCutoff),
  flag_baseCoverage_(flag_baseCoverage),
  flag_intervalCoverage_(flag_intervalCoverage),
  flag_sampleSummary_(flag_sampleSummary)
{
  // check input/output files
  output_path_ = check_output(output_path, flag_f);
}

void DOCWorker::check() {
  ref_path_   = check_input(ref_path_);
  input_path_ = check_input(input_path_);
  if(geneList_ != "")
  	geneList_   = check_input(geneList_);
  if(intervalList_ != "")
  	intervalList_ = check_input(intervalList_);
}

void DOCWorker::setup() {

  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx8g "
      << "-jar " << get_config<std::string>("gatk_path") << " "
      << "-T DepthOfCoverage "
      << "-R " << ref_path_ << " "
      << "-I " << input_path_ << " "
      << "-nt 24 "
      << "-ct 9 "
      << "-o " << output_path_ << " ";
  if(geneList_ != "")
  	cmd << "-genelist " << geneList_ << " ";
  if(intervalList_ != "")
  	cmd << "-L " << intervalList_ << " ";

  if(!flag_baseCoverage_)
  	cmd << "-omitBaseOutput "; 
  if(!flag_intervalCoverage_)
  	cmd << "-omitIntervals ";
  if(!flag_sampleSummary_)
  	cmd << "-omitSampleSummary ";


  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

} // namespace fcsgenome
