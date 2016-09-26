#include <glog/logging.h>
#include <string>
#include <sys/resource.h>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/MarkdupWorker.h"

namespace fcsgenome{

MarkdupWorker::MarkdupWorker(std::string input_path,
    std::string output_path,
    bool &flag_f): Worker(1, conf_markdup_nthreads)
{
  // check output
  output_file_ = check_output(output_path, flag_f, true);
  input_path_ = input_path;
}

void MarkdupWorker::check() {
  input_path_ = check_input(input_path_);
  get_input_list(input_path_, input_files_, ".*/part-[0-9].*");
}

void MarkdupWorker::setup() {
  // update limit
  struct rlimit file_limit;
  file_limit.rlim_cur = conf_markdup_maxfile;
  file_limit.rlim_max = conf_markdup_maxfile;

  if (setrlimit(RLIMIT_NOFILE, &file_limit) != 0) {
    throw internalError("Failed to update limit");
  }
  getrlimit(RLIMIT_NOFILE, &file_limit);
  if (file_limit.rlim_cur != conf_markdup_maxfile || 
      file_limit.rlim_max != conf_markdup_maxfile) {
    throw internalError("Failed to update limit");
  }

  // create cmd
  std::stringstream cmd;
  cmd << conf_sambamba_call << " markdup "
      << "-l 1 "
      << "-t " << conf_markdup_nthreads << " "
      << "--tmpdir=" << conf_temp_dir << " "
      << "--overflow-list-size=" << conf_markdup_overflowsize << " ";
  for (int i = 0; i < input_files_.size(); i++) {
    cmd << input_files_[i] << " ";
  }
  cmd << output_file_;
  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome
