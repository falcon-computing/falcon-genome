#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <sys/resource.h>

#include <bits/stdc++.h> 

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/workers.h"

namespace fcsgenome {

int markdup_main(int argc, char** argv,
    boost::program_options::options_description &opt_desc) 
{
  namespace po = boost::program_options;

  // Define arguments
  po::variables_map cmd_vm;

  opt_desc.add_options() 
    ("input,i", po::value<std::string>()->required(), "input file")
    ("output,o", po::value<std::string>()->required(), "output file")
    ("sample-id", po::value<std::string>()->implicit_value(""), "sample id for log files");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc),
      cmd_vm);

  if (cmd_vm.count("help")) { 
    throw helpRequest();
  } 

  // Check if required arguments are presented
  bool        flag_f      = get_argument<bool>(cmd_vm, "force", "f");
  std::string input_path  = get_argument<std::string>(cmd_vm, "input", "i");
  std::string output_path = get_argument<std::string>(cmd_vm, "output", "o");
  std::string sample_id  = get_argument<std::string>(cmd_vm, "sample-id");

  // finalize argument parsing 
  po::notify(cmd_vm);

  if (cmd_vm.count("sample-id") && sample_id.empty()) {
    throw pathEmpty("sample-id");
  }

  Executor executor("Mark Duplicates");
  Worker_ptr worker(new SambambaWorker(input_path, output_path, SambambaWorker::MARKDUP, ".*/part-[0-9].*.*", flag_f));
  executor.addTask(worker, sample_id);
  executor.run();

  return 0;
}
} // namespace fcsgenome
