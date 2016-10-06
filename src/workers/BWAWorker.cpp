#include <string>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/BWAWorker.h"

namespace fcsgenome {

BWAWorker::BWAWorker(std::string ref_path,
      std::string fq1_path,
      std::string fq2_path,
      std::string output_path,
      std::string sample_id,
      std::string read_group,
      std::string platform_id,
      std::string library_id,
      bool &flag_f): Worker(1)
{
  // check input files
  ref_path    = check_input(ref_path);
  fq1_path    = check_input(fq1_path);
  fq2_path    = check_input(fq2_path);
  output_path = check_output(output_path, flag_f);

  DLOG(INFO) << "ref_genome is " << ref_path;
  DLOG(INFO) << "fastq1 is " << fq1_path;
  DLOG(INFO) << "fastq2 is " << fq2_path;
  DLOG(INFO) << "output is " << output_path;

  if (sample_id.empty() ||
      read_group.empty() || 
      platform_id.empty() ||
      library_id.empty()) {
    throw invalidParam("Invalid @RG info");
  }
  
  // create cmd
  std::stringstream cmd;
  cmd << "LD_LIBRARY_PATH=" << conf_root_dir << "/lib:$LD_LIBRARY_PATH "
      << get_config<std::string>("bwa_path") << " mem -M "
      << "-R \"@RG\\tID:" << read_group << 
                 "\\tSM:" << sample_id << 
                 "\\tPL:" << platform_id << 
                 "\\tLB:" << library_id << "\" "
      << "--logtostderr "
      << "--v=1 "
      << "--offload "
      << "--sort "
      << "--output_flag=1 "
      << "--output_dir=\"" << output_path << "\" "
      << "--max_num_records=" << get_config<int>("bwa.max_records") << " "
      << ref_path << " "
      << fq1_path << " "
      << fq2_path;

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome
