#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <string>

#include <bits/stdc++.h> 

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
    ("sample-tag,t", po::value<std::string>(), "tag for log files")
    ("combine-only,c", "combine GVCFs only and skip genotyping")
    ("skip-combine,g", "(deprecated) perform genotype GVCFs only "
                       "and skip combine GVCFs");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc), cmd_vm);

  if (cmd_vm.count("help")) { 
    throw helpRequest();
  } 

  // Check if required arguments are presented
  bool flag_f            = get_argument<bool>(cmd_vm, "force", "f");
  bool flag_combine_only = get_argument<bool>(cmd_vm, "combine-only", "c");
  bool flag_skip_combine = get_argument<bool>(cmd_vm, "skip-combine", "g");

  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref", "r");
  std::string input_path  = get_argument<std::string>(cmd_vm, "input-dir", "i");
  std::string output_path = get_argument<std::string>(cmd_vm, "output", "o");
  std::string sample_tag  = get_argument<std::string>(cmd_vm, "sample-tag", "t");
  std::vector<std::string> extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");

  LOG(INFO) << "input_path " << input_path;

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

  std::vector<std::string> stage_levels{"Joint Combine Input GVCF"};
  //Executor* executor = create_executor("Joint Genotyping", stage_levels, sample_tag, get_config<int>("gatk.genotype.nprocs"));
  Executor executor("Joint Genotyping", stage_levels, sample_tag, get_config<int>("gatk.genotype.nprocs"));

  // Worker_ptr worker(new CombineGVCFsWorker(
  //Worker_ptr worker(new TabixWorker(

  //Worker_ptr worker(new GenotypeGVCFsWorker(ref_path,
  //Worker_ptr worker(new ZIPWorker(
  //Worker_ptr worker(new TabixWorker(
  //Worker_ptr worker(new VCFConcatWorker(
  //Worker_ptr worker(new ZIPWorker(																								   //Worker_ptr worker(new TabixWorker(

  if (!flag_skip_combine) {
    
    stage_levels.push_back("Joint Generate Parts VCF Index");
  }

  if (!flag_combine_only) {
    ;
  // stage_levels.push_back("Joint Genotype GVCF");
  // stage_levels.push_back("Joint Compress VCF");
  // stage_levels.push_back("Joint Generate compressed VCF Index");
  // stage_levels.push_back("Joint Concatenate compressed VCF");
  // stage_levels.push_back("Joint Compress VCF");
  // stage_levels.push_back("Joint Generate VCF Index");
  }

  std::string tag;

  // combine gvcfs
  if (!flag_skip_combine) {

    LOG(INFO) << "I am flag_skip_combine";
    if (!sample_tag.empty()){
      tag = "Joint Combine Input VCF " + sample_tag;
    }
    else {
      tag = "Joint Combine Input VCF";
    }    
    Worker_ptr worker(new CombineGVCFsWorker(
        ref_path, 
        input_path,
        parts_dir, 
        flag_f)
    );
    executor.addTask(worker, tag, true);

    LOG(INFO) << "parts_dir " << parts_dir;
 
    if (!sample_tag.empty()){
      tag = "Joint Generate Parts VCF Index " + sample_tag;
    }
    else {
      tag = "Joint Generate Parts VCF Index";
    }
 
    // tabix gvcf from combine gvcf output
    for (int contig = 0; contig < get_config<int>("gatk.joint.ncontigs"); contig++) { 
       Worker_ptr worker(new TabixWorker(get_contig_fname(parts_dir, contig, "gvcf.gz")));
       executor.addTask(worker, tag, contig == 0);
    }
  } // flag_skip_combine checked


  if (!flag_combine_only) {
    ;

//  
//     LOG(INFO) << "I am in flag_combine_only";
//     std::vector<std::string> vcf_parts(get_config<int>("gatk.joint.ncontigs"));
//  
//     if (!sample_tag.empty()){
//       tag = "Joint Genotype VCF " + sample_tag;
//     }
//     else {
//       tag = "Joint Genotype VCF";
//     }
//  
//     // call gatk genotype gvcfs on each combined gvcf partitions
//     for (int contig = 0; contig < get_config<int>("gatk.joint.ncontigs"); contig++) {
//       std::string suffix = "gvcf.gz";
//       if (flag_skip_combine) {
//         suffix = "gvcf";
//       }
//  
//       Worker_ptr worker(new GenotypeGVCFsWorker(ref_path,
//           get_contig_fname(parts_dir, contig, suffix),
//           get_contig_fname(parts_dir, contig, "vcf"),
//           extra_opts,
//           flag_f)
//       );
//  
//       executor.addTask(worker, tag, contig == 0);
//       vcf_parts[contig] = get_contig_fname(parts_dir, contig, "vcf");
//     }
//
//   for (int contig = 0; contig < get_config<int>("gatk.joint.ncontigs"); contig++){
//     if (!sample_tag.empty()){
//	tag = "Joint Compress VCF " + sample_tag;
//     }
//     else {
//	tag = "Joint Compress VCF";
//     }
//
//     Worker_ptr worker(new ZIPWorker(
//         vcf_parts[contig], 
//         vcf_parts[contig] + ".gz", 
//         flag_f)
//     );
//     executor.addTask(worker, tag, contig == 0);
//   }
//
//   if (!sample_tag.empty()){
//     tag = "Joint Generate VCF Index " + sample_tag;
//   }
//   else {
//     tag = "Joint Generate VCF Index";
//   }
//
//   for (int contig = 0; contig < get_config<int>("gatk.joint.ncontigs"); contig++) {
//     Worker_ptr worker(new TabixWorker(
//         vcf_parts[contig] + ".gz")
//     );
//     executor.addTask(worker, tag, contig == 0);
//   }
//
//   {
//     for (auto& vcf_part : vcf_parts)
//       vcf_part += ".gz";
//   }
//     
//   // start concat the vcfs
//   bool flag = true;
//   bool flag_a = true;
//   bool flag_bgzip = false;
//   std::string temp_vcf_path = parts_dir + "/" + get_basename(output_path);
//   // concat vcfs
//   { 
//     if (!sample_tag.empty()){
//	tag = "Joint Concatenate VCF " + sample_tag;
//     }
//     else {
//	tag = "Joint Concatenate VCF";
//     }
//     Worker_ptr worker(new VCFConcatWorker(
//         vcf_parts, 
//         temp_vcf_path,
//         flag_a, 
//         flag_bgzip,
//         flag)
//     );
//     executor.addTask(worker, tag, true);
//   }
//   // bgzip vcf
//   { 
//     if (!sample_tag.empty()){
//       tag = "Joint Compress VCF " + sample_tag;
//     }
//     else {
//       tag = "Joint Compress VCF";
//     }
//     Worker_ptr worker(new ZIPWorker(
//         temp_vcf_path, 
//         output_path + ".gz",
//         flag_f)
//     );
//     executor.addTask(worker, tag, true);
//   }
//   // tabix vcf
//   { 
//     if (!sample_tag.empty()){
//       tag = "Joint Generate VCF Index " + sample_tag;
//     }
//     else {
//       tag = "Joint Generate VCF Index";
//     }
//     Worker_ptr worker(new TabixWorker(
//         output_path + ".gz")
//     );
//     executor.addTask(worker, tag, true);
//   }
   } // flag_combine_only

   executor.run();

  return 0;
}
} // namespace fcsgenome
