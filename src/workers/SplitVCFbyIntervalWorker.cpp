#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/SplitVCFbyIntervalsWorker.h"

namespace fcsgenome {

SplitVCFbyIntervalsWorker::SplitVCFbyIntervalsWorker(
      std::vector<std::string> inputVCF,
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
  for (int i = 0; i < inputVCF_.size(); i++) {
    inputVCF_[i] = check_input(inputVCF_[i]);
  }
}

void SplitVCFByIntervalWorker::setup() {
  // create cmd
  std::stringstream cmd;
  for (){
  cmd << get_config<std::string>("bcftools_path") << " filter "
      << "-T " << intervalSet_[i] << " -Oz  -o " << vcfSets[i]
      << " " << inputVCF_ << "; " << get_config<std::string>("tabix_path")
      << " " <<  vcfSets[i] ;
  }


  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
/usr/local/falcon/tools/bin/bcftools filter -T intervals/list0.bed  -Oz  -o test.vcf.gz   ../ref/b37_cosmic_v54_120711.vcf ; /usr/local/falcon/tools/bin/tabix  test.vcf.gz


ZIPWorker::ZIPWorker(
      std::string input_path,
      std::string output_path,
      bool &flag_f): Worker(1, 1)
{
  input_file_  = input_path;
  output_file_ = check_output(output_path, flag_f);
}

void ZIPWorker::check() {
  DLOG(INFO) << "ZIPWorker check " << input_file_;
  input_file_ = check_input(input_file_);
}

void ZIPWorker::setup() {
  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("bcftools_path") << " norm "
      << "-m +any "
      << "-O z "
      << "-o " << output_file_ << " "
      << input_file_;

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

void TabixWorker::check() {
  path_ = check_input(path_);
}

void TabixWorker::setup() {
  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("tabix_path") << " -p vcf "
      << path_;
  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

void VCFSortWorker::check() {
  path_ = check_input(path_);
}

void VCFSortWorker::setup() {
  std::stringstream cmd;
  cmd << "("
      << "grep -v \"^[^#;]\" " << path_ << " && "
      << "grep \"^[^#;]\" " << path_ << " | sort -V"
      << ") > " << path_ << ".sort";
  cmd << ";";
  cmd << "mv " << path_ << ".sort " << path_;

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome
