#include <string>
#include <sys/resource.h>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/MarkdupWorker.h"

namespace fcsgenome{

MarkdupWorker::MarkdupWorker(std::string input_path,
    std::string output_path,
    bool &flag_f): Worker(1, get_config<int>("markdup.nt"))
{
  // check output
  output_file_ = check_output(output_path, flag_f, true);
  output_file_ + ".bai" = check_output(output_path + ".bai", flag_f, true);
  input_path_ = input_path;
}

void MarkdupWorker::check() {
  input_path_ = check_input(input_path_);
  get_input_list(input_path_, input_files_, ".*/part-[0-9].*", true);
}

void MarkdupWorker::setup() {
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

  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("sambamba_path") << " markdup "
      << "-l 1 "
      << "-t " << get_config<int>("markdup.nt") << " "
      << "--tmpdir=" << get_config<std::string>("temp_dir") << " "
      << "--overflow-list-size=" << get_config<int>("markdup.overflow-list-size") << " ";
  for (int i = 0; i < input_files_.size(); i++) {
    cmd << input_files_[i] << " ";
  }
  cmd << output_file_;
  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome
