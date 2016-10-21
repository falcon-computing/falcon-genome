#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <string>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/workers.h"

namespace fcsgenome {

int joint_main(int argc, char** argv,
    boost::program_options::options_description &opt_desc) 
{
  namespace po = boost::program_options;

  // Define arguments
  po::variables_map cmd_vm;

  opt_desc.add_options() 
    arg_decl_string("ref,r", "reference genome path")
    arg_decl_string("input-dir,i", "input dir containing "
                               "[sample_id].gvcf.gz files")
    arg_decl_string("output,o", "output vcf.gz file(s)")
    ("combine-only,c", "combine GVCFs only and skip genotyping")
    ("skip-combine,g", "genotype GVCFs only and skip combining "
                       "(for single sample)");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc),
      cmd_vm);

  if (cmd_vm.count("help")) { 
    throw helpRequest();
  } 

  // Check if required arguments are presented
  bool flag_f            = get_argument<bool>(cmd_vm, "force");
  bool flag_combine_only = get_argument<bool>(cmd_vm, "combine-only");
  bool flag_skip_combine = get_argument<bool>(cmd_vm, "skip-combine");

  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref",
                              get_config<std::string>("ref_genome"));
  std::string input_path  = get_argument<std::string>(cmd_vm, "input-dir");
  std::string output_path = get_argument<std::string>(cmd_vm, "output");

  // finalize argument parsing
  po::notify(cmd_vm);

  // create temp dir
  std::string parts_dir;
  if (flag_skip_combine) {
    parts_dir = input_path;
  }
  else {
    if (flag_combine_only) {
      parts_dir = output_path;
    }
    else {
      parts_dir = conf_temp_dir + "/joint/combined";
    }
    create_dir(parts_dir);
  }

  // run 
  Executor executor("Joint Genotyping", get_config<int>("gatk.joint.nprocs"));

  if (!flag_skip_combine) { // combine gvcfs
    Worker_ptr worker(new CombineGVCFsWorker(
          ref_path, input_path,
          parts_dir, flag_f));

    executor.addTask(worker);
  }
  if (!flag_combine_only) {
    std::vector<std::string> vcf_parts(get_config<int>("gatk.joint.ncontigs"));
    for (int contig = 0; 
         contig < get_config<int>("gatk.joint.ncontigs"); 
         contig++) 
    {
      Worker_ptr worker(new GenotypeGVCFsWorker(ref_path,
            get_contig_fname(parts_dir, contig, "gvcf"),
            get_contig_fname(parts_dir, contig, "vcf"),
            flag_f));
      executor.addTask(worker, contig == 0);
      vcf_parts[contig] = get_contig_fname(parts_dir, contig, "vcf");
    }

    // start concat the vcfs
    bool flag = true;
    std::string temp_vcf_path = parts_dir + "/" + get_basename(output_path);
    { // concat gvcfs
      Worker_ptr worker(new VCFConcatWorker(
            vcf_parts, temp_vcf_path,
            flag));
      executor.addTask(worker, true);
    }
    { // bgzip gvcf
      Worker_ptr worker(new ZIPWorker(
            temp_vcf_path, output_path + ".gz",
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
