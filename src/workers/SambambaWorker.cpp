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
       bool &flag_f): Worker(1, get_config<int>("markdup.nt"), std::vector<std::string>(), get_task_name(action))
{
  // check output
  output_file_ = check_output(output_path, flag_f, true);
  std::string bai_file = check_output(output_path + ".bai", flag_f, true);
  input_path_ = input_path;
  action_ = action;
  common_ = common;
}

void SambambaWorker::check() {
  input_path_ = check_input(input_path_);
  get_input_list(input_path_, input_files_, common_, true);
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
    if (boost::filesystem::extension(input_files_[i]) == ".bam" || boost::filesystem::extension(input_files_[i]) == ""){
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
        << inputBAMs.str() <<    " "
        << "-l 1 " << "-t " << get_config<int>("mergebam.nt") << " ";    
    break;
  case INDEX:
    cmd << get_config<std::string>("sambamba_path") << " index " 
        << output_file_ << " " 
        << "-t " << get_config<int>("mergebam.nt") << " ";
    break;
  case SORT:
    cmd << get_config<std::string>("sambamba_path") << " sort " 
        << "--tmpdir=" << get_config<std::string>("temp_dir") << " " 
        << input_path_ << ";";
    // mv bam and bai
    cmd << "mv " << get_fname_by_ext(input_path_, "sorted.bam") 
        << " " << input_path_ << ";";
    cmd << "mv " << get_fname_by_ext(input_path_, "sorted.bam.bai") 
        << " " << get_fname_by_ext(input_path_, "bai");
    break;
  default:
    throw internalError("Invalid action");
  }

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome
