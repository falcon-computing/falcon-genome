#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
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
    arg_decl_string("output,o", "output BAM file (if --align-only is set "
                                "the output will be a directory of BAM "
                                "files)")
    arg_decl_string("rg,R", "read group id ('ID' in BAM header)")
    arg_decl_string("sp,S", "sample id ('SM' in BAM header)")
    arg_decl_string("pl,P", "platform id ('PL' in BAM header)")
    arg_decl_string("lb,L", "library id ('LB' in BAM header)")
    ("align-only,l", "skip mark duplicates");

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
                              get_config<std::string>("ref_genome"));
  std::string fq1_path    = get_argument<std::string>(cmd_vm, "fastq1");
  std::string fq2_path    = get_argument<std::string>(cmd_vm, "fastq2");
  std::string output_path = get_argument<std::string>(cmd_vm, "output");
  std::string read_group  = get_argument<std::string>(cmd_vm, "rg");
  std::string sample_id   = get_argument<std::string>(cmd_vm, "sp");
  std::string platform_id = get_argument<std::string>(cmd_vm, "pl");
  std::string library_id  = get_argument<std::string>(cmd_vm, "lb");

  std::vector<std::string> extra_opts = 
          get_argument<std::vector<std::string>>(cmd_vm, "extra-options");

  // finalize argument parsing
  po::notify(cmd_vm);

  // start execution
  std::string parts_dir;
  std::string temp_dir = conf_temp_dir + "/align";

  create_dir(temp_dir);

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
    
    // workaround for output check
    create_dir(output_path+"/"+sample_id);
  }
  else {
    // check output path before alignment
    output_path = check_output(output_path, flag_f, true);

    // require output to be a file
    parts_dir = temp_dir + "/" +
                get_basename(output_path) + ".parts";
  }
  DLOG(INFO) << "Putting sorted BAM parts in '" << parts_dir << "'";

  Executor executor("bwa mem");

  Worker_ptr worker(new BWAWorker(ref_path,
        fq1_path, fq2_path,
        parts_dir,
        extra_opts,
        sample_id, read_group,
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
