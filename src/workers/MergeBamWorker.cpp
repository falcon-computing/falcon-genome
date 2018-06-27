#include <string>
#include <sstream>
#include <sys/resource.h>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/MarkdupWorker.h"

namespace fcsgenome{

MergeBamWorker::MergeBamWorker(std::stringstream inputPartsBAM,
    std::string outputBAM,
    bool &flag_f): Worker(1, get_config<int>("mergebam.nt"))
{
  // check output
  output_file_ = check_output(outputBAM, flag_f, true);
  inputPartsBAM_ = inputPartsBAM;
}

//void MergeBamWorker::check() {
//  input_path_ = check_input(input_path_);
//  get_input_list(input_path_, input_files_, ".*/part-[0-9].*", true);
//}

void MergeBamWorker::setup() {
  // update limit
  struct rlimit file_limit;
  file_limit.rlim_cur = get_config<int>("mergebam.max_files");
  file_limit.rlim_max = get_config<int>("mergebam.max_files");

  if (setrlimit(RLIMIT_NOFILE, &file_limit) != 0) {
    throw internalError("Failed to update limit");
  }
  getrlimit(RLIMIT_NOFILE, &file_limit);
  if (file_limit.rlim_cur != get_config<int>("mergebam.max_files") ||
      file_limit.rlim_max != get_config<int>("mergebam.max_files")) {
    throw internalError("Failed to update limit");
  }

  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("sambamba_path") << " merge "
      << "-l 1 "
      << "-t " << get_config<int>("mergebam.nt") << " " << outputBAM << " " << inputPartsBAM_;
      ;
  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome
