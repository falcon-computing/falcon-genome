#include <boost/filesystem.hpp>
#include <string>
#include <sys/statvfs.h>
#include <bits/stdc++.h> 
#include <unistd.h>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/Minimap2Worker.h"

namespace fs = boost::filesystem;

namespace fcsgenome {

Minimap2Worker::Minimap2Worker(std::string ref_path,
      std::string fq1_path,
      std::string fq2_path,
      std::string partdir_path,
      std::string output_path,
      int         num_buckets,
      std::vector<std::string> extra_opts,
      std::string sample_id,
      std::string read_group,
      std::string platform_id,
      std::string library_id,
      bool flag_merge_bams,
      bool &flag_f):
  Worker(1, 1, extra_opts, "minimap-flow"),
  ref_path_(ref_path),
  fq1_path_(fq1_path),
  fq2_path_(fq2_path),
  partdir_path_(partdir_path),
  output_path_(output_path),
  num_buckets_(num_buckets),
  sample_id_(sample_id),
  read_group_(read_group),
  platform_id_(platform_id),
  library_id_(library_id),
  flag_merge_bams_(flag_merge_bams)
{
  //partdir_path_ = check_output(partdir_path);

  if (sample_id.empty() ||
      read_group.empty() || 
      platform_id.empty() ||
      library_id.empty()) {
    throw invalidParam("Invalid @RG info");
  }
}

void Minimap2Worker::check() {
  // check reference files
  std::string mmi_path = get_fname_by_ext(ref_path_, "mmi");
  if (boost::filesystem::exists(mmi_path)) {
    ref_path_ = check_input(mmi_path);
  }
  else {
    ref_path_ = check_input(ref_path_);
  }

  // check input files
  fq1_path_ = check_input(fq1_path_);
  fq2_path_ = check_input(fq2_path_);

  // Checking if Temporal Storage fits with input:                                                                                                                   

  std::string temp_dir = partdir_path_;

  uintmax_t fastq_size = 0;
  uintmax_t mult=3;
  if (fs::exists(fq1_path_) && fs::exists(fq2_path_)) {
    fastq_size = mult*(fs::file_size(fq1_path_)+fs::file_size(fq2_path_));
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

void Minimap2Worker::setup() {
  // create cmd
  std::stringstream cmd;
  cmd << "LD_LIBRARY_PATH=" << conf_root_dir << "/lib:$LD_LIBRARY_PATH ";
  cmd << get_config<std::string>("minimap_path") << " "
      << "-R \"@RG\\tID:" << read_group_ << 
             "\\tSM:" << sample_id_ << 
             "\\tPL:" << platform_id_ << 
             "\\tLB:" << library_id_ << "\" "
      << "--temp_dir=\"" << partdir_path_ << "\" "
      << "--output=\"" << output_path_ << "\" "
      //<< "--num_buckets=" << num_buckets_ << " "
      << "--num_buckets=" << get_config<int>("minimap.num_buckets")  << " "

      << "--merge_bams=" << flag_merge_bams_ << " ";

  if (get_config<int>("minimap.nt") > 0) {
    cmd << "--t=" << get_config<int>("minimap.nt") << " ";
  }

  if (get_config<bool>("minimap.enforce_order")) {
    cmd << "--inorder_output ";
  }

  //if (get_config<bool>("minimap.use_fpga") && !get_config<std::string>("minimap.fpga.bit_path").empty()) {
  //  cmd << "--use_fpga "
  //      << "--fpga_path=" << get_config<std::string>("minimap.fpga.bit_path") << " ";
  //}

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
  LOG(INFO) << cmd_ << "\n";

}
} // namespace fcsgenome
