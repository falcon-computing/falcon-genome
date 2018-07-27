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
    //("tumor,t", po::value<std::string>()->required(), "input tumor BAM file or dir")
    ("tumor,t", po::value<std::string>(), "input tumor BAM file or dir")
    ("output,o", po::value<std::string>()->required(), "output VCF file")
    //("dbsnp,d", po::value<std::vector<std::string> >(), "list of dbsnp files for Mutect2")
    //("cosmic,c", po::value<std::vector<std::string> >(), "list of cosmic files for Mutect2")
    //("intervalList,L", po::value<std::vector<std::string> >(), "interval list file")
    ("dbsnp,d", po::value<std::string>(), "list of dbsnp files for Mutect2")
    ("cosmic,c", po::value<std::string>(), "list of cosmic files for Mutect2")
    ("intervalList,L", po::value<std::string>(), "interval list file")
    ("skip-concat,s", "produce a set of VCF files instead of one");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc),
      cmd_vm);

  if (cmd_vm.count("help")) {
    throw helpRequest();
  }

  // Check configurations
  check_nprocs_config("mutect2");
  check_memory_config("mutect2");

  // Check if required arguments are presented
  bool flag_f             = get_argument<bool>(cmd_vm, "force", "f");
  bool flag_skip_concat    = get_argument<bool>(cmd_vm, "skip-concat", "s");
  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref", "r");
  std::string normal_path = get_argument<std::string>(cmd_vm, "normal", "n");
  std::string tumor_path = get_argument<std::string>(cmd_vm, "tumor", "t");
  std::string output_path = get_argument<std::string>(cmd_vm, "output", "o");
  std::vector<std::string> dbsnp_path = get_argument<std::vector<std::string> >(cmd_vm, "dbsnp", "d", std::vector<std::string>());
  std::vector<std::string> cosmic_path = get_argument<std::vector<std::string> >(cmd_vm, "cosmic","c", std::vector<std::string>());
  std::vector<std::string> intv_list = get_argument<std::vector<std::string> >(cmd_vm, "intervalList", "L");
  std::vector<std::string> extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");

  // finalize argument parsing
  po::notify(cmd_vm);

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
  //std::vector<std::string> intv_paths = init_contig_intv(ref_path);
  std::vector<std::string> intv_paths;

  std::map<size_t, std::vector<std::string> > dbSNP_sets;
  std::map<size_t, std::vector<std::string> > cosmic_sets;

  if (!intv_list.empty()) {
     DLOG(INFO) << "Interval list defined";
     intv_paths = split_by_nprocs(intv_list, "bed");
  } else {
     DLOG(INFO) << "Interval list not defined. Coordinates from Reference will be used for MuTect2";
     intv_paths = split_ref_by_nprocs(ref_path);
  }

  if (!dbsnp_path.empty()){
      std::vector<std::string> tmp_dnsp;
      for (size_t i = 0; i < dbsnp_path.size(); i++){
           tmp_dnsp = split_vcf_by_nprocs(dbsnp_path[i], intv_paths, "vcf");
           dbSNP_sets.insert(pair<size_t, std:vector<std::string>>(i,tmp_dbsnp));
           tmp_dbsnp.clear();
      }

  }

  if (!cosmic_path.empty()){
      std::vector<std::string> tmp_cosmic;

  }



  // start an executor for NAM
  Worker_ptr blaze_worker(new BlazeWorker(
        get_config<std::string>("blaze.nam_path"),
        get_config<std::string>("blaze.conf_path")));

  BackgroundExecutor bg_executor("blaze-nam", blaze_worker);

  Executor executor("Mutect2", get_config<int>("gatk.mutect2.nprocs"));

  bool flag_mutect2_f = !flag_skip_concat | flag_f;
  for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {
    std::string normal_file;
    std::string tumor_file;
    if (boost::filesystem::is_directory(normal_path)) {
      // if input is a directory, automatically go into contig mode
      normal_file = get_contig_fname(normal_path, contig);
    }
    else {
      normal_file = normal_path;
    }
    if (boost::filesystem::is_directory(tumor_path)) {
      tumor_file = get_contig_fname(tumor_path, contig);
    }
    else {
      tumor_file = tumor_path;
    }
    std::string file_ext = "vcf";
    std::string output_file = get_contig_fname(output_dir, contig, file_ext);
    Worker_ptr worker(new Mutect2Worker(ref_path,
          intv_paths[contig], normal_file, tumor_file,
          output_file,
          extra_opts,
          dbsnp_path,
          cosmic_path,
          intv_list,
          contig,
          flag_mutect2_f));
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
