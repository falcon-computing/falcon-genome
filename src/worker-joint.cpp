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
    ("ref,r", po::value<std::string>()->required(), "reference genome path")
    ("input-dir,i", po::value<std::string>()->required(), "input dir containing "
                               "[sample_id].gvcf.gz files")
    ("output,o", po::value<std::string>()->required(), "output vcf.gz file(s)")
    ("sample-id", po::value<std::string>(), "sample id for log files")
    ("database_name", po::value<std::string>()->implicit_value(""), "database name (gatk4 only)")
    ("combine-only,c", "combine GVCFs only and skip genotyping")
    ("skip-combine", "(deprecated) perform genotype GVCFs only and skip combine GVCFs")
    ("gatk4,g", "use gatk4 to perform analysis");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc), cmd_vm);

  if (cmd_vm.count("help")) { 
    throw helpRequest();
  } 

  // Check if required arguments are presented
  bool flag_f            = get_argument<bool>(cmd_vm, "force", "f");
  bool flag_combine_only = get_argument<bool>(cmd_vm, "combine-only", "c");
  bool flag_skip_combine = get_argument<bool>(cmd_vm, "skip-combine");
  bool flag_gatk         = get_argument<bool>(cmd_vm, "gatk4","g");

  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref", "r");
  std::string input_path  = get_argument<std::string>(cmd_vm, "input-dir", "i");
  std::string output_path = get_argument<std::string>(cmd_vm, "output", "o");
  std::string sample_id   = get_argument<std::string>(cmd_vm, "sample-id");
  std::string database_name  = get_argument<std::string>(cmd_vm, "database_name");
  std::vector<std::string> extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");

  if (flag_gatk || get_config<bool>("use_gatk4") ) {
    if (database_name.empty()) throw pathEmpty("database_name");
  }

  po::notify(cmd_vm);

  LOG(INFO) << "I am database " << database_name;
  LOG(INFO) << "I am GATK4 " << flag_gatk;
  if (flag_gatk || get_config<bool>("use_gatk4") ) {
    if (cmd_vm.count("database_name")==0) throw pathEmpty("database_name");
  }
  
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

  Executor executor("Joint Genotyping", get_config<int>("gatk.genotype.nprocs"));

  if (flag_gatk || get_config<bool>("use_gatk4")) {

    Worker_ptr worker1(new CombineGVCFsWorker(
       ref_path,
       input_path,
       parts_dir,
       database_name,
       flag_f,
       flag_gatk)
    );
    executor.addTask(worker1, sample_id, 0);

    Worker_ptr worker2(new GenotypeGVCFsWorker(
       ref_path,
       database_name,
       output_path,
       extra_opts,
       flag_f,
       flag_gatk)
    );
    executor.addTask(worker2, sample_id, true);   
  }
  else {
    LOG(INFO) << "I am here";

    // combine gvcfs
    if (!flag_skip_combine) {
      Worker_ptr worker(new CombineGVCFsWorker(
         ref_path, 
         input_path,
         parts_dir, 
         database_name,
         flag_f,
         flag_gatk)
      );
      executor.addTask(worker, sample_id, true);
   
      // tabix gvcf from combine gvcf output
      for (int contig = 0; contig < get_config<int>("gatk.joint.ncontigs"); contig++) { 
        Worker_ptr worker(new TabixWorker(get_contig_fname(parts_dir, contig, "gvcf.gz")));
        executor.addTask(worker, sample_id, contig == 0);
      }
    } // flag_skip_combine checked

    if (!flag_combine_only) {
      std::vector<std::string> vcf_parts(get_config<int>("gatk.joint.ncontigs"));
 
      // call gatk genotype gvcfs on each combined gvcf partitions
      for (int contig = 0; contig < get_config<int>("gatk.joint.ncontigs"); contig++) {
         std::string suffix = "gvcf.gz";
         if (flag_skip_combine) {
           suffix = "gvcf";
         }  
         Worker_ptr worker(new GenotypeGVCFsWorker(
            ref_path,
            get_contig_fname(parts_dir, contig, suffix),
            get_contig_fname(parts_dir, contig, "vcf"),
            extra_opts,
	    flag_f,
	    flag_gatk)
         );
         executor.addTask(worker, sample_id, contig == 0);
         vcf_parts[contig] = get_contig_fname(parts_dir, contig, "vcf");
      }

      for (int contig = 0; contig < get_config<int>("gatk.joint.ncontigs"); contig++){
        Worker_ptr worker(new ZIPWorker(
           vcf_parts[contig], 
           vcf_parts[contig] + ".gz", 
           flag_f)
        );
        executor.addTask(worker, sample_id, contig == 0);
      }

      for (int contig = 0; contig < get_config<int>("gatk.joint.ncontigs"); contig++) {
        Worker_ptr worker(new TabixWorker(
	    vcf_parts[contig] + ".gz")
        );
        executor.addTask(worker, sample_id, contig == 0);
      }

      {
        for (auto& vcf_part : vcf_parts)
           vcf_part += ".gz";
      }
    
      // start concat the vcfs
      bool flag = true;
      bool flag_a = true;
      bool flag_bgzip = false;
      std::string temp_vcf_path = parts_dir + "/" + get_basename(output_path);
      // concat vcfs
      { 
        Worker_ptr worker(new VCFConcatWorker(
           vcf_parts, 
           temp_vcf_path,
           flag_a, 
           flag_bgzip,
           flag)
        );
        executor.addTask(worker, sample_id, true);
      }
      // bgzip vcf
      { 
        Worker_ptr worker(new ZIPWorker(
           temp_vcf_path, 
           output_path + ".gz",
           flag_f)
        );
        executor.addTask(worker, sample_id, true);
      }
      // tabix vcf
      { 
        Worker_ptr worker(new TabixWorker(
	    output_path + ".gz")
        );
        executor.addTask(worker, sample_id, true);
       }
   } // flag_combine_only

  } // Check if gatk3 or gatk4 is used
  executor.run();

  return 0;
}
} // namespace fcsgenome
