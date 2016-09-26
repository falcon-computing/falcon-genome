#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <cmath>
#include <iomanip>
#include <glog/logging.h>
#include <string>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/workers.h"

namespace fcsgenome {

int baserecal_main(int argc, char** argv,
    boost::program_options::options_description &opt_desc) 
{
  namespace po = boost::program_options;

  // Define arguments
  po::variables_map cmd_vm;

  opt_desc.add_options() 
    arg_decl_string("ref,r", "reference genome path")
    arg_decl_string("input,i", "input BAM file or dir")
    arg_decl_string("output,o", "output BQSR file")
    ("knownSites", po::value<std::vector<std::string> >(),
     "known sites for base recalibration")
    ("help,h", "print help messages")
    ("force,f", "overwrite output file if exists");

    // Parse arguments
    po::store(po::parse_command_line(argc, argv, opt_desc),
        cmd_vm);

    if (cmd_vm.count("help")) { 
      throw helpRequest();
    } 

    // Check if required arguments are presented
    bool flag_f             = get_argument<bool>(cmd_vm, "force");
    std::string ref_path    = get_argument<std::string>(cmd_vm, "ref",
                                  conf_default_ref);
    std::string input_path  = get_argument<std::string>(cmd_vm, "input");
    std::string output_path = get_argument<std::string>(cmd_vm, "output");

    std::vector<std::string> known_sites = get_argument<
      std::vector<std::string> >(cmd_vm, "knownSites");

    // finalize argument parsing
    po::notify(cmd_vm);

    if (boost::filesystem::is_directory(input_path)) {
      // TODO
      throw fileNotFound("Do not support input as a directory yet");
    }

    std::vector<std::string> intv_paths = init_contig_intv(ref_path);
    std::vector<std::string> bqsr_paths(conf_gatk_ncontigs);

    Executor executor("Base Recalibrator", conf_bqsr_nprocs);

    // compute bqsr for each contigs
    for (int contig = 0; contig < conf_gatk_ncontigs; contig++) {
      std::stringstream ss;
      ss << output_path << "." << contig;
      bqsr_paths[contig] = ss.str();
      DLOG(INFO) << "Task " << contig << " bqsr: " << bqsr_paths[contig];

      Worker_ptr worker(new BQSRWorker(ref_path, known_sites,
            intv_paths[contig],
            input_path, bqsr_paths[contig], 
            contig, flag_f));

      executor.addTask(worker);
    }
    // gather bqsr for contigs
    Worker_ptr worker(new BQSRGatherWorker(bqsr_paths,
          output_path, flag_f));

    executor.addTask(worker, true);
    executor.run();

    // delete all partial contig bqsr
    for (int contig = 0; contig < conf_gatk_ncontigs; contig++) {
      remove_path(bqsr_paths[contig]);
    }
   
  return 0;
}

int pr_main(int argc, char** argv,
    boost::program_options::options_description &opt_desc) 
{
  namespace po = boost::program_options;

  // Define arguments
  po::variables_map cmd_vm;

  opt_desc.add_options() 
    arg_decl_string("ref,r", "reference genome path")
    arg_decl_string("bqsr,b", "input BQSR file")
    arg_decl_string("input,i", "input BAM file or dir")
    arg_decl_string("output,o", "output BAM files")
    ("help,h", "print help messages")
    ("force,f", "overwrite output file if exists");

  // Parse arguments
  po::store(
      po::parse_command_line(argc, argv, opt_desc),
      cmd_vm);

  if (cmd_vm.count("help")) { 
    throw helpRequest();
  } 

  // Check if required arguments are presented
  bool flag_f             = get_argument<bool>(cmd_vm, "force");
  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref",
                                conf_default_ref);
  std::string bqsr_path   = get_argument<std::string>(cmd_vm, "bqsr", "");
  std::string input_path  = get_argument<std::string>(cmd_vm, "input");
  std::string output_path = get_argument<std::string>(cmd_vm, "output");

  // finalize argument parsing
  po::notify(cmd_vm);

  // the output path will be a directory
  // TODO: handle the case where this directory already exists
  create_dir(output_path);

  std::vector<std::string> intv_paths = init_contig_intv(ref_path);

  Executor executor("Print Reads", conf_gatk_ncontigs);

  for (int contig = 0; contig < conf_gatk_ncontigs; contig++) {
    Worker_ptr worker(new PRWorker(ref_path,
          intv_paths[contig], bqsr_path,
          input_path,
          get_contig_fname(output_path, contig),
          contig, flag_f));

    executor.addTask(worker);
  }
  executor.run();
}

int bqsr_main(int argc, char** argv,
    boost::program_options::options_description &opt_desc) 
{
  namespace po = boost::program_options;

  // Define arguments
  po::variables_map cmd_vm;

  opt_desc.add_options() 
    arg_decl_string("ref,r", "reference genome path")
    arg_decl_string("bqsr,b", "output BQSR file")
    arg_decl_string("input,i", "input BAM file or dir")
    arg_decl_string("output,o", "output BAM files")
    ("knownSites", po::value<std::vector<std::string> >(),
     "known sites for base recalibration")
    ("help,h", "print help messages")
    ("force,f", "overwrite output file if exists");

  // Parse arguments
  po::store(
      po::parse_command_line(argc, argv, opt_desc),
      cmd_vm);

  if (cmd_vm.count("help")) { 
    throw helpRequest();
  } 

  // Check if required arguments are presented
  bool flag_f             = get_argument<bool>(cmd_vm, "force");
  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref", 
      conf_default_ref);
  std::string bqsr_path   = get_argument<std::string>(cmd_vm, "bqsr", "");
  std::string input_path  = get_argument<std::string>(cmd_vm, "input");
  std::string output_path = get_argument<std::string>(cmd_vm, "output");

  bool delete_bqsr = bqsr_path.empty();
  if (delete_bqsr) {
    bqsr_path = input_path + ".grp";
    DLOG(INFO) << "bqsr_path = " << bqsr_path;
  }

  std::vector<std::string> known_sites = get_argument<
    std::vector<std::string> >(cmd_vm, "knownSites");

  // finalize argument parsing
  po::notify(cmd_vm);

  if (boost::filesystem::is_directory(input_path)) {
    // TODO
    throw fileNotFound("Do not support input as a directory yet");
  }

  std::vector<std::string> intv_paths = init_contig_intv(ref_path);
  std::vector<std::string> bqsr_paths(conf_gatk_ncontigs);

  Executor executor("Base Recalibration", conf_bqsr_nprocs);

  // first, do base recal
  for (int contig = 0; contig < conf_gatk_ncontigs; contig++) {
    std::stringstream ss;
    ss << bqsr_path << "." << contig;
    bqsr_paths[contig] = ss.str();
    DLOG(INFO) << "Task " << contig << " bqsr: " << bqsr_paths[contig];

    Worker_ptr worker(new BQSRWorker(ref_path, known_sites,
          intv_paths[contig],
          input_path, bqsr_paths[contig], 
          contig, flag_f));

    executor.addTask(worker);
  }

  // gather bqsr for contigs
  Worker_ptr worker(new BQSRGatherWorker(bqsr_paths,
        bqsr_path, flag_f));

  executor.addTask(worker, true);

  // the output path will be a directory
  // TODO: handle the case where this directory already exists
  create_dir(output_path);

  // then, do print reads
  for (int contig = 0; contig < conf_gatk_ncontigs; contig++) {
    Worker_ptr worker(new PRWorker(ref_path,
          intv_paths[contig], bqsr_path,
          input_path,
          get_contig_fname(output_path, contig),
          contig, flag_f));

    executor.addTask(worker, contig==0);
  }
  executor.run();

  // delete all partial contig bqsr
  for (int contig = 0; contig < conf_gatk_ncontigs; contig++) {
    remove_path(bqsr_paths[contig]);
  }
 
  if (delete_bqsr) {
    remove_path(bqsr_path);
  }

  return 0;
}
} // namespace fcsgenome
