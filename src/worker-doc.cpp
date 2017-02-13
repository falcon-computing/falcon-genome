#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <string>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"
//#include "fcs-genome/workers.h"
#include "fcs-genome/workers/DOCWorker.h"

namespace fcsgenome {

int doc_main(int argc, char** argv,
    boost::program_options::options_description &opt_desc) 
{
  namespace po = boost::program_options;

  // Define arguments
  po::variables_map cmd_vm;

  opt_desc.add_options() 
    arg_decl_string("ref,r", "reference genome path")
    arg_decl_string("input-bam,i", "input alignment file of bam format")
    arg_decl_string("output,o", "output file prefix");
    //("combine-only,c", "combine GVCFs only and skip genotyping")

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc),
      cmd_vm);

  if (cmd_vm.count("help")) { 
    throw helpRequest();
  } 

  // Check if required arguments are presented
  bool flag_f            = get_argument<bool>(cmd_vm, "force");
  //bool flag_combine_only = get_argument<bool>(cmd_vm, "combine-only");

  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref",
                              get_config<std::string>("ref_genome"));
  std::string input_path  = get_argument<std::string>(cmd_vm, "input-bam");
  std::string output_path = get_argument<std::string>(cmd_vm, "output");

  // finalize argument parsing
  po::notify(cmd_vm);

  Executor* executor = create_executor("depthOfCoverage", 4);
  Worker_ptr worker(new DOCWorker(ref_path, input_path, output_path, flag_f));
  executor->addTask(worker);
  executor->run();

  return 0;
}
} // namespace fcsgenome
