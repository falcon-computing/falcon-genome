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

int htc_main(int argc, char** argv,
    boost::program_options::options_description &opt_desc) 
{
  namespace po = boost::program_options;

  // Define arguments
  po::variables_map cmd_vm;

  opt_desc.add_options() 
    arg_decl_string("ref,r", "reference genome path")
    arg_decl_string("input,i", "input BAM file or dir")
    arg_decl_string("output,o", "output gvcf file (if --skip-concat is set"
                                "the output will be a directory of gvcf files)")
    ("skip-concat,s", "produce a set of gvcf files instead of one");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc),
      cmd_vm);

  if (cmd_vm.count("help")) { 
    throw helpRequest();
  } 

  // Check if required arguments are presented
  bool flag_f             = get_argument<bool>(cmd_vm, "force");
  bool flag_skip_concat   = get_argument<bool>(cmd_vm, "skip-concat");
  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref",
                                get_config<std::string>("ref_genome"));
  std::string input_path  = get_argument<std::string>(cmd_vm, "input");
  std::string output_path = get_argument<std::string>(cmd_vm, "output");

  // finalize argument parsing
  po::notify(cmd_vm);

  std::string temp_dir = conf_temp_dir + "/htc";

  // TODO: deal with the case where 
  // 1. output_path is a dir but should not be deleted
  // 2. output_path is a file
  std::string output_dir;

  if (flag_skip_concat) {
    output_dir = check_output(output_path, flag_f);
  }
  else {
    output_dir = temp_dir;
  }
  std::string temp_gvcf_path = output_dir + "/" + get_basename(output_path);

  create_dir(output_dir);

  std::vector<std::string> output_files(get_config<int>("gatk.ncontigs"));
  std::vector<std::string> intv_paths = init_contig_intv(ref_path);

  Executor executor("Haplotype Caller", get_config<int>("gatk.htc.nprocs"));

  bool flag_htc_f = !flag_skip_concat || flag_f;
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
    std::string output_file = get_contig_fname(output_dir, contig, "gvcf");
    Worker_ptr worker(new HTCWorker(ref_path,
          intv_paths[contig], input_file,
          output_file,
          contig, flag_htc_f));
    output_files[contig] = output_file;

    executor.addTask(worker);
  }

  if (!flag_skip_concat) {

    bool flag = true;
    { // concat gvcfs
      Worker_ptr worker(new VCFConcatWorker(
            output_files, temp_gvcf_path,
            flag));
      executor.addTask(worker, true);
    }
    { // sort gvcf
      Worker_ptr worker(new VCFSortWorker(temp_gvcf_path));
      executor.addTask(worker, true);
    }
    { // bgzip gvcf
      Worker_ptr worker(new ZIPWorker(
            temp_gvcf_path, output_path+".gz",
            flag_f));
      executor.addTask(worker, true);
    }
    { // tabix gvcf
      Worker_ptr worker(new TabixWorker(
            output_path + ".gz"));
      executor.addTask(worker, true);
    }
  }
  executor.run();

  return 0;
}
} // namespace fcsgenome
