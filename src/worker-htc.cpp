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
    ("output,o", po::value<std::string>()->required(), "output GVCF/VCF file (if --skip-concat is set"
                                "the output will be a directory of gvcf files)")
    ("produce-vcf,v", "produce VCF files from HaplotypeCaller instead of GVCF")
    // TODO: skip-concat should be deprecated
    ("intervalList,L", po::value<std::vector<std::string> >(), "interval list file")
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
  bool flag_skip_concat   = get_argument<bool>(cmd_vm, "skip-concat", "s");
  bool flag_vcf           = get_argument<bool>(cmd_vm, "produce-vcf", "v");
  bool flag_gatk          = get_argument<bool>(cmd_vm, "gatk4", "g");
  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref", "r");
  std::string input_path  = get_argument<std::string>(cmd_vm, "input", "i");
  std::string output_path = get_argument<std::string>(cmd_vm, "output", "o");
  std::vector<std::string> intv_list = get_argument<std::vector<std::string> >(cmd_vm, "intervalList", "L");
  std::vector<std::string> extra_opts =
          get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");

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

  std::map<int, std::vector<std::string>> intv_sets;
  if (!intv_list.empty() && !boost::filesystem::is_directory(input_path)){
      for (int i = 0; i < intv_list.size(); i++) {
          std::vector<std::string> temp_intv = split_by_nprocs(intv_list[i], "bed", i);
          for (int k = 0; k < temp_intv.size(); k++) {
               if (i==0){
                   std::vector<std::string> ivect;
                   ivect.push_back(temp_intv[k]);
                   intv_sets.insert(std::make_pair(k, ivect ) );
                   ivect.clear();
               }else{
                   intv_sets[k].push_back(temp_intv[k]);
               }
          }
          temp_intv.clear();
      }
  }

  std::vector<std::string> intv_paths = init_contig_intv(ref_path);

  // start an executor for NAM
  Worker_ptr blaze_worker(new BlazeWorker(
        get_config<std::string>("blaze.nam_path"),
        get_config<std::string>("blaze.conf_path")));

  BackgroundExecutor bg_executor("blaze-nam", blaze_worker);

  Executor executor("Haplotype Caller", get_config<int>("gatk.htc.nprocs", "gatk.nprocs"));

  bool flag_htc_f = !flag_skip_concat || flag_f;
  for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {
    std::string input_file;
    std::vector <std::string> IntervalFiles;
    if (boost::filesystem::is_directory(input_path)) {
      // if input is a directory, automatically go into contig mode
      // TODO: here should validate if the bam files in folder
      //       is generated by fcs-genome with the same num contigs
      input_file = get_contig_fname(input_path, contig);
      IntervalFiles=intv_list;
    }
    else {
      input_file = input_path;
      if (!intv_list.empty()){
        IntervalFiles=intv_sets.find(contig)->second;
      }
    }

    std::string file_ext = "vcf";
    if (!flag_vcf) {
      file_ext = "g." + file_ext;
    }

    std::string output_file = get_contig_fname(output_dir, contig, file_ext);
    Worker_ptr worker(new HTCWorker(ref_path,
          intv_paths[contig], input_file,
          output_file,
          extra_opts,
          IntervalFiles,
          contig,
          flag_vcf,
          flag_htc_f,
          flag_gatk)
    );

    IntervalFiles.clear();
    output_files[contig] = output_file;

    executor.addTask(worker);
  }
  
  if (!flag_skip_concat) {

    bool flag = true;
    bool flag_a = false;
    { // concat gvcfs
      Worker_ptr worker(new VCFConcatWorker(
            output_files, temp_gvcf_path,
            flag_a, flag));
      executor.addTask(worker, true);
    }
    //{ // sort gvcf
    //  Worker_ptr worker(new VCFSortWorker(temp_gvcf_path));
    //  executor.addTask(worker, true);
    //}
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
