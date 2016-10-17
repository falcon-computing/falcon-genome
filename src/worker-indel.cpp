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

int ir_main(int argc, char** argv,
    boost::program_options::options_description &opt_desc) 
{
  namespace po = boost::program_options;

  // Define arguments
  po::variables_map cmd_vm;

  opt_desc.add_options() 
    arg_decl_string("ref,r", "reference genome path")
    arg_decl_string("input,i", "input BAM file or dir")
    arg_decl_string("output,o", "output diretory of BAM files")
    ("known,K", po::value<std::vector<std::string> >(),
     "known indels for realignment");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc),
      cmd_vm);

  if (cmd_vm.count("help")) { 
    throw helpRequest();
  } 

  // Check if required arguments are presented
  bool flag_f             = get_argument<bool>(cmd_vm, "force");
  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref",
                                get_config<std::string>("ref_genome"));
  std::string input_path  = get_argument<std::string>(cmd_vm, "input");
  std::string output_path = get_argument<std::string>(cmd_vm, "output");
  std::string target_path = input_path + ".intervals";

  std::vector<std::string> known_indels = get_argument<
    std::vector<std::string> >(cmd_vm, "known");

  // finalize argument parsing
  po::notify(cmd_vm);

  // the output path will be a directory
  output_path = check_output(output_path, flag_f);
  create_dir(output_path);

  Executor executor("Indel Realignment", get_config<int>("gatk.indel.nprocs"));

  { // realign target creator
    Worker_ptr worker(new RTCWorker(ref_path, known_indels,
          input_path, target_path));
    executor.addTask(worker, true);
  }

  std::vector<std::string> intv_paths = init_contig_intv(ref_path);
  for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {
    std::string input_file;
    if (boost::filesystem::is_directory(input_path)) {
      // if input is a directory, automatically go into contig mode
      // TODO: here should validate if the bam files in folder
      //       is generated by fcs-genome with the same num contigs
      input_file = get_contig_fname(input_path, contig);
    }
    else {
      input_file = input_path;
    }
    Worker_ptr worker(new IndelWorker(ref_path, known_indels,
          intv_paths[contig],
          input_file, target_path,
          get_contig_fname(output_path, contig),
          flag_f));
    executor.addTask(worker, contig==0);
  }
  executor.run();

  // delete all partial contig bqsr
  remove_path(target_path);

  return 0;
}
} // namespace fcsgenome