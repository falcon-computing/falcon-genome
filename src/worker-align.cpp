#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <glog/logging.h>
#include <string>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"

namespace fcsgenome {

int bwa_command(
    std::string& ref_path,
    std::string& fq1_path,
    std::string& fq2_path,
    std::string& output_path,
    std::string& sample_id,
    std::string& read_group,
    std::string& platform_id,
    std::string& library_id) 
{
  create_dir(conf_log_dir);
  std::string log_fname = conf_log_dir + "/bwa-" + sample_id + ".log";
  DLOG(INFO) << "log dir is " << log_fname;

  std::stringstream cmd;
  cmd << "LD_LIBRARY_PATH=" << conf_root_dir << "/lib:$LD_LIBRARY_PATH "
      << conf_bwa_call << " mem -M "
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
      << "--max_num_records=" << conf_bwa_maxrecords << " "
      << ref_path << " "
      << fq1_path << " "
      << fq2_path << " &> " << log_fname;

  DLOG(INFO) << cmd.str();
  int ret = system(cmd.str().c_str());
  if (ret) {
    std::string msg = "BWAMEM failed, please check " + log_fname + " for details";
    throw failedCommand(msg);
  }

  return 0;
}

int align_main(int argc, char** argv) {

  namespace po = boost::program_options;
  std::string str_opt;

  // Define arguments
  po::options_description opt_desc("'fcs-genome align' options");
  po::variables_map cmd_vm;

  opt_desc.add_options() 
    arg_decl_string_w_def("ref,r", conf_default_ref, "reference genome path")
    arg_decl_string("fastq1,1", "input pair-end fastq file")
    arg_decl_string("fastq2,2", "input pair-end fastq file")
    arg_decl_string("output,o", "output file")
    arg_decl_string("rg,R", "read group id ('ID' in BAM headers)")
    arg_decl_string("sp,S", "sample id ('SM' in BAM headers)")
    arg_decl_string("pl,P", "platform id ('PL' in BAM headers)")
    arg_decl_string("lb,L", "library id ('LB' in BAM headers)")
    ("align-only", "skip duplicates marking and output "
                   "multiple BAM files in output_dir") 
    ("help,h", "print help messages")
    ("force,f", "overwrite output file if exists");

  // Parse arguments
  po::store(
      po::parse_command_line(argc, argv, opt_desc),
      cmd_vm);

  if (cmd_vm.count("help")) { 
    std::cerr << opt_desc << std::endl; 
    return 0; 
  } 

  // Check if required arguments are presented
  bool flag_f          = get_argument<bool>(cmd_vm, "force");
  bool flag_align_only = get_argument<bool>(cmd_vm, "align-only");

  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref");
  std::string fq1_path    = get_argument<std::string>(cmd_vm, "fastq1");
  std::string fq2_path    = get_argument<std::string>(cmd_vm, "fastq2");
  std::string output_path = get_argument<std::string>(cmd_vm, "output");
  std::string read_group  = get_argument<std::string>(cmd_vm, "rg");
  std::string sample_id   = get_argument<std::string>(cmd_vm, "sp");
  std::string platform_id = get_argument<std::string>(cmd_vm, "pl");
  std::string library_id  = get_argument<std::string>(cmd_vm, "lb");

  po::notify(cmd_vm);

  // Check input and output files
  ref_path    = check_input(ref_path);
  fq1_path    = check_input(fq1_path);
  fq2_path    = check_input(fq2_path);

  DLOG(INFO) << "ref_genome is " << ref_path;
  DLOG(INFO) << "fastq1 is " << fq1_path;
  DLOG(INFO) << "fastq2 is " << fq2_path;
  DLOG(INFO) << "output is " << output_path;

  std::string parts_dir;

  if (flag_align_only) {
    // Check if output in path already exists but is not a dir
    if (boost::filesystem::exists(output_path) &&
       !boost::filesystem::is_directory(output_path)) {
      throw fileNotFound("Output path '" +
                         output_path +
                         "' is not a directory");
    }
    parts_dir = output_path + "/" +
                sample_id + "/" +
                read_group;
    parts_dir = check_output(parts_dir, flag_f);
    create_dir(parts_dir);
  }
  else {
    output_path = check_output(output_path, flag_f);
    parts_dir = output_path + ".parts";

    // Remove parts_dir if it already exists
    remove_path(parts_dir);

    DLOG(INFO) << "Putting sorted BAM parts in '" << parts_dir << "'";
  }

  LOG(INFO) << "Start doing alignment";
  uint64_t start_ts = getTs();
  bwa_command(ref_path,
      fq1_path, fq2_path,
      parts_dir,
      read_group, sample_id,
      platform_id, library_id);
  log_time("BWA-MEM", start_ts);

  if (!flag_align_only) {
    std::string input_files = get_input_list(parts_dir, ".*/part-[0-9].*");

    LOG(INFO) << "Start marking duplicates";
    uint64_t start_ts = getTs();
    markdup_command(input_files, output_path);
    log_time("Mark duplicates", start_ts);

    // Remove parts_dir
    remove_path(parts_dir);
    DLOG(INFO) << "Removing temp file in '" << parts_dir << "'";
  }
  return 0;
}
} // namespace fcsgenome
