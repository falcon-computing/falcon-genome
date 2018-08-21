#include <boost/algorithm/string.hpp>
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

static void baserecalAddWorkers(Executor &executor,
    std::string &ref_path,
    std::vector<std::string> &known_sites,
    std::vector<std::string> &extra_opts,
    std::string &input_path,
    std::string &output_path,
    std::vector<std::string> &intv_list,
    bool flag_f, bool flag_gatk)
{

  std::vector<std::string> intv_sets;
  if (!intv_list.empty()){
     intv_sets  = split_by_nprocs(intv_list, "bed");
  }

  std::vector<std::string> intv_paths = init_contig_intv(ref_path);
  std::vector<std::string> bqsr_paths(get_config<int>("gatk.ncontigs"));

  // compute bqsr for each contigs
  for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {
    // handle input as directory
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

    // output bqsr filename
    std::stringstream ss;
    ss << output_path << "." << contig;
    bqsr_paths[contig] = ss.str();
    DLOG(INFO) << "Task " << contig << " bqsr: " << bqsr_paths[contig];
    if (intv_list.empty()) intv_sets[contig] = " ";

    Worker_ptr worker(new BQSRWorker(ref_path, known_sites,
	  intv_paths[contig],
	  input_file, bqsr_paths[contig],
	  extra_opts, intv_sets[contig],
	  contig, flag_f, flag_gatk));
    executor.addTask(worker, contig == 0);
  }

  // gather bqsr for contigs
  Worker_ptr worker(new BQSRGatherWorker(bqsr_paths, output_path, flag_f, flag_gatk));

  executor.addTask(worker, true);
}

static void removePartialBQSR(std::string bqsr_path) {
  // delete all partial contig bqsr
  for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {
     std::stringstream ss;
     ss << bqsr_path << "." << contig;
     remove_path(ss.str());
  }
}

static void prAddWorkers(Executor &executor,
  std::string &ref_path,
  std::string &input_path,
  std::string &bqsr_path,
  std::string &output_path,
  std::vector<std::string> &extra_opts,
  std::vector<std::string> &intv_list,
  bool flag_f, bool flag_gatk)
{

  std::vector<std::string> intv_sets;
  if (!intv_list.empty()){
     intv_sets  = split_by_nprocs(intv_list, "bed");
  }
  std::vector<std::string> intv_paths = init_contig_intv(ref_path);

  for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {
       // handle input as directory
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
       if (intv_list.empty()) intv_sets[contig] = " ";

       Worker_ptr worker(new PRWorker(ref_path,
          intv_paths[contig], bqsr_path,
          input_file,
          get_contig_fname(output_path, contig),
	        extra_opts, intv_sets[contig],
          contig, flag_f, flag_gatk));
       executor.addTask(worker, contig == 0);
  }

}

static void mergebamBQSRWorker(Executor &merge_executor,
  std::string &output_path,
  std::string &mergeBAM_path, bool flag_f)
{
  output_path = check_input(output_path);
  std::stringstream partsBAM;
  int check_parts = 1;  // For more than 1 part BAM file
  std::string inputPartsBAM;
  for (int n = 0; n < get_config<int>("gatk.ncontigs"); n++) {
       if (boost::filesystem::is_directory(output_path)) {
           inputPartsBAM = get_contig_fname(output_path, n);
       };
       partsBAM << inputPartsBAM << " ";
  }
  DLOG(INFO) << "Input Part BAM files: " << partsBAM.str() << "\n";
  DLOG(INFO) << "Output Merged BAM file: " << mergeBAM_path << "\n";
  Worker_ptr merger_worker(new MergeBamWorker(partsBAM.str(), mergeBAM_path, check_parts, flag_f));
  merge_executor.addTask(merger_worker);
}

int baserecal_main(int argc, char** argv, boost::program_options::options_description &opt_desc)
{
  namespace po = boost::program_options;

  // Define arguments
  po::variables_map cmd_vm;
  opt_desc.add_options()
    ("ref,r", po::value<std::string>()->required(), "reference genome path")
    ("input,i", po::value<std::string>()->required(), "input BAM file or dir")
    ("output,o", po::value<std::string>()->required(), "output BQSR file")
    ("knownSites,K", po::value<std::vector<std::string> >(), "known sites for base recalibration")
    ("gatk4,g", "use gatk4 to perform analysis")
    ("intervalList,L", po::value<std::vector<std::string> >(), "interval list file");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc), cmd_vm);

  if (cmd_vm.count("help")) {
    throw helpRequest();
  }

  // Check if required arguments are presented
  bool flag_f             = get_argument<bool>(cmd_vm, "force", "f");
  bool flag_gatk          = get_argument<bool>(cmd_vm, "gatk4", "g");
  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref", "r");
  std::string input_path  = get_argument<std::string>(cmd_vm, "input", "i");
  std::string output_path = get_argument<std::string>(cmd_vm, "output", "o");
  std::vector<std::string> intv_list = get_argument<std::vector<std::string>>(cmd_vm, "intervalList", "L");
  std::vector<std::string> known_sites = get_argument<std::vector<std::string>>(cmd_vm, "knownSites", "K");

  std::vector<std::string> extra_opts =
          get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");

  // finalize argument parsing
  po::notify(cmd_vm);

  // check configurations
  check_nprocs_config("bqsr");
  check_memory_config("bqsr");

  Executor executor("Base Recalibrator", get_config<int>("gatk.bqsr.nprocs"));
  baserecalAddWorkers(executor, ref_path, known_sites, extra_opts, input_path, output_path, intv_list, flag_f, flag_gatk);

  executor.run();

  // delete all partial contig bqsr
  removePartialBQSR(output_path);

  return 0;
}

int pr_main(int argc, char** argv, boost::program_options::options_description &opt_desc)
{
  namespace po = boost::program_options;
  // Define arguments
  po::variables_map cmd_vm;
  opt_desc.add_options()
    ("ref,r", po::value<std::string>()->required(), "reference genome path")
    ("bqsr,b", po::value<std::string>()->required(), "input BQSR file")
    ("input,i", po::value<std::string>()->required(), "input BAM file or dir")
    ("output,o", po::value<std::string>()->required(), "output Folder with Parts BAM files")
    ("merge-bam,m", "merge Parts BAM files")
    ("gatk4,g", "use gatk4 to perform analysis")
    ("intervalList,L", po::value<std::vector<std::string> >(), "interval list file");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc), cmd_vm);

  if (cmd_vm.count("help")) {
    throw helpRequest();
  }

  // Check if required arguments are presented
  bool flag_f             = get_argument<bool>(cmd_vm, "force", "f");
  bool flag_gatk          = get_argument<bool>(cmd_vm, "gatk4", "g");
  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref", "r");
  std::string bqsr_path   = get_argument<std::string>(cmd_vm, "bqsr", "b");
  std::string input_path  = get_argument<std::string>(cmd_vm, "input", "i");
  std::string output_path = get_argument<std::string>(cmd_vm, "output", "o");
  bool merge_bam_flag     = get_argument<bool>(cmd_vm, "merge-bam", "m");
  std::vector<std::string> intv_list = get_argument<std::vector<std::string> >(cmd_vm, "intervalList", "L");
  std::vector<std::string> extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");

  // finalize argument parsing
  po::notify(cmd_vm);

  // check configurations
  check_nprocs_config("pr");
  check_memory_config("pr");

  // the output path will be a directory
  create_dir(output_path);

  Executor executor("Print Reads", get_config<int>("gatk.pr.nprocs", "gatk.nprocs"));
  prAddWorkers(executor, ref_path, input_path, bqsr_path, output_path, extra_opts, intv_list, flag_f, flag_gatk);

  executor.run();

  if (merge_bam_flag){
      Executor merge_executor("Print Reads", get_config<int>("gatk.pr.nprocs", "gatk.nprocs"));
      std::string mergeBAM_path(output_path);
      boost::replace_all(mergeBAM_path, ".bam", "_merged.bam");
      mergebamBQSRWorker(merge_executor, output_path, mergeBAM_path, flag_f);
      merge_executor.run();

      // Removing Part BAM files:
      remove_path(output_path);
  }

}

int bqsr_main(int argc, char** argv, boost::program_options::options_description &opt_desc)
{
  namespace po = boost::program_options;

  // Define arguments
  po::variables_map cmd_vm;

  opt_desc.add_options()
    ("ref,r", po::value<std::string>()->required(), "reference genome path")
    ("bqsr,b", po::value<std::string>(), "output BQSR file (if left blank no file will be produced)")
    ("input,i", po::value<std::string>()->required(), "input BAM file or dir")
    ("output,o", po::value<std::string>()->required(), "output directory of BAM files")
    ("knownSites,K", po::value<std::vector<std::string> >()->required(), "known sites for base recalibration")
    ("intervalList,L", po::value<std::vector<std::string> >(), "interval list file")
    ("gatk4,g", "use gatk4 to perform analysis")
    ("merge-bam,m", "merge Parts BAM files");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc),cmd_vm);

  if (cmd_vm.count("help")) {
    throw helpRequest();
  }

  // check configurations
  check_nprocs_config("bqsr");
  check_memory_config("bqsr");
  check_nprocs_config("pr");
  check_memory_config("pr");

  // Check if required arguments are presented
  bool flag_f             = get_argument<bool>(cmd_vm, "force", "f");
  bool flag_gatk          = get_argument<bool>(cmd_vm, "gatk4", "g");
  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref", "r");
  std::string input_path  = get_argument<std::string>(cmd_vm, "input", "i");
  std::string output_path = get_argument<std::string>(cmd_vm, "output", "o");
  std::string intv_list   = get_argument<std::string> >(cmd_vm, "intervalList", "L");
  bool merge_bam_flag     = get_argument<bool>(cmd_vm, "merge-bam", "m");

  std::vector<std::string> extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");

  std::string temp_dir = conf_temp_dir + "/bqsr";
  create_dir(temp_dir);

  bool delete_bqsr = false;
  std::string bqsr_path = get_argument<std::string>(cmd_vm, "bqsr", "b");
  if (bqsr_path.empty()) {
    delete_bqsr = true;
    bqsr_path = temp_dir + "/" +  get_basename(input_path) + ".grp";
    DLOG(INFO) << "Use default bqsr_path = " << bqsr_path;
  }

  std::vector<std::string> known_sites = get_argument<std::vector<std::string> >(cmd_vm, "knownSites", "K");

  // finalize argument parsing
  po::notify(cmd_vm);

  // the output path will be a directory
  create_dir(output_path);

  Executor executor("Base Recalibration", get_config<int>("gatk.bqsr.nprocs"));
  // first, do base recal

  baserecalAddWorkers(executor, ref_path, known_sites, extra_opts, input_path, bqsr_path, intv_list, flag_f, flag_gatk);
  prAddWorkers(executor, ref_path, input_path, bqsr_path, output_path, extra_opts, intv_list, flag_f, flag_gatk);

  executor.run();

  if (merge_bam_flag){
      Executor merge_executor("Print Reads", get_config<int>("gatk.pr.nprocs", "gatk.nprocs"));
      std::string mergeBAM_path(output_path);
      boost::replace_all(mergeBAM_path, ".bam", "_merged.bam");
      mergebamBQSRWorker(merge_executor, output_path, mergeBAM_path, flag_f);
      merge_executor.run();

      // Removing Part BAM files:
      remove_path(output_path);
  }

  removePartialBQSR(bqsr_path);

  if (delete_bqsr) {
    remove_path(bqsr_path);
  }

  return 0;
}
} // namespace fcsgenome
