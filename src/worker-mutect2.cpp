#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <cmath>
#include <iomanip>
#include <map>
#include <string>

#include "fcs-genome/BackgroundExecutor.h"
#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/workers.h"

namespace fcsgenome {

int mutect2_main(int argc, char** argv,
    boost::program_options::options_description &opt_desc)
{
  namespace po = boost::program_options;

  // Define arguments
  po::variables_map cmd_vm;
  bool opt_bool = false;

  opt_desc.add_options()
    ("ref,r", po::value<std::string>()->required(), "reference genome path")
    ("normal,n", po::value<std::string>()->required(), "input normal BAM file or dir")
    ("tumor,t", po::value<std::string>()->required(), "input tumor BAM file or dir")
    ("output,o", po::value<std::string>()->required(), "output VCF file")
    ("dbsnp,d", po::value<std::vector<std::string> >(), "list of dbsnp files for Mutect2 (gatk3)")
    ("cosmic,c", po::value<std::vector<std::string> >(), "list of cosmic files for Mutect2 (gatk3)")
    ("germline,m", po::value<std::string>()->implicit_value(""), "VCF file containing annotate variant alleles by specifying a population germline resource (gatk4)")
    ("panels_of_normals,p", po::value<std::string>()->implicit_value(""), "Panels of normals VCF file")
    ("intervalList,L", po::value<std::string>()->implicit_value(""), "interval list file")
    ("normal_name,a", po::value<std::string>()->implicit_value(""), "Sample name for Normal Input BAM. Must match the SM tag in the BAM header (gatk4) ")
    ("tumor_name,b", po::value<std::string>()->implicit_value(""), "Sample name for Tumor Input BAM. Must match the SM tag in the BAM header (gatk4)")
    ("contamination_table", po::value<std::string>()->implicit_value(""), "tumor contamination table (gatk4)")
    ("filtered_vcf",po::value<std::string>()->implicit_value(""), "filtered vcf (gatk4)")
    ("sample-id", po::value<std::string>()->implicit_value(""),"sample id for log file")
    ("gatk4,g", "use gatk4 to perform analysis")
    ("filtering-extra-options", po::value<std::vector<std::string> >(), "extra options for the filtering command")
    ("skip-concat,s", "produce a set of VCF files instead of one");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc), cmd_vm);

  if (cmd_vm.count("help")) {
    throw helpRequest();
  }

  // Check configurations
  check_nprocs_config("mutect2");
  check_memory_config("mutect2");

  // Check if required arguments are presented
  bool flag_f             = get_argument<bool>(cmd_vm, "force", "f");
  bool flag_skip_concat   = get_argument<bool>(cmd_vm, "skip-concat", "s");
  bool flag_gatk          = get_argument<bool>(cmd_vm, "gatk4", "g");
  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref", "r");
  std::string normal_path = get_argument<std::string>(cmd_vm, "normal", "n");
  std::string tumor_path  = get_argument<std::string>(cmd_vm, "tumor", "t");
  std::string output_path = get_argument<std::string>(cmd_vm, "output", "o");
  std::vector<std::string> dbsnp_path = get_argument<std::vector<std::string> >(cmd_vm, "dbsnp", "d", std::vector<std::string>());
  std::vector<std::string> cosmic_path = get_argument<std::vector<std::string> >(cmd_vm, "cosmic","c", std::vector<std::string>());
  std::string germline_path = get_argument<std::string> (cmd_vm, "germline","m");
  std::string panels_of_normals = get_argument<std::string> (cmd_vm, "panels_of_normals","p");
  std::string intv_list    = get_argument<std::string>(cmd_vm, "intervalList", "L");
  std::string normal_name  = get_argument<std::string> (cmd_vm, "normal_name","a");
  std::string tumor_name   = get_argument<std::string> (cmd_vm, "tumor_name","b");
  std::string tumor_table  = get_argument<std::string> (cmd_vm, "contamination_table");
  std::string filtered_vcf = get_argument<std::string>(cmd_vm, "filtered_vcf");
  std::string sample_id    = get_argument<std::string> (cmd_vm, "sample-id");
  std::vector<std::string> extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");
  std::vector<std::string> filtering_extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "filtering-extra-options");

  // finalize argument parsing
  po::notify(cmd_vm);

  if (cmd_vm.count("germline") || cmd_vm.count("m")) {
    if (germline_path.empty()) throw pathEmpty("germline");
  }

  if (cmd_vm.count("panels_of_normals") || cmd_vm.count("p")) {
    if (panels_of_normals.empty()) throw pathEmpty("panels_of_normals");
  }

  if (cmd_vm.count("normal_name") || cmd_vm.count("a")) {
    if (normal_name.empty()) throw pathEmpty("normal_name");
  }

  if (cmd_vm.count("tumor_name") || cmd_vm.count("b")) {
    if (tumor_name.empty()) throw pathEmpty("tumor_name");
  }

  if (cmd_vm.count("contamination_table")) {
    if (tumor_table.empty()) throw pathEmpty("tumor_table");
  }

  if (cmd_vm.count("filtered_vcf")) {
    if (filtered_vcf.empty()) throw pathEmpty("filtered_vcf");
  }

  if (cmd_vm.count("sample-id") && sample_id.empty()) {
    throw pathEmpty("sample-id");
  }

  if (cmd_vm.count("intervalList") || cmd_vm.count("L")) {
    if (intv_list.empty()) throw pathEmpty("intervalList");
  }

  std::string filtered_dir;
  if (flag_gatk || get_config<bool>("use_gatk4") ) {
    if (!cosmic_path.empty()) LOG(WARNING) << "cosmic VCF file ignored in GATK4";
    if (!dbsnp_path.empty())  LOG(WARNING) << "dbSNP VCF file ignored in GATK4";
    if (normal_name.empty())  throw pathEmpty("normal_name");
    if (tumor_name.empty())   throw pathEmpty("tumor_name");
    if (filtered_vcf.empty()) throw pathEmpty("filtered_vcf");
    filtered_dir = check_output(filtered_vcf, flag_f);
    create_dir(filtered_dir);
  }

  std::string temp_dir = conf_temp_dir + "/mutect2";
  create_dir(temp_dir);

  std::string output_dir;
  if (flag_skip_concat) {
    output_dir = check_output(output_path, flag_f);
  }
  else {
    output_dir = check_output(output_path, flag_f);
  }
  std::string temp_gvcf_path = output_dir + "/" + get_basename(output_path);

  create_dir(output_dir);

  std::vector<std::string> output_files(get_config<int>("gatk.ncontigs"));
  std::vector<std::string> filtered_files(get_config<int>("gatk.ncontigs"));

  // Defining Interval File :
  std::vector<std::string> intv_paths;
  if (!intv_list.empty()) {
    intv_paths.push_back(intv_list);
  }  

  // If BAM input is a regular file, post the intervals from Reference: 
  std::vector<std::string> temp_intv;
  if (boost::filesystem::is_regular_file(normal_path) && boost::filesystem::is_regular_file(tumor_path)){
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

  BackgroundExecutor bg_executor(tag,  blaze_worker);

  Executor executor("Mutect2", get_config<int>("gatk.mutect2.nprocs"));

  bool flag_mutect2_f = !flag_skip_concat | flag_f;
  for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {

    if (boost::filesystem::is_regular_file(normal_path) && boost::filesystem::is_regular_file(tumor_path)){
      intv_paths.push_back(temp_intv[contig]);
    }
 
    std::string file_ext = "vcf";
    std::string output_file = get_contig_fname(output_dir, contig, file_ext);

    Worker_ptr mutect2_worker(new Mutect2Worker(ref_path,
	intv_paths,
	normal_path,
        tumor_path,
        output_file,
        extra_opts,
        dbsnp_path,
        cosmic_path,
        germline_path,
        panels_of_normals,
        normal_name,
        tumor_name,
        contig,
        flag_mutect2_f,
	flag_gatk)
    );
    output_files[contig] = output_file;
    executor.addTask(mutect2_worker, sample_id, contig == 0);

    if (boost::filesystem::is_regular_file(normal_path) && boost::filesystem::is_regular_file(tumor_path)){
      intv_paths.pop_back();
    }
  }

  if (flag_gatk || get_config<bool>("use_gatk4") ) {
    std::string filtered_ext = "vcf";
    for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {
       std::string filtered_file = get_contig_fname(filtered_dir, contig, filtered_ext);
       Worker_ptr mutect2Filter_worker(new Mutect2FilterWorker(
    	  intv_paths,
    	  output_files[contig],
    	  tumor_table,
    	  filtered_file,
    	  filtering_extra_opts,
          flag_f,
    	  flag_gatk)
       );
       filtered_files[contig] = filtered_file;
       executor.addTask(mutect2Filter_worker, sample_id, contig == 0);
    }
  }

  std::map<int, std::vector<std::string> > target_set;
  std::map<int, std::string> final_set;    
  if  (flag_gatk || get_config<bool>("use_gatk4")) {
    target_set = {{0, filtered_files}, {1, output_files}};  
    final_set  = {{0, filtered_vcf}, {1, output_path} };
  }
  else {
    target_set = {{0, output_files}};
    final_set  = {{0, output_path}};
  }

  for (int m=0; m<target_set.size(); ++m){

  if (!flag_skip_concat) {
    bool flag = true;
    bool flag_a = false;
    bool flag_bgzip = false;
  
    { // concat gvcfs
      Worker_ptr worker(new VCFConcatWorker(
	  target_set[m],
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
          final_set[m] + ".gz",
          flag_f)
      );
      executor.addTask(worker, sample_id, true);
    }
    { // tabix gvcf
      Worker_ptr worker(new TabixWorker(final_set[m] + ".gz")
      );
      executor.addTask(worker, sample_id, true);
    }
  }

  };

  executor.run();

  // Removing temporal data :                                                                                                                                 
  remove_path(output_dir + "/");
  if (flag_gatk || get_config<bool>("use_gatk4")) {
    remove_path(filtered_dir + "/");
  }

  return 0;
}
} // namespace fcsgenome
