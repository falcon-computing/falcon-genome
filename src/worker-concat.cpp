#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <cmath>
#include <iomanip>
#include <string>


#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/workers.h"

namespace fcsgenome {

int concat_main(int argc, char** argv, 
    boost::program_options::options_description &opt_desc) 
{
  namespace po = boost::program_options;

  // Define arguments
  po::variables_map cmd_vm;

  opt_desc.add_options() 
    arg_decl_string("input,i", "folder of input vcf/gvcf files")
    arg_decl_string("output,o", "output vcf/gvcf file (automatically "
                                "compressed to .vcf.gz/.gvcf.gz)");

  // Parse arguments
  po::store(
      po::parse_command_line(argc, argv, opt_desc),
      cmd_vm);

  if (cmd_vm.count("help")) { 
    throw helpRequest();
  } 

  // Check if required arguments are presented
  bool flag_f             = get_argument<bool>(cmd_vm, "force", "f");
  std::string input_path  = get_argument<std::string>(cmd_vm, "input", "i");
  std::string output_path = get_argument<std::string>(cmd_vm, "output", "o");

  // finalize argument parsing
  po::notify(cmd_vm);

  std::vector<std::string> input_files;
  try {
    get_input_list(input_path, input_files, ".*\\.gvcf");
  } catch (fileNotFound &e) {
    // will throw if cannot find any .vcf files
    get_input_list(input_path, input_files, ".*\\.vcf");
  }

  Executor executor("VCF concat");
  { // concat gvcfs
    // TODO: auto detect the input and decide flag_a
    bool flag_a = false;
    Worker_ptr worker(new VCFConcatWorker(
          input_files, output_path,
          flag_a, flag_f));
    executor.addTask(worker);
  }
  //{ // sort gvcf
  //  Worker_ptr worker(new VCFSortWorker(output_path));
  //  executor.addTask(worker, true);
  //}
  { // bgzip gvcf
    Worker_ptr worker(new ZIPWorker(
          output_path, output_path+".gz",
          flag_f));
    executor.addTask(worker, true);
  }
  { // tabix gvcf
    Worker_ptr worker(new TabixWorker(
          output_path + ".gz"));
    executor.addTask(worker, true);
  }
  executor.run();

  remove_path(output_path);
   
  return 0;
}
} // namespace fcsgenome
