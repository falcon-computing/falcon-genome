#include <iostream>
#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/BQSRWorker.h"

namespace fcsgenome {

BQSRWorker::BQSRWorker(std::string ref_path,
      std::vector<std::string> &known_sites,
      std::string intv_path,
      std::string input_path,
      std::string output_path,
      std::vector<std::string> extra_opts,
      std::vector<std::string> &intv_list,
      int  contig,
      bool &flag_f,
      bool flag_gatk):
  Worker(1,
         get_config<int>("gatk.bqsr.nct", "gatk.nct"),
         extra_opts),
  ref_path_(ref_path),
  intv_path_(intv_path),
  input_path_(input_path),
  known_sites_(known_sites),
  intv_list_(intv_list),
  flag_gatk_(flag_gatk)
{
  output_path_ = check_output(output_path, flag_f);
}

void BQSRWorker::check() {
  ref_path_   = check_input(ref_path_);
  intv_path_  = check_input(intv_path_);
  input_path_ = check_input(input_path_);

  namespace fs = boost::filesystem;
  for (int i = 0; i < known_sites_.size(); i++) {
    known_sites_[i] = check_input(known_sites_[i]);
    std::string site = known_sites_[i];
    std::string ext  = fs::extension(site);
    std::string idx_ext;

    // check for '.idx' suffix
    if (ext == ".vcf") {
      idx_ext = ".idx";
    }
    else if (ext == ".gz") {
      idx_ext = ".tbi";
    }
    else { // unrecognized extension
      LOG(ERROR) << "Unrecognized extension for known site: " << site;
      throw silentExit();
    }

    auto vcf_file = fs::path(site);
    auto idx_file = fs::path(site + idx_ext);

    if (std::difftime(
          fs::last_write_time(vcf_file),
          fs::last_write_time(idx_file)) > 0) {
      // if idx is older than vcf
      //LOG(WARNING) << idx_file << " is older than "
      //  << vcf_file << ", which will cause slowdown in GATK";

      boost::system::error_code err;

      fs::last_write_time(idx_file, std::time(NULL), err);
      if (err != 0) {
        LOG(ERROR) << "Attempting to update the last modified time for " << idx_file
                   << ", but failed.";
        LOG(ERROR) << "Please fix it manually before running this command again, "
                   << "otherwise GATK will be slowed down significantly.";
        throw silentExit();
      }
      else {
        VLOG(1) << "Successfully updated stat for " << idx_file;
      }
    }
  }
}

void BQSRWorker::setup() {
  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx" << get_config<int>("gatk.bqsr.memory", "gatk.memory") << "g ";

  if (flag_gatk_ || get_config<bool>("use_gatk4")) {
      cmd << "-jar " << get_config<std::string>("gatk4_path") << " BaseRecalibrator ";
  } else {
      cmd << "-jar " << get_config<std::string>("gatk_path") << " -T BaseRecalibrator ";
  }

  cmd << "-R " << ref_path_ << " "
      << "-I " << input_path_ << " ";

  if (flag_gatk_ || get_config<bool>("use_gatk4")) {
      cmd << "-O " << output_path_  << " ";
  } else {
      cmd << "-nct " << get_config<int>("gatk.bqsr.nct", "gatk.nct") << " "
          << "--disable_auto_index_creation_and_locking_when_reading_rods "
          << "-o " << output_path_ << " ";
  }

  if (!intv_list_.empty()){
     for (int i = 0; i < intv_list_.size(); i++) {
        cmd << "-L " << intv_list_[i] << " ";
        LOG(INFO) << "command " << cmd << "\n";
     }
  } else {
     cmd << "-L " << intv_path_ << " ";
  }

  if (intv_list_.size() > 0 ) {
    cmd << "-isr INTERSECTION ";
  }

  for (int i = 0; i < known_sites_.size(); i++) {
    if (flag_gatk_ || get_config<bool>("use_gatk4")) {
        cmd << "-known-sites " << known_sites_[i] << " ";
    } else {
        cmd << "-knownSites " << known_sites_[i] << " ";
    }
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
  cmd << "1> /dev/null";

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

BQSRGatherWorker::BQSRGatherWorker(std::vector<std::string> &input_files,
    std::string output_file, bool &flag_f, bool flag_gatk): Worker(1, 1),
  input_files_(input_files),flag_gatk_(flag_gatk)
{
  output_file_ = check_output(output_file, flag_f);
}

void BQSRGatherWorker::check() {
  for (int i = 0; i < input_files_.size(); i++) {
    input_files_[i] = check_input(input_files_[i]);
  }
}

void BQSRGatherWorker::setup() {
  // create cmd
  std::stringstream cmd;
  if (flag_gatk_ || get_config<bool>("use_gatk4")){
      cmd << get_config<std::string>("java_path") << " "
          << "-Xmx" << get_config<int>("gatk.bqsr.memory", "gatk.memory") << "g "
          << "-jar " << get_config<std::string>("gatk4_path") << " "
          << "GatherBQSRReports ";
      for (int i = 0; i < input_files_.size(); i++) {
          cmd << "-I " << input_files_[i] << " ";
      }
      cmd << "-O " << output_file_;

  } else {
      cmd << get_config<std::string>("java_path") << " "
          << "-cp " << get_config<std::string>("gatk_path") << " "
          << "org.broadinstitute.gatk.tools.GatherBqsrReports ";
      for (int i = 0; i < input_files_.size(); i++) {
            cmd << "I=" << input_files_[i] << " ";
      }
      cmd << "O=" << output_file_;
  }
  cmd << " 1> /dev/null";

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

PRWorker::PRWorker(std::string ref_path,
      std::string intv_path,
      std::string bqsr_path,
      std::string input_path,
      std::string output_path,
      std::vector<std::string> extra_opts,
      std::vector<std::string> &intv_list,
      int  contig,
      bool &flag_f, bool flag_gatk):
  Worker(1, get_config<int>("gatk.pr.nct", "gatk.nct"), extra_opts),
  ref_path_(ref_path),
  intv_path_(intv_path),
  bqsr_path_(bqsr_path),
  input_path_(input_path),
  intv_list_(intv_list),
  flag_gatk_(flag_gatk)
{
  // check output files
  output_path_ = check_output(output_path, flag_f);
}

void PRWorker::check() {
  ref_path_    = check_input(ref_path_);
  intv_path_   = check_input(intv_path_);
  bqsr_path_   = check_input(bqsr_path_);
  input_path_  = check_input(input_path_);

  DLOG(INFO) << "intv is " << intv_path_;
  DLOG(INFO) << "output is " << output_path_;
}

void PRWorker::setup() {
  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx" << get_config<int>("gatk.pr.memory", "gatk.memory") << "g ";

  if (flag_gatk_ || get_config<bool>("use_gatk4")) {
      cmd << "-jar " << get_config<std::string>("gatk4_path") << " ApplyBQSR ";
  } else {
      cmd << "-jar " << get_config<std::string>("gatk_path") << " -T PrintReads ";
  }

  cmd << "-R " << ref_path_ << " "
      << "-I " << input_path_ << " ";

  if (flag_gatk_ || get_config<bool>("use_gatk4")) {
     cmd << "-O " << output_path_ << " --bqsr-recal-file " << bqsr_path_ << " ";
  } else {
     cmd << "-BQSR " << bqsr_path_ << " "
         << "-nct " << get_config<int>("gatk.pr.nct", "gatk.nct") << " "
         << "-o " << output_path_ << " ";
  }

  if (!intv_list_.empty()){
      for (int i = 0; i < intv_list_.size(); i++) {
           cmd << "-L " << intv_list_[i] << " ";
      }
  } else {
      cmd << "-L " << intv_path_ << " ";
  }

  if (intv_list_.size() > 0 ) {
    cmd << "-isr INTERSECTION ";
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
  cmd << " 1> /dev/null";

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome
