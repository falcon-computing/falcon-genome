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
    ("sample-name,n", po::value<std::string>(), "sample name")
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
  std::string intv_list   = get_argument<std::string>(cmd_vm, "intervalList", "L");
  std::string geneList    = get_argument<std::string>(cmd_vm, "geneList", "g");
  std::string sample_name = get_argument<std::string>(cmd_vm, "sample_name", "n");
  bool flag_f                = get_argument<bool>(cmd_vm, "force", "f");
  bool flag_baseCoverage     = get_argument<bool>(cmd_vm, "omitBaseOutput", "b");
  bool flag_intervalCoverage = get_argument<bool>(cmd_vm, "omitIntervals", "v");
  bool flag_sampleSummary    = get_argument<bool>(cmd_vm, "omitSampleSummary", "s");
  std::vector<std::string> extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");

  bool flag_genes;
  if (!intv_list.empty() && 
      !geneList.empty())
  {
    flag_genes = true;
  } else {
    flag_genes = false;
  }

  // finalize argument parsing
  po::notify(cmd_vm);

  std::string temp_dir = conf_temp_dir + "/depth";
  create_dir(temp_dir);
  std::string output_dir = temp_dir;

  // Split Interval List and Gene List into several parts according to gatk.ncontigs:
  int ncontigs = get_config<int>("gatk.ncontigs");
  std::vector<std::string> output_files(ncontigs);
  std::vector<std::string> intv_paths(ncontigs);
  std::vector<std::string> geneList_paths(ncontigs);

  if (!geneList.empty()) {
    geneList_paths = split_by_nprocs(geneList, "list");
  }

  if (!intv_list.empty()) {
    intv_paths = split_by_nprocs(intv_list, "bed");
  }
  else {
    intv_paths = split_ref_by_nprocs(ref_path);
  }

  std::string input_file;
  if (boost::filesystem::is_directory(input_path)) {
    // Merging BAM files if the input is a folder containing PARTS BAM files:
    DLOG(INFO) << input_path << " is a directory.  Proceed to merge all BAM files";
    std::string MergeTAG = "Merge BAM Files";
    if (!sample_name.empty()) {
      MergeTAG = MergeTAG + " " + sample_name;
    }
    std::string mergeBAM = input_path + "/merge_parts.bam  ";
    Executor merger_executor(MergeTAG, get_config<int>("gatk.pr.nprocs", "gatk.nprocs"));
    Worker_ptr merger_worker(new SambambaWorker(input_path, mergeBAM, SambambaWorker::MERGE, flag_f));
    merger_executor.addTask(merger_worker);
    merger_executor.run();
  }
  else {
    input_file = input_path;
  }

  std::string DepthTAG="Depth";
  if (!sample_name.empty()) DepthTAG=DepthTAG + " " + sample_name; 
  Executor executor(DepthTAG, get_config<int>("gatk.depth.nprocs"));
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
