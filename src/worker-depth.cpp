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

int depth_main(int argc, char** argv,
    boost::program_options::options_description &opt_desc)
{
  namespace po = boost::program_options;

  // Define arguments
  po::variables_map cmd_vm;
  bool opt_bool = false;

  opt_desc.add_options()
    ("ref,r", po::value<std::string>()->required(), "reference genome path")
    ("input,i", po::value<std::string>()->required(),"input BAM file")
    ("output,o", po::value<std::string>()->required(),"output coverage file")
    ("intervalList,L", po::value<std::string>(), "Interval List BED File")
    ("geneList,g", po::value<std::string>(), "list of genes over which the coverage is calculated")
    ("omitBaseOutput,b", "omit output coverage depth at each base (default: false)")
    ("omitIntervals,v", "omit output coverage per-interval statistics (default false)")
    ("omitSampleSummary,s", "omit output summary files for each sample (default false");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc), cmd_vm);

  if (cmd_vm.count("help")) {
    throw helpRequest();
  }

  // Check configurations
  check_nprocs_config("depth");
  check_memory_config("depth");

  // Check if required arguments are presented

  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref", "r");
  std::string input_path  = get_argument<std::string>(cmd_vm, "input", "i");
  std::string output_path = get_argument<std::string>(cmd_vm, "output", "o");
  std::string intv_list = get_argument<std::string>(cmd_vm, "intervalList", "L");
  std::string geneList = get_argument<std::string>(cmd_vm, "geneList", "g");
  bool flag_f = get_argument<bool>(cmd_vm, "force", "f");
  bool flag_baseCoverage     = get_argument<bool>(cmd_vm, "omitBaseOutput", "b");
  bool flag_intervalCoverage = get_argument<bool>(cmd_vm, "omitIntervals", "v");
  bool flag_sampleSummary    = get_argument<bool>(cmd_vm, "omitSampleSummary", "s");

  bool flag_genes;
  if ( !intv_list.empty() && !geneList.empty() ){
    flag_genes=true;
  } else {
    flag_genes=false;
  }

  std::vector<std::string> extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");

  // finalize argument parsing
  po::notify(cmd_vm);

  std::string temp_dir = conf_temp_dir + "/depth";
  create_dir(temp_dir);
  std::string output_dir = temp_dir;
  //create_dir(output_dir);

  // Split Interval List and Gene List into several parts according to gatk.ncontigs:
  std::vector<std::string> output_files(get_config<int>("gatk.ncontigs"));

  std::vector<std::string> intv_paths;
  std::vector<std::string> geneList_paths;

  if (!intv_list.empty() && !geneList.empty()) {
     DLOG(INFO) << "intv_list NOT EMPTY and geneList NOT EMPTY";
     intv_paths = split_by_nprocs(intv_list, "bed");
     geneList_paths = split_by_nprocs(geneList, "list");
  }

  if (!intv_list.empty() && geneList.empty()) {
     DLOG(INFO) << "intv_list NOT EMPTY and geneList EMPTY";
     intv_paths = split_by_nprocs(intv_list, "bed");
     for (int k = 0; k < get_config<int>("gatk.ncontigs"); k++ ) geneList_paths.push_back("");
  }

  if (intv_list.empty() && geneList.empty()) {
     DLOG(INFO) << "intv_list EMPTY and geneList EMPTY";
     intv_paths = split_ref_by_nprocs(ref_path);
     for (int k = 0; k < get_config<int>("gatk.ncontigs"); k++ ) geneList_paths.push_back("");
  }

  if (intv_list.empty() && !geneList.empty()) {
     intv_paths = split_ref_by_nprocs(ref_path);
     for (int k = 0; k < get_config<int>("gatk.ncontigs"); k++ ) geneList_paths.push_back("");
     DLOG(INFO) << "intv_list EMPTY and geneList NOT EMPTY";
  }

  DLOG(INFO) << "intv_paths Size: " << intv_paths.size();

  std::string input_file;
  if (boost::filesystem::is_directory(input_path)) {
      // Merging BAM files if the input is a folder containing PARTS BAM files:
      DLOG(INFO) << input_path << " is a directory.  Proceed to merge all BAM files" << std::endl;
      std::string mergeBAM = input_path + "/merge_parts.bam  ";
      std::stringstream partsBAM;
      std::string parts_dir = input_path;
      std::vector<std::string> input_files_ ;
      int check_parts = 1;
      get_input_list(parts_dir, input_files_, ".*/part-[0-9].*bam", true);
      for (int n = 0; n < input_files_.size(); n++) {
           partsBAM << input_files_[n] << " ";
      }
      if (input_files_.size() == 1) check_parts = 0;
      uint64_t start_merging = getTs();
      std::string log_filename_merge  = input_path + "/mergebam.log";
      std::ofstream merge_log;
      merge_log.open(log_filename_merge, std::ofstream::out | std::ofstream::app);
      merge_log << input_path << ":" << "Start Merging BAM Files " << std::endl;
      Executor merger_executor("Merge BAM files");
      Worker_ptr merger_worker(new MergeBamWorker(partsBAM.str(), mergeBAM, check_parts, flag_f));
      merger_executor.addTask(merger_worker);
      merger_executor.run();
      DLOG(INFO) << "Merging Parts BAM in  " << input_path << " completed " << std::endl;
      merge_log << input_path << ":" << "Merging BAM files finishes in " << getTs() - start_merging << " seconds" << std::endl;
      merge_log.close(); merge_log.clear();
      input_file = mergeBAM;
  }
  else {
      input_file = input_path;
  }

  Executor executor("Depth", get_config<int>("gatk.depth.nprocs"));

  for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {
       std::string file_ext = "cov";
       std::string output_file = get_contig_fname(output_dir, contig, file_ext);
       Worker_ptr worker(new DepthWorker(ref_path,
              intv_paths[contig],
              input_file,
              output_file,
              geneList_paths[contig],
              extra_opts,
              contig,
              flag_f,
              flag_baseCoverage,
              flag_intervalCoverage,
              flag_sampleSummary));
       output_files[contig] = output_file;
       executor.addTask(worker);
  }

  bool flag = true;

  Worker_ptr worker(new DepthCombineWorker(output_files, output_path,
        flag_baseCoverage, flag_intervalCoverage, flag_sampleSummary, flag_genes, flag));
  executor.addTask(worker, true);
  executor.run();

  return 0;
}
} // namespace fcsgenome
