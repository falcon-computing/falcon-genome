#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>

#include <cmath>
#include <iomanip>
#include <string>

#include "fcs-genome/BamFolder.h"
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
    std::string &intv_list,
    std::string sample_id,
    bool flag_f, 
    bool flag_gatk
    )
{

  init_contig_intv(ref_path);

  std::vector<std::string> intv_paths;                                                                                                                                           
  if (!intv_list.empty()) intv_paths.push_back(intv_list);  

  // temp_dir definition :
  std::string temp_dir = conf_temp_dir + "/bqsr";

  // Counting the number of parts BAM files in directory.                                                                                                                            
  // Currently, the result will be an odd number where the last BAM file                                                                                                             
  // contains the unmapped reads. The number of BAM files for analysis                                                                                                               
  // should be data.bamfiles_number-1 :     
  BamFolder bamdir(input_path);
  BamFolderInfo data = bamdir.getInfo();
  data = bamdir.merge_bed(get_config<int>("gatk.ncontigs"));
  assert(data.partsBAM.size() == data.mergedBED.size());
  if (data.bam_isdir) assert(data.partsBAM.size() == data.mergedBED.size());
  DLOG(INFO) << "BAM Dirname : " << data.bam_name;
  DLOG(INFO) << "Number of BAM files : " << data.bamfiles_number;
  DLOG(INFO) << "Number of BAI files : " << data.baifiles_number;
  DLOG(INFO) << "Number of BED files : " << data.bedfiles_number;
  if (data.bamfiles_number-1 != data.bedfiles_number && data.bamfiles_number-1 != data.baifiles_number) {
    throw std::runtime_error("Number of BAM and bai Files in are inconsistent");
  }

  std::vector<std::string> bqsr_paths(get_config<int>("gatk.ncontigs"));
  // compute bqsr for each contigs
  for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {

    if (data.bam_isdir) {
      intv_paths.push_back(data.mergedBED[contig]);
    }

    // output bqsr filename
    std::stringstream ss;
    ss << temp_dir << "/" << get_basename(output_path) << "." << contig;

    bqsr_paths[contig] = ss.str();
    DLOG(INFO) << "Task " << contig << " bqsr: " << bqsr_paths[contig];

    Worker_ptr worker(new BQSRWorker(ref_path, 
        known_sites,
    	intv_paths,
	data.partsBAM[contig],
        bqsr_paths[contig],
    	extra_opts,
    	contig, 
        flag_f, 
        flag_gatk)
    );

    executor.addTask(worker, sample_id, contig == 0);
    // Clean the vector for the next worker:
    if (data.bam_isdir) intv_paths.pop_back();
  }

  // gather bqsr for contigs
  Worker_ptr worker(new BQSRGatherWorker(bqsr_paths, output_path, flag_f, flag_gatk));

  executor.addTask(worker, sample_id, true);
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
  std::string &intv_list,
  std::string sample_id,
  bool flag_f, bool flag_gatk)
{

  init_contig_intv(ref_path);

  std::vector<std::string> intv_paths;
  if (!intv_list.empty()) intv_paths.push_back(intv_list);

  // Counting the number of parts BAM files in directory.                                                                                                                            
  // Currently, the result will be an odd number where the last BAM file                                                                                                             
  // contains the unmapped reads. The number of BAM files for analysis                                                                                                               
  // should be count-1; 
  BamFolder bamdir(input_path);
  BamFolderInfo data = bamdir.getInfo();
  data = bamdir.merge_bed(get_config<int>("gatk.ncontigs"));
  assert(data.partsBAM.size() == data.mergedBED.size());
  if (data.bam_isdir) assert(data.partsBAM.size() == data.mergedBED.size());
  DLOG(INFO) << "BAM Dirname : " << data.bam_name;
  DLOG(INFO) << "Number of BAM files : " << data.bamfiles_number;
  DLOG(INFO) << "Number of BAI files : " << data.baifiles_number;
  DLOG(INFO) << "Number of BED files : " << data.bedfiles_number;
  if (data.bamfiles_number-1 != data.bedfiles_number && data.bamfiles_number-1 != data.baifiles_number) {
    throw std::runtime_error("Number of BAM and bai Files in are inconsistent");
  }

  for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {

    if (data.bam_isdir) {
      intv_paths.push_back(data.mergedBED[contig]);
    } 

    std::string gatk_method;
    if (flag_gatk) {
      gatk_method = "ApplyBQSR "+ sample_id ;
    }
    else {
      gatk_method = "Print Reads " + sample_id ;
    }

    Worker_ptr worker(new PRWorker(ref_path,
    	intv_paths,
    	bqsr_path,
    	data.partsBAM[contig],
    	get_contig_fname(output_path, contig),
    	extra_opts,
    	contig,
    	flag_f,
       flag_gatk)
    );

    executor.addTask(worker, sample_id, contig == 0);
    // Clean the vector for the next worker:                       
    if (data.bam_isdir) intv_paths.pop_back();
  }

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
    ("sample-id", po::value<std::string>()->implicit_value(""), "sample tag for log files")
    ("intervalList,L", po::value<std::string>()->implicit_value(""), "interval list file");

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
  std::string sample_id   = get_argument<std::string>(cmd_vm, "sample-id", "");
  std::string intv_list   = get_argument<std::string>(cmd_vm, "intervalList", "L");
  std::vector<std::string> known_sites = get_argument<std::vector<std::string>>(cmd_vm, "knownSites", "K");

  std::vector<std::string> extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");

  // finalize argument parsing
  po::notify(cmd_vm);

  if (cmd_vm.count("sample-id") && sample_id.empty()) {
    throw pathEmpty("sample-id");
  }

  if (cmd_vm.count("intervalList") || cmd_vm.count("L")) {
    if (intv_list.empty()) throw pathEmpty("intervalList");
  }

  std::string temp_dir = conf_temp_dir + "/bqsr";
  create_dir(temp_dir);

  // check configurations
  check_nprocs_config("bqsr");
  check_memory_config("bqsr");

  Executor executor("Base Recalibration", get_config<int>("gatk.bqsr.nprocs"));
  baserecalAddWorkers(executor, ref_path, known_sites, extra_opts, input_path, output_path, intv_list, sample_id, flag_f, flag_gatk);

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
    ("sample-id,t", po::value<std::string>()->implicit_value(""), "sample tag for log file")
    ("intervalList,L", po::value<std::string>()->implicit_value(""), "interval list file");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc), cmd_vm);

  if (cmd_vm.count("help")) {
    throw helpRequest();
  }

  std::string temp_dir = conf_temp_dir + "/bqsr";
  create_dir(temp_dir);

  // Check if required arguments are presented
  bool flag_f             = get_argument<bool>(cmd_vm, "force", "f");
  bool flag_gatk          = get_argument<bool>(cmd_vm, "gatk4", "g");
  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref", "r");
  std::string bqsr_path   = get_argument<std::string>(cmd_vm, "bqsr", "b");
  std::string input_path  = get_argument<std::string>(cmd_vm, "input", "i");
  std::string output_path = get_argument<std::string>(cmd_vm, "output", "o");
  bool merge_bam_flag     = get_argument<bool>(cmd_vm, "merge-bam", "m");
  std::string sample_id   = get_argument<std::string>(cmd_vm, "sample-id", "t");
  std::string intv_list   = get_argument<std::string>(cmd_vm, "intervalList", "L");
  std::vector<std::string> extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");

  // finalize argument parsing
  po::notify(cmd_vm);

  if (cmd_vm.count("sample-id") && sample_id.empty()) {
    throw pathEmpty("sample-id");
  }

  if (cmd_vm.count("intervalList") || cmd_vm.count("L")) {
    if (intv_list.empty()) throw pathEmpty("intervalList");
  }

  // check configurations
  check_nprocs_config("pr");
  check_memory_config("pr");

  // the output path will be a directory
  create_dir(output_path);

  Executor executor("PrintReads", get_config<int>("gatk.pr.nprocs", "gatk.nprocs"));
  prAddWorkers(executor, ref_path, input_path, bqsr_path, output_path, extra_opts, intv_list, sample_id, flag_f, flag_gatk);

  if (merge_bam_flag){
    std::string mergeBAM(output_path);
    boost::replace_all(mergeBAM, ".bam", "_merged.bam");
    Worker_ptr merger_worker(new SambambaWorker(output_path, mergeBAM, SambambaWorker::MERGE, ".*/part-[0-9].*.*", flag_f));
    executor.addTask(merger_worker, sample_id, true);
  }

  executor.run(); 
  if (merge_bam_flag) {
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
    ("intervalList,L", po::value<std::string>()->implicit_value(""), "interval list file")
    ("sample-id",  po::value<std::string>()->implicit_value(""), "sample id for log files")
    ("extra-options-pr", po::value<std::vector<std::string> >(), "extra options for Printreads (ApplyBQSR)")
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
  std::string intv_list   = get_argument<std::string>(cmd_vm, "intervalList", "L");
  std::string sample_id   = get_argument<std::string>(cmd_vm, "sample-id");
  bool merge_bam_flag     = get_argument<bool>(cmd_vm, "merge-bam", "m");

  std::vector<std::string> extra_opts_baserecal = get_argument<std::vector<std::string>>(cmd_vm, "extra-options","O");
  std::vector<std::string> extra_opts_pr = get_argument<std::vector<std::string>>(cmd_vm, "extra-options-pr");

  if (cmd_vm.count("sample-id") && sample_id.empty()) {
    throw pathEmpty("sample-id");
  }

  if (cmd_vm.count("intervalList") || cmd_vm.count("L")) {
    if (intv_list.empty()) throw pathEmpty("intervalList");
  }

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
  
  Executor executor("BQSR", get_config<int>("gatk.bqsr.nprocs"));
  // first, do base recal
  baserecalAddWorkers(executor, ref_path, known_sites, extra_opts_baserecal, input_path, bqsr_path, intv_list, sample_id, flag_f, flag_gatk);
  prAddWorkers(executor, ref_path, input_path, bqsr_path, output_path, extra_opts_pr, intv_list, sample_id, flag_f, flag_gatk);

  // Merge BAM Files if --merge-bam is set:
  if (merge_bam_flag) {
    std::string mergeBAM(output_path);
    boost::replace_all(mergeBAM, ".bam", "_merged.bam");
    Worker_ptr merger_worker(new SambambaWorker(output_path, mergeBAM, SambambaWorker::MERGE, ".*/part-[0-9].*.*", flag_f));
    executor.addTask(merger_worker, sample_id, true);
  }

  executor.run();

  removePartialBQSR(bqsr_path);
  if (merge_bam_flag) {
    remove_path(output_path);
  }

  if (delete_bqsr) {
    remove_path(bqsr_path);
  }

  return 0;
}
} // namespace fcsgenome
