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
    default:
      return "";
  }
}

SambambaWorker::SambambaWorker(std::string input_path,
       std::string output_path,  
       Action action, 
       bool flag_merge_bam,
       bool &flag_f): Worker(1, get_config<int>("markdup.nt"), std::vector<std::string>(), get_task_name(action))
{
  // check output
  output_file_ = check_output(output_path, flag_f, true);
  std::string bai_file = check_output(output_path + ".bai", flag_f, true);
  input_path_ = input_path;
  action_ = action;
  flag_merge_bam_ = flag_merge_bam;

  LOG(INFO) << output_file_;

}

void SambambaWorker::check() {
  input_path_ = check_input(input_path_);
  get_input_list(input_path_, input_files_, ".*/part*-[0-9].*.*", true);
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

  // Fot the new bwa-flow, parts BAM need to be sorted:
  std::stringstream cmd;
  std::string sambamba_sort(get_config<std::string>("sambamba_path") +  " sort ");

  std::stringstream inputBAMs;
  for (int i = 0; i < input_files_.size(); i++) {
    if (boost::filesystem::extension(input_files_[i]) == ".bam"){
      cmd << sambamba_sort << input_files_[i] << " -o " << input_files_[i] << ".temp  -t 1 ; mv " << input_files_[i] << ".temp " << input_files_[i] << ";";        
      inputBAMs << input_files_[i] << " ";
    }
  }

  std::stringstream cmd2;
  switch (action_) {
  case MARKDUP: 
    cmd << get_config<std::string>("sambamba_path") << " markdup "
        << "--overflow-list-size=" << get_config<int>("markdup.overflow-list-size") << " " 
        << inputBAMs.str() << " "
        << "--tmpdir=" << get_config<std::string>("temp_dir") << " " << output_file_ << " "
        << "-l 1 " << "-t " << get_config<int>("markdup.nt") << " ";
    break;
  case MERGE:
    if (input_files_.size() > 1) {
      if (!flag_merge_bam_){
        cmd << get_config<std::string>("sambamba_path") << " merge "  << output_file_ << " "
            << inputBAMs.str() <<    " "
            << "-l 1 " << "-t " << get_config<int>("mergebam.nt") << " ";
      }
      else {
        cmd << "mv " << input_path_ << " " << output_file_;
      }
    }
    else { 
      cmd << "mv " << inputBAMs.str() <<  " " <<  output_file_ << "; " 
          << get_config<std::string>("samtools_path") << " index " << output_file_ ;
    }
    break;
  default:
    throw internalError("Invalid action");
  }

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome
