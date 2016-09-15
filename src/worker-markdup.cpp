#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <glog/logging.h>
#include <sys/resource.h>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"

namespace fcsgenome {

int markdup_command(
    std::string& input_files,
    std::string& output_file) 
{
  //int num_parts = get_columns(input_files);

  // update limit
  struct rlimit file_limit;
  file_limit.rlim_cur = conf_markdup_maxfile;
  file_limit.rlim_max = conf_markdup_maxfile;

  if (setrlimit(RLIMIT_NOFILE, &file_limit) != 0) {
    throw internalError("Failed to update limit");
  }
  getrlimit(RLIMIT_NOFILE, &file_limit);
  if (file_limit.rlim_cur != conf_markdup_maxfile || 
      file_limit.rlim_max != conf_markdup_maxfile) {
    throw internalError("Failed to update limit");
  }

  create_dir(conf_log_dir);
  std::string log_fname = conf_log_dir
                        + "/markdup-" 
                        + get_basename_wo_ext(output_file) 
                        + ".log";
  std::stringstream cmd;
  cmd << conf_sambamba_call << " markdup "
      << "-l 1 "
      << "-t " << conf_markdup_nthreads << " "
      << "--tmpdir=" << conf_temp_dir << " "
      << "--overflow-list-size=" << conf_markdup_overflowsize << " "
      << input_files << " "
      << output_file << " &> " << log_fname;

  DLOG(INFO) << cmd.str();
  int ret = system(cmd.str().c_str());
  if (ret) {
    std::string msg = "Mark Duplicate failed, please check "
                     + log_fname + " for details";
    throw failedCommand(msg);
  }
}

int markdup_main(int argc, char** argv) {

  namespace po = boost::program_options;
  std::string str_opt;

  // Define arguments
  po::options_description opt_desc("'fcs-genome markdup' options"); 
  opt_desc.add_options() 
    arg_decl_string("input,i", "input file")
    arg_decl_string("output,o", "output file")
    ("h,help", "print help messages") 
    ("f,force", "overwrite output file if exists");

  boost::program_options::variables_map cmd_vm;

  try {
    // Parse arguments
    po::store(
        po::parse_command_line(argc, argv, opt_desc),
        cmd_vm);

    if (cmd_vm.count("help")) { 
      std::cerr << opt_desc << std::endl; 
      return 0; 
    } 

    // Check if required arguments are presented
    bool        flag_f      = get_argument<bool>(cmd_vm, "force");
    std::string input_path  = get_argument<std::string>(cmd_vm, "input");
    std::string output_path = get_argument<std::string>(cmd_vm, "output");

    po::notify(cmd_vm);
    
    input_path  = check_input(input_path);
    output_path = check_output(output_path, flag_f);

    // Get input files
    std::string input_files = get_input_list(input_path, ".*/part-[0-9].*");
    if (input_files.empty()) {
      throw fileNotFound("Cannot find input file(s) in " + input_path);
    }
    
    LOG(INFO) << "Start stage";
    uint64_t start_ts = getTs();
    markdup_command(input_files, output_path);
    log_time("Mark duplicates", start_ts);
  } 
  catch (invalidParam &e) { 
    LOG(ERROR) << "Missing argument '" << e.what() << "'";
    std::cerr << opt_desc << std::endl; 
    return 1;
  }
  catch (boost::program_options::error &e) { 
    LOG(ERROR) << "Failed to parse arguments, " << e.what();
    std::cerr << opt_desc << std::endl; 
    return 1; 
  } 
  return 0;
}
} // namespace fcsgenome
