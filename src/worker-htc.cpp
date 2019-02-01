#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>

#include <assert.h>
#include <cmath>
#include <iomanip>
#include <string>

#include "fcs-genome/BackgroundExecutor.h"
#include "fcs-genome/BamInput.h"
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
  bool opt_bool = false;

  opt_desc.add_options()
    ("ref,r", po::value<std::string>()->required(), "reference genome path")
    ("input,i", po::value<std::string>()->required(), "input BAM file or dir")
    ("output,o", po::value<std::string>()->required(), "output GVCF/VCF file")
    ("produce-vcf,v", "produce VCF files from HaplotypeCaller instead of GVCF")
    ("intervalList,L", po::value<std::string>()->implicit_value(""), "interval list file")
    ("sample-id", po::value<std::string>()->implicit_value(""), "sample id for log files")
    ("skip-concat,s", "(deprecated) produce a set of GVCF/VCF files instead of one")
    ("gatk4,g", "use gatk4 to perform analysis");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc),
      cmd_vm);

  if (cmd_vm.count("help")) {
    throw helpRequest();
  }

  // check configurations
  check_nprocs_config("htc");
  check_memory_config("htc");

  // Check if required arguments are presented
  bool flag_f             = get_argument<bool>(cmd_vm, "force", "f");
  bool flag_vcf           = get_argument<bool>(cmd_vm, "produce-vcf", "v");
  bool flag_gatk          = get_argument<bool>(cmd_vm, "gatk4", "g");
  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref", "r");
  std::string input_path  = get_argument<std::string>(cmd_vm, "input", "i");
  std::string output_path = get_argument<std::string>(cmd_vm, "output", "o");
  std::string sample_id   = get_argument<std::string>(cmd_vm, "sample-id");
  std::string intv_list   = get_argument<std::string>(cmd_vm, "intervalList", "L");
  std::vector<std::string> extra_opts =
          get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");

  // finalize argument parsing
  po::notify(cmd_vm);

  if (cmd_vm.count("sample-id") && sample_id.empty()) {
    throw pathEmpty("sample-id");
  }

  if (cmd_vm.count("intervalList") || cmd_vm.count("L")) { 
    if (intv_list.empty()) throw pathEmpty("intervalList");
  }

  std::string temp_dir = conf_temp_dir + "/htc";

  // TODO: deal with the case where
  // 1. output_path is a dir but should not be deleted
  // 2. output_path is a file
  std::string output_dir;
  //output_dir = check_output(output_path, flag_f);
  output_dir = temp_dir;

  std::string temp_gvcf_path = output_dir + "/" + get_basename(output_path);

  create_dir(output_dir);
  std::vector<std::string> output_files(get_config<int>("gatk.ncontigs"));

  // Defining Interval File : 
  std::vector<std::string> intv_paths;
  if (!intv_list.empty()) {
   intv_paths.push_back(intv_list);
  }

  // If BAM input is a regular file, post the intervals from Reference:
  std::vector<std::string> temp_intv;
  if (boost::filesystem::is_regular_file(input_path)){
    temp_intv=init_contig_intv(ref_path);
  }

  // start an executor for NAM
  Worker_ptr blaze_worker(new BlazeWorker(
        get_config<std::string>("blaze.nam_path"),
        get_config<std::string>("blaze.conf_path")));

  std::string tag;
  if (!sample_id.empty()) {
    tag = "blaze-nam-" + sample_id;
  }
  else {
    tag = "blaze-nam";
  }

  BackgroundExecutor bg_executor(tag, blaze_worker);  
  Executor executor("Haplotype Caller", get_config<int>("gatk.htc.nprocs", "gatk.nprocs"));

  bool flag_htc_f = flag_f;
  for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {

    std::string file_ext = "vcf";                                                                                                                                                  
    if (!flag_vcf) {                                                                                                                                                               
      file_ext = "g." + file_ext;                                                                                                                                                  
    }                             

    if (boost::filesystem::is_regular_file(input_path)){
      intv_paths.push_back(temp_intv[contig]);
    } 

    std::string output_file = get_contig_fname(output_dir, contig, file_ext);
    Worker_ptr worker(new HTCWorker(ref_path,
       intv_paths,
       input_path,
       output_file,
       extra_opts,
       contig,
       flag_vcf,
       flag_htc_f,
       flag_gatk)
    );
 
    output_files[contig] = output_file;
    executor.addTask(worker,sample_id);
    if (boost::filesystem::is_regular_file(input_path)){
      intv_paths.pop_back();
    }

  }

  bool flag = true;
  bool flag_a = false;
  bool flag_bgzip = false;

  std::string process_tag;

  { // concat gvcfs
    Worker_ptr worker(new VCFConcatWorker(
        output_files, 
        temp_gvcf_path,
        flag_a, 
        flag_bgzip,
        flag)
    );
    executor.addTask(worker, sample_id, true);
  }
  { // bgzip gvcf
    Worker_ptr worker(new ZIPWorker(
        temp_gvcf_path, 
        output_path+".gz",
        flag_f)
    );
    executor.addTask(worker, sample_id, true);
  }
  { // tabix gvcf
    Worker_ptr worker(new TabixWorker(
        output_path + ".gz") 
    );
    executor.addTask(worker, sample_id, true);
  }

  executor.run();

  return 0;
}
} // namespace fcsgenome
