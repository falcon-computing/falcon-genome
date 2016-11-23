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
      bool &flag_f):
  Worker(get_config<bool>("latency_mode") ?  conf_host_list.size() : 1),
  ref_path_(ref_path),
  fq1_path_(fq1_path),
  fq2_path_(fq2_path),
  sample_id_(sample_id),
  read_group_(read_group),
  platform_id_(platform_id),
  library_id_(library_id)
{
  output_path_ = check_output(output_path, flag_f);

  if (sample_id.empty() ||
      read_group.empty() || 
      platform_id.empty() ||
      library_id.empty()) {
    throw invalidParam("Invalid @RG info");
  }
}

void BWAWorker::check() {

  // check input files
  ref_path_ = check_input(ref_path_);
  fq1_path_ = check_input(fq1_path_);
  fq2_path_ = check_input(fq2_path_);
}

void BWAWorker::setup() {
  // create cmd
  std::stringstream cmd;
  if (!get_config<bool>("latency_mode")) {
    cmd << "LD_LIBRARY_PATH=" << conf_root_dir << "/lib:$LD_LIBRARY_PATH ";
  }
  else {
    // TODO: here needs to make sure necessary LD_LIBRARY_PATH
    // is set in user's bash mode
    cmd << get_config<std::string>("mpi_path") << "/bin/mpirun " 
        << "--prefix " << get_config<std::string>("mpi_path") << " "
        << "--bind-to none "
        << "-np " << conf_host_list.size() << " ";

    // set host list
    cmd << "--host ";
    for (int i = 0; i < conf_host_list.size(); i++) {
      cmd << conf_host_list[i];
      if (i < conf_host_list.size() - 1) {
        cmd << ",";
      }
      else {
        cmd << " ";
      }
    }
  }
  cmd << get_config<std::string>("bwa_path") << " mem -M "
      << "-R \"@RG\\tID:" << read_group_ << 
                 "\\tSM:" << sample_id_ << 
                 "\\tPL:" << platform_id_ << 
                 "\\tLB:" << library_id_ << "\" "
      << "--logtostderr "
      << "--offload "
      << "--sort "
      << "--output_flag=1 "
      << "--v=0 "
      << "--output_dir=\"" << output_path_ << "\" "
      << "--max_num_records=" << get_config<int>("bwa.max_records") << " ";
  if (get_config<bool>("bwa.use_fpga") &&
      !get_config<std::string>("bwa.fpga_path").empty()) {
    cmd << "--use_fpga "
        << "--max_fpga_thread=3 "
        << "--chunk_size=10000 "
        << "--fpga_path=" << get_config<std::string>("bwa.fpga_path") << " ";
  }
  cmd << ref_path_ << " "
      << fq1_path_ << " "
      << fq2_path_;

  cmd_ = cmd.str();
}
} // namespace fcsgenome
