#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>

#include <cmath>
#include <iomanip>
#include <string>

#include "fcs-genome/BackgroundExecutor.h"
#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/workers.h"

namespace fcsgenome {

int variant_filtration_main(int argc, char** argv,
    boost::program_options::options_description &opt_desc)
{
  namespace po = boost::program_options;

  // Define arguments
  po::variables_map cmd_vm;
  bool opt_bool = false;

  opt_desc.add_options()
    ("ref,r", po::value<std::string>()->required(), "reference genome path")
    ("input,i", po::value<std::string>()->required(), "input VCF filename")
    ("output,o", po::value<std::string>()->required(), "output Filtered VCF filename")
    ("intervalList,L", po::value<std::string>()->implicit_value(""), "interval list file")
    ("filteringExpression", po::value<std::string>()->implicit_value(""), "parameters used to filter variants")
    ("filter_name", po::value<std::string>()->implicit_value(""), "filter name for log files")
    ("sample-id", po::value<std::string>()->implicit_value(""), "sample tag for log files")
    ("gatk4,g", "use gatk4 to perform analysis");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc), cmd_vm);

  if (cmd_vm.count("help")) {
    throw helpRequest();
  }

  // check configurations
  check_nprocs_config("htc");
  check_memory_config("htc");

  // Check if required arguments are presented
  bool flag_f             = get_argument<bool>(cmd_vm, "force", "f");
  bool flag_gatk          = get_argument<bool>(cmd_vm, "gatk4", "g");
  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref", "r");
  std::string input_path  = get_argument<std::string>(cmd_vm, "input", "i");
  std::string output_path = get_argument<std::string>(cmd_vm, "output", "o");
  std::string filter_name = get_argument<std::string>(cmd_vm, "filter_name");
  std::string filter_par  = get_argument<std::string>(cmd_vm, "filteringExpression");
  std::string sample_id   = get_argument<std::string>(cmd_vm, "sample-id");
  std::string intv_list   = get_argument<std::string>(cmd_vm, "intervalList", "L");
  std::vector<std::string> extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");

  // finalize argument parsing
  po::notify(cmd_vm);

  if (cmd_vm.count("filter_name") && filter_name.empty()) {
    throw pathEmpty("filter_name");
  }

  if (cmd_vm.count("filteringExpression") && filter_par.empty()) {
    throw pathEmpty("filteringExpression");
  }

  if (cmd_vm.count("sample-id") && sample_id.empty()) {
    throw pathEmpty("sample-id");
  }
  
  if (cmd_vm.count("intervalList") || cmd_vm.count("L")) {
    if (intv_list.empty()) throw pathEmpty("intervalList");
  }

  std::string temp_dir = conf_temp_dir + "/vcf_filtered";

  std::string output_dir;
  output_dir = temp_dir;

  std::string temp_gvcf_path = output_dir + "/" + get_basename(output_path);
  create_dir(output_dir);

  std::vector<std::string> output_files(get_config<int>("gatk.ncontigs"));
  std::vector<std::string> intv_paths;
  if (!intv_list.empty()) {
    intv_paths = split_by_nprocs(intv_list, "bed");
  }
  else {
    intv_paths = init_contig_intv(ref_path);
  }

  Executor executor("Variants Filtration", get_config<int>("gatk.htc.nprocs", "gatk.nprocs"));

  //bool flag_vcfFilter_f = !flag_skip_concat || flag_f;
  for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {
    std::string input_file;
    if (boost::filesystem::is_directory(input_path)) {
      input_file = get_contig_fname(input_path, contig);
    }
    else {
      input_file = input_path;
    }

    std::string file_ext = "vcf";
    //if (!flag_vcf) {
    //  file_ext = "g." + file_ext;
    //}

    std::string output_file = get_contig_fname(output_dir, contig, file_ext);
    Worker_ptr worker(new VariantsFilterWorker(ref_path,
        intv_paths[contig], 
        input_file,
        output_file,
        extra_opts,
        contig,
	filter_par,
	filter_name,
        flag_f,
	flag_gatk)
    );

    output_files[contig] = output_file;
    executor.addTask(worker,sample_id);
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
