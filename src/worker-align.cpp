#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <glog/logging.h>
#include <string>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/workers.h"

namespace fcsgenome {

int align_main(int argc, char** argv,
    boost::program_options::options_description &opt_desc) 
{
  namespace po = boost::program_options;

  // Define arguments
  po::variables_map cmd_vm;

  opt_desc.add_options() 
    arg_decl_string("ref,r", "reference genome path")
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
  po::store(po::parse_command_line(argc, argv, opt_desc),
      cmd_vm);

  if (cmd_vm.count("help")) { 
    throw helpRequest();
  } 

  // Check if required arguments are presented
  bool flag_f          = get_argument<bool>(cmd_vm, "force");
  bool flag_align_only = get_argument<bool>(cmd_vm, "align-only");

  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref",
      conf_default_ref);
  std::string fq1_path    = get_argument<std::string>(cmd_vm, "fastq1");
  std::string fq2_path    = get_argument<std::string>(cmd_vm, "fastq2");
  std::string output_path = get_argument<std::string>(cmd_vm, "output");
  std::string read_group  = get_argument<std::string>(cmd_vm, "rg");
  std::string sample_id   = get_argument<std::string>(cmd_vm, "sp");
  std::string platform_id = get_argument<std::string>(cmd_vm, "pl");
  std::string library_id  = get_argument<std::string>(cmd_vm, "lb");

  // finalize argument parsing
  po::notify(cmd_vm);

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
    //create_dir(parts_dir);
  }
  else {
    // require output to be a file
    output_path = check_output(output_path, flag_f, true);
    parts_dir = output_path + ".parts";

    // Remove parts_dir if it already exists
    remove_path(parts_dir);

    DLOG(INFO) << "Putting sorted BAM parts in '" << parts_dir << "'";
  }

  Executor executor("bwa mem");

  Worker_ptr worker(new BWAWorker(ref_path,
        fq1_path, fq2_path,
        parts_dir,
        read_group, sample_id,
        platform_id, library_id, flag_f));

  executor.addTask(worker);
  executor.run();

  if (!flag_align_only) {
    Executor executor("Mark Duplicates");
    Worker_ptr worker(new MarkdupWorker(parts_dir, output_path, flag_f));
    executor.addTask(worker);
    executor.run();

    // Remove parts_dir
    remove_path(parts_dir);
    DLOG(INFO) << "Removing temp file in '" << parts_dir << "'";
  }

  return 0;
}
} // namespace fcsgenome
