#include <string>
#include <sstream>
#include <sys/resource.h>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/MergeBamWorker.h"

namespace fcsgenome{

MergeBamWorker::MergeBamWorker(std::string inputPartsBAM, std::string outputBAM, int check_parts,
    bool &flag_f): Worker(1, get_config<int>("mergebam.nt"))
{
  // check output
  output_file_   = check_output(outputBAM, flag_f, true);
  inputPartsBAM_ = inputPartsBAM;
  check_parts_ = check_parts;
}

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

  std::stringstream cmd;
  // Create Command if check_parts==1:
  if (check_parts_ == 1){
      cmd << get_config<std::string>("sambamba_path") << " merge "
          << "-l 1 "
          << "-t " << get_config<int>("mergebam.nt") << " " << output_file_ << " " << inputPartsBAM_;
      cmd_ = cmd.str();
  } else {
      cmd << "mv " << inputPartsBAM_ << " " << output_file_ << " ; "
          << get_config<std::string>("sambamba_path") << " index " << "-t "
          << get_config<int>("mergebam.nt") << " " << output_file_ ;
      cmd_ = cmd.str();
  }

  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome
