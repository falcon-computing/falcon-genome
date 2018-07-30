#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/SplitVCFbyIntervalsWorker.h"

namespace fcsgenome {

SplitVCFbyIntervalsWorker::SplitVCFbyIntervalsWorker(
      std::string inputVCF,
      std::vector<std::string> intervalSet,
      std::vector<std::string> &vcfSets,
      std::string commonString, bool &flag_f): Worker(1, 1) {
      inputVCF_(inputVCF),
      intervalSet_(intervalSet),
      commonString_(commonString)
{
  // check output files
  vcfSets_ = check_output(vcfSets, flag_f);
}

void SplitVCFByIntervalsWorker::check() {
    inputVCF_ = check_input(inputVCF_);
}

void SplitVCFByIntervalWorker::setup() {
  // create cmd
  std::stringstream cmd;
  for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++){
    cmd << get_config<std::string>("bcftools_path") << " filter "
        << "-T " << intervalSet_[i] << " -Oz  -o " << vcfSets[i]
        << " " << inputVCF_ << "; " << get_config<std::string>("tabix_path")
        << " " <<  vcfSets[i] ;
    }
    cmd_ = cmd.str();
    DLOG(INFO) << cmd_;
  }
} // namespace fcsgenome
