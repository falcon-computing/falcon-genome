#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <string>

#include "fcs-genome/BackgroundExecutor.h"
#include "fcs-genome/BamInput.h"
#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/workers.h"
#include "fcs-genome/SampleSheet.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>

namespace fcsgenome {

int germline_main(int argc, char** argv, boost::program_options::options_description &opt_desc)
{
  namespace po = boost::program_options;

  std::string opt_str;

  // Define arguments
  po::variables_map cmd_vm;

  opt_desc.add_options()
    // Common Options:
    ("ref,r", po::value<std::string>()->required(), "reference genome path")

    // Alignment Options:
    ("fastq1,1", po::value<std::string>(), "input pair-end fastq file")
    ("fastq2,2", po::value<std::string>(), "input pair-end fastq file")

    arg_decl_string("sample_sheet,F", "Sample Sheet or Folder")   
    arg_decl_string_w_def("read-group,R", "sample",   "read group id ('ID' in BAM header)")
    arg_decl_string_w_def("sample-id,S", "sample",   "sample id ('SM' in BAM header)")
    arg_decl_string_w_def("platform,P", "illumina", "platform id ('PL' in BAM header)")
    arg_decl_string_w_def("library,l", "sample",   "library id ('LB' in BAM header)")
    ("produce-bam, b", "select to produce sorted BAM file after alignment")

    // HaplotypeCaller Options:
    ("output,o", po::value<std::string>()->required(), "output GVCF/VCF file")
    ("produce-vcf,v", "produce VCF files from HaplotypeCaller instead of GVCF")
    ("intervalList,L", po::value<std::string>()->implicit_value(""), "interval list file")
    ("gatk4", "use GATK 4.0 instead of 3.x")
    ("htc-extra-options", po::value<std::vector<std::string> >(), "extra options for HaplotypeCaller");  

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc), cmd_vm);

  if (cmd_vm.count("help")) {
    throw helpRequest();
  }

  // Common Arguments:
  bool flag_f             = get_argument<bool>(cmd_vm, "force", "f");
  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref", "r");

  // Alignment Arguments :
  std::string sampleList  = get_argument<std::string>(cmd_vm, "sample_sheet", "F");
  std::string fq1_path    = get_argument<std::string>(cmd_vm, "fastq1", "1");
  std::string fq2_path    = get_argument<std::string>(cmd_vm, "fastq2", "2");
  std::string read_group  = get_argument<std::string>(cmd_vm, "read-group", "R");
  std::string sample_tag  = get_argument<std::string>(cmd_vm, "sample-id", "S");
  std::string platform_id = get_argument<std::string>(cmd_vm, "platform", "P");
  std::string library_id  = get_argument<std::string>(cmd_vm, "library", "l");
  bool flag_produce_bam   = get_argument<bool>(cmd_vm, "produce-bam", "b");
  
  // Extra Options for Aligner:
  std::vector<std::string> extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");

  // check configurations for HaplotypeCaller:
  check_nprocs_config("htc");
  check_memory_config("htc");

  // HaplotypeCaller Arguments:
  bool flag_vcf           = get_argument<bool>(cmd_vm, "produce-vcf", "v");
  bool flag_gatk4         = get_argument<bool>(cmd_vm, "gatk4");

  std::string output_path = get_argument<std::string>(cmd_vm, "output", "o");
  std::string intv_list   = get_argument<std::string>(cmd_vm, "intervalList", "L");

  // Extra Options for HaplotypeCaller:
  std::vector<std::string> htc_extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "htc-extra-options");

  // finalize argument parsing
  po::notify(cmd_vm);

  //output_path = check_output(output_path, flag_f);

  // Sample Sheet must satisfy the following format:
  // #sample_id,fastq1,fastq2,rg,platform_id,library_id
  SampleSheetMap sample_data;
  std::vector<SampleDetails> sample_info_vect;
  if (sampleList.empty()) {
    if (fq1_path.empty() || fq2_path.empty()) {
      LOG(ERROR) << "Either --sample-sheet or --fastq1,fastq2 needs to be specified";
      throw invalidParam("");
    }
    else {
      SampleDetails sample_info;
      sample_info.fastqR1 = fq1_path;
      sample_info.fastqR2 = fq2_path;
      sample_info.ReadGroup = read_group;
      sample_info.Platform = platform_id;
      sample_info.LibraryID = library_id;
      sample_info_vect.push_back(sample_info);
      sample_data.insert(make_pair(sample_tag, sample_info_vect));
    }
  }
  else {
    if (!fq1_path.empty() || !fq2_path.empty()) {
      LOG(ERROR) << "--sample-sheet and --fastq1,fastq2 cannot be specified at the same time";
      throw invalidParam("");
    }
    else {
      SampleSheet my_sheet(sampleList);
      sample_data = my_sheet.get();
    }
  }

  int my_num = (int) round(get_config<int>("minimap.num_buckets")/get_config<int>("gatk.ncontigs"));

  // start execution
  
  // check available space in temp dir
  namespace fs = boost::filesystem;

  // generate interval folder. This vector will be used if input_htc is a regular merged BAM:
  std::vector<std::string> temp_intv=init_contig_intv(ref_path);

  // TODO: number can be tuned
  int num_buckets =  get_config<int>("gatk.ncontigs");

  // The BAM file used for HTC:
  std::string input_htc;

  // Going through each line in the Sample Sheet:
  for (auto pair : sample_data) {
    std::string sample_id = pair.first;
    std::vector<SampleDetails> list = pair.second;

    std::string temp_bam_dir = conf_temp_dir + "/align/" + sample_id;
    std::string temp_vcf_dir = conf_temp_dir + "/htc/" + sample_id;

    // Every sample will have a temporal folder where each pair of FASTQ files will have its own
    // folder using the Read Group as label.
    create_dir(temp_bam_dir);
    create_dir(temp_vcf_dir);

    std::vector<std::string> output_bams;

    // Loop through all the pairs of FASTQ files:
    std::string read_tag;
    for (int i = 0; i < list.size(); ++i) {

      Executor executor("Falcon Fast Germline", get_config<int>("sort.nprocs", "gatk.nprocs"));

      std::string fq1_path    = list[i].fastqR1;
      std::string fq2_path    = list[i].fastqR2;
      std::string read_group  = list[i].ReadGroup;
      std::string platform_id = list[i].Platform;
      std::string library_id  = list[i].LibraryID;

      read_tag = read_group;
      std::string parts_dir = temp_bam_dir + "/" + read_group;
      std::string output;
      create_dir(parts_dir);
      //check_output(parts_dir, flag_f);
     
      // If sample sheet is defined, then output_path is the parent dir and for each sample in the sample sheet, 
      // a folder is created in the parent dir. 
      if (!sampleList.empty()) {
        DLOG(INFO) << "Creating : " + output_path + "/" + sample_id;
        std::string output_dir = output_path + "/" + sample_id;
        create_dir(output_dir);
        output = output_dir + "/" + read_group + ".bam";
      } 
      else {
        output = get_fname_by_ext(output_path, "bam");
      }
      if (flag_produce_bam) {
        check_output(output, flag_f);
      }
      output_bams.push_back(output);

      Worker_ptr worker(new Minimap2Worker(ref_path, fq1_path, fq2_path,
            parts_dir, output,
            num_buckets,
            extra_opts,
            sample_id, read_group, platform_id, library_id, 
            flag_produce_bam, flag_f));

      executor.addTask(worker, sample_id, true);

      // perform sambamba sort for each part BAM if not producing bam
      if (!flag_produce_bam) {

        input_htc = temp_bam_dir + "/" + read_tag;

        for (int i = 0; i < get_config<int>("minimap.num_buckets"); i++) {
          std::string input = get_bucket_fname(parts_dir, i);
          bool flag = true;
          Worker_ptr worker(new SambambaWorker(
                input, "",
                SambambaWorker::SORT,
                "", flag)); 
          executor.addTask(worker, sample_id, i == 0);
        };
      }
      else {
        // Produce Merged BAM is required, then 
        bool flag = true;
        Worker_ptr worker(new SambambaWorker(
              output, "",
              SambambaWorker::INDEX,
              "", flag)); 
        executor.addTask(worker, sample_id, true);
        // The single BAM used for HTC:
        input_htc=output;
      }

      executor.run();

    } // END for (int i = 0; i < list.size(); ++i)

    DLOG(INFO) << "Alignment Completed for " << sample_id;
 
    // TODO: merge bam if --generate-bam is selected
    // ============================
 
    // Proceed to compute VCF file:
    // ============================
 
    DLOG(INFO) << " VCF dir : " << temp_vcf_dir;
 
    // start an executor for NAM
    Worker_ptr blaze_worker(new BlazeWorker(
          get_config<std::string>("blaze.nam_path"),
          get_config<std::string>("blaze.conf_path")));
 
    BackgroundExecutor bg_executor(
          sample_id.empty() ? "blaze-nam" : "blaze-nam-" + sample_id,
          blaze_worker);
 
    std::string file_ext = flag_vcf ? "vcf" : "g.vcf";
    std::vector<std::string> output_files(get_config<int>("gatk.ncontigs"));
    std::vector<std::string> intv_paths;
    // If user defines an interval list, then that list will be the first 
    // element of intv_paths and will be a common file for all HTC process.
    if (!intv_list.empty()) {
      intv_paths.push_back(intv_list);
    }

    Executor executor("Falcon Fast Germline", 
        get_config<int>("gatk.htc.nprocs", "gatk.nprocs"));
 
    for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {
      std::string output_file = get_contig_fname(temp_vcf_dir, contig, file_ext);

      // If input BAM is a regular file and not a folder, then each java process will use 
      // the corresponding region from the reference genome.  The folder BAM has the parts BAM with their
      // corresponding region list
      if (boost::filesystem::is_regular_file(input_htc)){
        intv_paths.push_back(temp_intv[contig]);
      }

      Worker_ptr worker(new HTCWorker(ref_path,
	 intv_paths,
	 input_htc,
         output_file,
         htc_extra_opts,
         contig,
         flag_vcf, flag_f, flag_gatk4)
      );

      // For the next java process, we need to delete the previous part reference list
      // since each java process is using the same BAM file if input is a regular BAM file.
      // If interval list is defined, it will be kept unaltered since it is posted as element 0, i.e., intv_paths
      // will have two elements (0: interval list and 1: part reference list) in this case.  If interval list not
      // not defined, then intv_paths has 1 element (part reference list). 
      // In the case of a BAM folder, the BAMInput Class will use the parts list from its component (check HTCWorker.cpp):       
      if (boost::filesystem::is_regular_file(input_htc)){
        intv_paths.pop_back();
      }

      output_files[contig] = output_file;
      executor.addTask(worker, sample_id, contig == 0);
 
    } // END of for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++)
   
    bool flag = true;
    bool flag_a = false;
    bool flag_bgzip = false;
   
    std::string output_vcf;
    if (!sampleList.empty()) {
      output_vcf = output_path + "/" + sample_id + file_ext;
    }
    else {
      output_vcf = output_path;
    } 
    DLOG(INFO) << output_vcf << "\n";     
   
    { // concat gvcfs
      Worker_ptr worker(new VCFConcatWorker(
         output_files,
         temp_vcf_dir + "/output." + file_ext,
         flag_a,
         flag_bgzip,
         flag_f)
      );
      executor.addTask(worker, sample_id, true);
    }
    { // bgzip gvcf
      Worker_ptr worker(new ZIPWorker(
         temp_vcf_dir + "/output." + file_ext,
         output_vcf + ".gz",
         flag_f)
      );
      executor.addTask(worker, sample_id, true);
    }
    { // tabix gvcf
      Worker_ptr worker(new TabixWorker(
         output_vcf + ".gz")
      );
      executor.addTask(worker, sample_id, true);
    }
    executor.run();
   }; //for (auto pair : SampleData)

  return 0;
}
} // namespace fcsgenome
