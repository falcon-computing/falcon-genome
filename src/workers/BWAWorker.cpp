#include <boost/filesystem.hpp>
#include <string>
#include <sys/statvfs.h>
#include <bits/stdc++.h> 
#include <unistd.h>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/BWAWorker.h"

namespace fs = boost::filesystem;

namespace fcsgenome {

BWAWorker::BWAWorker(std::string ref_path,
      std::string fq1_path,
      std::string fq2_path,
      std::string partdir_path,
      std::string output_path,
      std::vector<std::string> extra_opts,
      std::string sample_id,
      std::string read_group,
      std::string platform_id,
      std::string library_id,
      bool flag_align_only,
      bool flag_merge_bams,
      bool &flag_f):
  Worker(get_config<bool>("bwa.scaleout_mode") || 
         get_config<bool>("latency_mode") 
         ? conf_host_list.size() : 1, 
         1, extra_opts, "bwa mem"),
  ref_path_(ref_path),
  fq1_path_(fq1_path),
  fq2_path_(fq2_path),
  partdir_path_(partdir_path),
  output_path_(output_path),
  sample_id_(sample_id),
  read_group_(read_group),
  platform_id_(platform_id),
  library_id_(library_id),
  flag_align_only_(flag_align_only),
  flag_merge_bams_(flag_merge_bams),
  flag_f_(flag_f)
{
  ;
}

void BWAWorker::check() {

  // check partdir_path
  partdir_path_ = check_output(partdir_path_, flag_f_);

  // check output_path
  if (flag_merge_bams_) {
    check_output(output_path_, flag_f_);
  }

  // check the header args
  if (sample_id_.empty() ||
      read_group_.empty() || 
      platform_id_.empty() ||
      library_id_.empty()) {
    throw invalidParam("Invalid @RG info");
  }

  // check input files
  ref_path_ = check_input(ref_path_);
  fq1_path_ = check_input(fq1_path_);
  fq2_path_ = check_input(fq2_path_);

  // Checking if Temporal Storage fits with input:                                                                                                                   
  auto p = boost::filesystem::path(conf_temp_dir);
  std::string temp_dir = p.parent_path().string();

  uintmax_t fastq_size=0;
  uintmax_t mult=3;
  if (fs::exists(fq1_path_) && fs::exists(fq2_path_)){
    fastq_size=mult*(fs::file_size(fq1_path_)+fs::file_size(fq2_path_));
  }
  else{
    LOG(ERROR) << "FASTQ Files: " << fq1_path_ << " and " << fq2_path_ << " do not exist";
    throw silentExit();
  }

  fs::space_info si = fs::space(temp_dir);
  if (si.available < fastq_size){
    LOG(ERROR) << "Not enough space in temporary storage. "
	       << "The size of the temporary folder should be at least "
	       << mult << " times the size of input FASTQ files";
    throw silentExit();
  }
}

void BWAWorker::setup() {
  // create cmd
  std::stringstream cmd;
  if (get_config<bool>("bwa.scaleout_mode") ||
      get_config<bool>("latency_mode")) {
    // NOTE: Needs to make sure necessary LD_LIBRARY_PATH
    // is set in user's bash mode
    cmd << get_config<std::string>("mpi_path") << "/bin/mpirun " 
        << "--prefix " << get_config<std::string>("mpi_path") << " "
        << "--bind-to none ";
    if (check_config("bwa.mpi_if")) {
      cmd << "--mca oob_tcp_if_include " << get_config<std::string>("bwa.mpi_if") << " "
          << "--mca btl_tcp_if_include " << get_config<std::string>("bwa.mpi_if") << " ";
    }
        /* 
         * This '--mca' option is used to get rid of the problem of
         * hfi_wait_for_device which causes a 15 sec delay.
         * This problem could be related to boxes with Intel OmniPath HFI.
         * Source: 
         * http://users.open-mpi.narkive.com/7efFnJXR/ompi-users-device-failed-to-appear-connection-timed-out
         */
    cmd << "--mca pml ob1 "
        << "--allow-run-as-root "
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
  else {
    cmd << "LD_LIBRARY_PATH=" << conf_root_dir << "/lib:$LD_LIBRARY_PATH ";
  }
  cmd << get_config<std::string>("bwa_path") << " mem "
      << "-R \"@RG\\tID:" << read_group_ << 
                 "\\tSM:" << sample_id_ << 
                 "\\tPL:" << platform_id_ << 
                 "\\tLB:" << library_id_ << "\" "
      << "--logtostderr "
      << "--offload "
      << "--output_flag=1 "
      << "--chunk_size=2000 "
      << "--v=" << get_config<int>("bwa.verbose") << " "
      << "--temp_dir=\"" << partdir_path_ << "\" "
      << "--output=\"" << output_path_ << "\" " 
      << "--merge_bams=" << flag_merge_bams_ << " "
      << "--num_buckets=" << get_config<int>("bwa.num_buckets")  << " " ;

  if (get_config<int>("bwa.nt") > 0) {
    cmd << "--t=" << get_config<int>("bwa.nt") << " ";
  }

  if (flag_align_only_) {
    cmd << "--disable_markdup=true ";
  }

  if (get_config<bool>("bwa.enforce_order")) {
    cmd << "--inorder_output ";
  }

  if (get_config<bool>("bwa.use_fpga") &&
      !get_config<std::string>("bwa.fpga.bit_path").empty())
  {
    cmd << "--use_fpga "
        << "--fpga_path=" << get_config<std::string>("bwa.fpga.bit_path") << " ";
  }
  for (auto it = extra_opts_.begin(); it != extra_opts_.end(); it++) {
    cmd << it->first << " ";
    for( auto vec_iter = it->second.begin(); vec_iter != it->second.end(); vec_iter++) {
      if (!(*vec_iter).empty() && vec_iter == it->second.begin()) {
        cmd << *vec_iter << " ";
      }
      else if (!(*vec_iter).empty()) {
        cmd << it->first << " " << *vec_iter << " ";
      }
    }    
  }

  cmd << ref_path_ << " "
      << fq1_path_ << " "
      << fq2_path_;

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_ << "\n";

}
} // namespace fcsgenome
