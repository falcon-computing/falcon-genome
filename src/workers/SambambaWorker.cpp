#include <regex>
#include <string>
#include <sys/resource.h>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/SambambaWorker.h"

namespace fcsgenome{

static inline std::string get_task_name(SambambaWorker::Action action) {
  switch (action) {
    case SambambaWorker::MARKDUP:
      return "Mark Duplicates";
    case SambambaWorker::MERGE :
      return "Merge BAM";
    case SambambaWorker::INDEX :
      return "Index BAM";
    case SambambaWorker::SORT :
      return "Sorting BAM";
    default:
      return "";
  }
}

SambambaWorker::SambambaWorker(std::string input_path,
       std::string output_path,  
       Action action,
       std::string common,
       bool &flag_f,
       std::vector<std::string> files
       ): 
       Worker(1, get_config<int>("markdup.nt"), 
              std::vector<std::string>(), get_task_name(action)), 
       flag_f_(flag_f), input_files_(files), output_file_(output_path),
       input_path_(input_path), action_(action), common_(common)
{
  ;
}

void SambambaWorker::check() {
  if (input_files_.size() == 0) {
    input_path_ = check_input(input_path_);
  }
  if (boost::filesystem::exists(input_path_)) { // when input_files_ is not empty,
                                                // we don't want get_input_list() error out
                                                // because input_path_ does not exist.
    get_input_list(input_path_, input_files_, common_, true);
  }
  for (auto & it : input_files_) {
    it = check_input(it);
  }
  if (output_file_.length() != 0) {
    output_file_ = check_output(output_file_, flag_f_, true);
    std::string bai_file = check_output(output_file_ + ".bai", flag_f_, true);
  }
}

void SambambaWorker::setup() {
  // update limit
  struct rlimit file_limit;
  file_limit.rlim_cur = get_config<int>("markdup.max_files");
  file_limit.rlim_max = get_config<int>("markdup.max_files");

  if (setrlimit(RLIMIT_NOFILE, &file_limit) != 0) {
    throw internalError("Failed to update limit");
  }
  getrlimit(RLIMIT_NOFILE, &file_limit);
  if (file_limit.rlim_cur != get_config<int>("markdup.max_files") ||
      file_limit.rlim_max != get_config<int>("markdup.max_files")) {
    throw internalError("Failed to update limit");
  }

  // create cmd:
  std::stringstream inputBAMs;
  for (int i = 0; i < input_files_.size(); i++) {
    if (boost::filesystem::extension(input_files_[i]) == ".bam" 
        || boost::filesystem::extension(input_files_[i]) == ""){
      inputBAMs << input_files_[i] << " ";
    }
  }

  std::stringstream cmd;
  switch (action_) {
  case MARKDUP:
    cmd << get_config<std::string>("sambamba_path") << " markdup "
        << "--overflow-list-size=" << get_config<int>("markdup.overflow-list-size") << " " 
        << inputBAMs.str() << " "
        << "--tmpdir=" << get_config<std::string>("temp_dir") << " " 
        << output_file_ << " "
        << "-l 1 " << "-t " << get_config<int>("markdup.nt") << " ";
    break;
  case MERGE:
    cmd << get_config<std::string>("sambamba_path") << " merge "  
        << output_file_ << " "
        << inputBAMs.str() << " "
        << "-l 1 " << "-t " << get_config<int>("mergebam.nt") << " ;";
    // move corresponding bed files if exist
    { // giving this '{}' space to make "bed_path" only locally available
      std::string bed_path;
      while (inputBAMs >> bed_path) {
        if (boost::filesystem::exists(get_fname_by_ext(bed_path, "bed"))) {
          cmd << "mv " << get_fname_by_ext(bed_path, "bed") 
              << " " << get_fname_by_ext(output_file_, "bed") << ";";
        }
      }
    }
    break;
  case INDEX:
    cmd << get_config<std::string>("sambamba_path") << " index " 
        << input_path_ << " " 
        << "-t " << get_config<int>("mergebam.nt") << " ";
    break;
  case SORT:
    cmd << get_config<std::string>("sambamba_path") << " sort " 
        << "--tmpdir=" << get_config<std::string>("temp_dir") << " "
        << "-t 1" << " " << "-l 1" << " "
        << input_path_ << ";";
    // mv to the output_file_ path if it is not empty
    {    
      // mv bam and bai
      std::string mv_output_path = input_path_;
      if (output_file_ != "") {
        mv_output_path = output_file_;
      }  
      cmd << "mv " << get_fname_by_ext(input_path_, "sorted.bam") 
          << " " << mv_output_path << ";";
      cmd << "mv " << get_fname_by_ext(input_path_, "sorted.bam.bai") 
          << " " << get_fname_by_ext(mv_output_path, "bai") << ";";
      // mv corresponding bed file if exists
      if (boost::filesystem::exists(get_fname_by_ext(input_path_, "bed"))
	  && !boost::filesystem::exists(get_fname_by_ext(output_file_, "bed"))) {
        cmd << "mv " << get_fname_by_ext(input_path_, "bed")
            << " " << get_fname_by_ext(mv_output_path, "bed");
      }
    }
    break;
  default:
    throw internalError("Invalid action");
  }

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome
