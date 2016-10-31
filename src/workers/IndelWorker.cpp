#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/IndelWorker.h"

namespace fcsgenome {

RTCWorker::RTCWorker(std::string ref_path,
      std::vector<std::string> &known_indels,
      std::string input_path,
      std::string output_path):
  Worker(1, get_config<int>("gatk.rtc.nt")),
  ref_path_(ref_path),
  known_indels_(known_indels),
  input_path_(input_path)
{
  bool flag = true;
  output_path_ = check_output(output_path, flag);
}

void RTCWorker::check() {
  ref_path_ = check_input(ref_path_);
  input_path_ = check_input(input_path_);
  if (boost::filesystem::is_directory(input_path_)) {
    get_input_list(input_path_, input_files_, ".*/part-[0-9].bam");
  }
  else {
    input_files_.push_back(input_path_);
  }

  for (int i = 0; i < known_indels_.size(); i++) {
    known_indels_[i] = check_input(known_indels_[i]);
  }
}

void RTCWorker::setup() {
  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx" << get_config<int>("gatk.rtc.memory") << "g "
      << "-jar " << get_config<std::string>("gatk_path") << " "
      << "-T RealignerTargetCreator "
      << "-R " << ref_path_ << " "
      << "-nt " << get_config<int>("gatk.rtc.nt") << " "
      << "-o " << output_path_ << " ";

  for (int i = 0; i < known_indels_.size(); i++) {
    cmd << "-known " << known_indels_[i] << " ";
  }

  for (int i = 0; i < input_files_.size(); i++) {
    cmd << "-I " << input_files_[i] << " ";
  }
  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

IndelWorker::IndelWorker(std::string ref_path,
      std::vector<std::string> &known_indels,
      std::string intv_path,
      std::string input_path,
      std::string target_path,
      std::string output_path,
      bool &flag_f): Worker(1, 1),
  ref_path_(ref_path),
  known_indels_(known_indels),
  intv_path_(intv_path),
  input_path_(input_path),
  target_path_(target_path)
{
  output_path_ = check_output(output_path, flag_f);
}

void IndelWorker::check() {
  ref_path_    = check_input(ref_path_);
  intv_path_   = check_input(intv_path_);
  target_path_ = check_input(target_path_);

  input_path_  = check_input(input_path_);
  //if (boost::filesystem::is_directory(input_path_)) {
  //  get_input_list(input_path_, input_files_, ".*/part-[0-9].bam");
  //}
  //else {
  //  input_files_.push_back(input_path_);
  //}
  for (int i = 0; i < known_indels_.size(); i++) {
    known_indels_[i] = check_input(known_indels_[i]);
  }
}

void IndelWorker::setup() {
  
  std::stringstream cmd;
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx" << get_config<int>("gatk.indel.memory") << "g "
      << "-jar " << get_config<std::string>("gatk_path") << " "
      << "-T IndelRealigner "
      << "-R " << ref_path_ << " "
      << "-L " << intv_path_ << " "
      << "-targetIntervals " << target_path_ << " "
      << "-I " << input_path_ << " "
      << "-o " << output_path_ << " ";

  for (int i = 0; i < known_indels_.size(); i++) {
    cmd << "-known " << known_indels_[i] << " ";
  }

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome

