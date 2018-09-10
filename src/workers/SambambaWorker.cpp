#include <regex>
#include <string>
#include <sys/resource.h>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/SambambaWorker.h"

namespace fcsgenome{

SambambaWorker::SambambaWorker(std::string input_path,
    std::string output_path, std::string action, 
    bool &flag_f): Worker(1, get_config<int>("markdup.nt"))
{
  // check output
  output_file_ = check_output(output_path, flag_f, true);
  std::string bai_file = check_output(output_path + ".bai", flag_f, true);
  input_path_ = input_path;
  action_ = action;
}

void SambambaWorker::check() {
  input_path_ = check_input(input_path_);
  get_input_list(input_path_, input_files_, ".*/part-[0-9].*", true);
}

void SambambaWorker::setup() {
  // update limit
  struct rlimit file_limit;
  file_limit.rlim_cur = get_config<int>("sambamba.max_files");
  file_limit.rlim_max = get_config<int>("sambamba.max_files");

  if (setrlimit(RLIMIT_NOFILE, &file_limit) != 0) {
    throw internalError("Failed to update limit");
  }
  getrlimit(RLIMIT_NOFILE, &file_limit);
  if (file_limit.rlim_cur != get_config<int>("sambamba.max_files") ||
      file_limit.rlim_max != get_config<int>("sambamba.max_files")) {
    throw internalError("Failed to update limit");
  }

  // create cmd
  std::stringstream cmd;

  if (action_ == "markdup") {
    cmd << get_config<std::string>("sambamba_path") << " markdup ";
  }
   
  if (action_ == "merge") {
    cmd << get_config<std::string>("sambamba_path") << " merge "  << output_file_ << " ";
  }

  LOG(INFO) << input_files_.size();
  for (int i = 0; i < input_files_.size(); i++) {
    if (boost::filesystem::extension(input_files_[i]) == ".bam" || boost::filesystem::extension(input_files_[i]) == ""){
        cmd << input_files_[i] << " ";
    }
  }

  cmd << "-l 1 " << "-t " << get_config<int>("sambamba.nt") << " ";

  if (action_ == "markdup") {
    cmd << "--overflow-list-size=" << get_config<int>("sambamba.overflow-list-size") << " "
        << "--tmpdir=" << get_config<std::string>("temp_dir") << " " << output_file_;
  }

  LOG(INFO) << "Reach here " ;

  cmd_ = cmd.str();
  if (action_ == "merge"  && input_files_.size() == 1) {
     cmd_ = "mv " + input_files_[0] + " " + output_file_;
  }

  LOG(INFO) << cmd_;
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome
