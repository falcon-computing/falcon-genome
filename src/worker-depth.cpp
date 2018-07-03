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
    arg_decl_string("ref,r", "reference genome path")
    arg_decl_string("input,i", "input BAM file")
    arg_decl_string("output,o", "output coverage file")
    arg_decl_string("intervalList,L", "Interval List BED File")
    arg_decl_string("geneList,g", "list of genes over which to calculate coverage")
    ("depthCutoff,d", po::value<int>()->default_value(15), "Cutoff for coverage depth summary")
    ("baseCoverage,b", "Calculate coverage depth of each base")
    ("intervalCoverage,v", "Calculate Coverage Summary of given intervals")
    ("sampleSummary,s", "Output Summary Files for each sample");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc),
      cmd_vm);

  if (cmd_vm.count("help")) {
    throw helpRequest();
  }

  // Check configurations
  check_nprocs_config("depth");
  check_memory_config("depth");

  // Check if required arguments are presented
  bool flag_f             = get_argument<bool>(cmd_vm, "force");
  bool flag_baseCoverage    = get_argument<bool>(cmd_vm, "baseCoverage");
  bool flag_intervalCoverage = get_argument<bool>(cmd_vm, "intervalCoverage");
  bool flag_sampleSummary = get_argument<bool>(cmd_vm, "sampleSummary");
  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref",get_config<std::string>("ref_genome"));
  std::string input_path = get_argument<std::string>(cmd_vm, "input");
  std::string output_path = get_argument<std::string>(cmd_vm, "output");
  std::string intv_list = get_argument<std::string>(cmd_vm, "intervalList");
  std::string geneList = get_argument<std::string>(cmd_vm, "geneList");
  int depthCutoff = get_argument<int>(cmd_vm, "depthCutoff");
  std::vector<std::string> extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "extra-options");

  // finalize argument parsing
  po::notify(cmd_vm);

  //std::string temp_dir = conf_temp_dir + "/depth";
  std::string temp_dir = "/genome/disk2/alfonso/depth";
  create_dir(temp_dir);

  //output path
  std::string output_dir;
  //output_dir = check_output(output_path, flag_f);
  output_dir = temp_dir;
  create_dir(output_dir);
  std::string temp_depth_path = output_dir + "/" + get_basename(output_path);

  // Split Interval List and Gene List into several parts according to gatk.ncontigs:
  std::vector<std::string> output_files(get_config<int>("gatk.ncontigs"));
  std::vector<std::string> intv_paths = split_by_nprocs(intv_list, "bed");
  std::vector<std::string> geneList_paths = split_by_nprocs(geneList, "list");

  std::vector<std::string>::const_iterator i1;
  std::vector<std::string>::const_iterator i2;
  for( i1 = intv_paths.begin(), i2 = geneList_paths.begin(); i1 < intv_paths.end() && i2 < geneList_paths.end();
     ++i1, ++i2 ){
     DLOG(INFO) << *i1 << std::endl;
     DLOG(INFO) << *i2 << std::endl;
  }

  //exit(0);


  Executor executor("Depth", get_config<int>("gatk.depth.nprocs"));

  for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {
    std::string input_file;
    if (boost::filesystem::is_directory(input_path)) {
      // if input is a directory, automatically go into contig mode
      input_file = get_contig_fname(input_path, contig);
    }
    else {
      input_file = input_path;
    }




    std::string file_ext = "cov";
    std::string output_file = get_contig_fname(output_dir, contig, file_ext);
    Worker_ptr worker(new DepthWorker(ref_path,
          intv_paths,
          input_file,
          output_file,
          geneList_paths,
          depthCutoff,
          extra_opts,
          contig,
          flag_f,
          flag_baseCoverage,
          flag_intervalCoverage,
          flag_sampleSummary));
    output_files[contig] = output_file;
    DLOG(INFO) << "Processing " << contig << " " input_file << " " << output_file << " " << std::endl;
    //executor.addTask(worker);
  }

  exit(0);


  bool flag = true;
  //bool flag_a = false;

  Worker_ptr worker(new DepthCombineWorker(
        output_files, output_path,
        flag_baseCoverage, flag_intervalCoverage, flag_sampleSummary, flag));
  executor.addTask(worker, true);

  executor.run();

  return 0;
}
} // namespace fcsgenome
