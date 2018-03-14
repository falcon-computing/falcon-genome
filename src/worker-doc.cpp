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
    arg_decl_string("output,o", "output file prefix")
    ("genelist,g", po::value<std::string>()->default_value(""), 
                       "a gene list file given to caculate coverage over these genes")
    ("intervalList,L", po::value<std::string>()->default_value(""), 
                       "a interval list file given to caculate coverage over these intervals")
    ("depthCutoff,d", po::value<int>()->default_value(15), "cutoff for coverage depth summary")  // this place shuld be adjusted, the arg is not string
    ("baseCoverage,b", "caculate coverage depth of each base")
    ("intervalCoverage,v", "caculate coverage summacy of given intervals")
    ("sampleSummary,s", "output summary files for each sample");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc),
      cmd_vm);

  if (cmd_vm.count("help")) { 
    throw helpRequest();
  } 

  // Check if required arguments are presented
  bool flag_f            = get_argument<bool>(cmd_vm, "force");
  bool flag_baseCoverage = get_argument<bool>(cmd_vm, "baseCoverage");
  bool flag_intervalCoverage = get_argument<bool>(cmd_vm, "intervalCoverage");
  bool flag_sampleSummary = get_argument<bool>(cmd_vm, "sampleSummary");

  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref",
                              get_config<std::string>("ref_genome"));
  std::string input_path  = get_argument<std::string>(cmd_vm, "input-bam");
  std::string output_path = get_argument<std::string>(cmd_vm, "output");
  std::string geneList    = get_argument<std::string>(cmd_vm, "genelist");
  std::string intervalList = get_argument<std::string>(cmd_vm, "intervalList");
  int depthCutoff = get_argument<int>(cmd_vm, "depthCutoff");

  // finalize argument parsing
  po::notify(cmd_vm);

  Executor* executor = create_executor("depthOfCoverage", 4);
  Worker_ptr worker(new DOCWorker(ref_path, input_path, output_path, geneList, intervalList, depthCutoff,
  		    flag_f, flag_baseCoverage, flag_intervalCoverage, flag_sampleSummary));
  executor->addTask(worker);
  executor->run();

  return 0;
}
} // namespace fcsgenome
