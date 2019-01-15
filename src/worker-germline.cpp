#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <string>

#include "fcs-genome/BackgroundExecutor.h"
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
    ("sample-id", po::value<std::string>()->implicit_value(""), "sample id for log files")
    ("output-bam", po::value<std::string>()->required(), "output sorted BAM file generated by aligner")

    // Alignment Options:
    ("fastq1,1", po::value<std::string>(), "input pair-end fastq file")
    ("fastq2,2", po::value<std::string>(), "input pair-end fastq file")

    arg_decl_string("sample_sheet,F", "Sample Sheet or Folder")   
    arg_decl_string_w_def("rg,R", "sample",   "read group id ('ID' in BAM header)")
    arg_decl_string_w_def("sp,S", "sample",   "sample id ('SM' in BAM header)")
    arg_decl_string_w_def("pl,P", "illumina", "platform id ('PL' in BAM header)")
    arg_decl_string_w_def("lb,L", "sample",   "library id ('LB' in BAM header)")
    ("align-only,l", "skip mark duplicates")

    // HaplotypeCaller Options:
    ("output-vcf", po::value<std::string>()->implicit_value(""), "output GVCF/VCF file")
    ("produce-vcf,v", "produce VCF files from HaplotypeCaller instead of GVCF")
    ("intervalList,L", po::value<std::string>()->implicit_value(""), "interval list file")
    ("htc-extra-options", po::value<std::vector<std::string> >(), "extra options for HaplotypeCaller");  

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc), cmd_vm);

  if (cmd_vm.count("help")) {
    throw helpRequest();
  }

  // Common Arguments:
  bool flag_f             = get_argument<bool>(cmd_vm, "force", "f");
  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref", "r");
  std::string sample_id   = get_argument<std::string>(cmd_vm, "sample-id");
  std::string output_bam_path = get_argument<std::string>(cmd_vm, "output-bam");

  // Alignment Arguments :
  bool flag_align_only = get_argument<bool>(cmd_vm, "align-only", "l");

  std::string sampleList  = get_argument<std::string>(cmd_vm, "sample_sheet", "F");
  std::string fq1_path    = get_argument<std::string>(cmd_vm, "fastq1", "1");
  std::string fq2_path    = get_argument<std::string>(cmd_vm, "fastq2", "2");
  std::string read_group  = get_argument<std::string>(cmd_vm, "rg", "R");
  std::string sample_tag  = get_argument<std::string>(cmd_vm, "sp", "S");
  std::string platform_id = get_argument<std::string>(cmd_vm, "pl", "P");
  std::string library_id  = get_argument<std::string>(cmd_vm, "lb", "L");
  
  // Extra Options for Aligner:
  std::vector<std::string> extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");

  // check configurations for HaplotypeCaller:
  check_nprocs_config("htc");
  check_memory_config("htc");

  // HaplotypeCaller Arguments:
  bool flag_vcf           = get_argument<bool>(cmd_vm, "produce-vcf", "v");

  std::string output_vcf_path = get_argument<std::string>(cmd_vm, "output-vcf");
  std::string intv_list       = get_argument<std::string>(cmd_vm, "intervalList", "L");

  // Extra Options for HaplotypeCaller:
  std::vector<std::string> htc_extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "htc-extra-options");

  // finalize argument parsing
  po::notify(cmd_vm);

  // Check if Reference File (FASTA) has its index MMI:
  std::string ref_map, ref_htc;
  boost::filesystem::path p0(ref_path);
  boost::filesystem::path refdir = p0.parent_path();
  if (boost::filesystem::exists(p0)){ 
    if (boost::filesystem::exists(refdir.string() + "/" + p0.stem().string() + ".mmi") ) {
      ref_map=refdir.string() + "/" + p0.stem().string() + ".mmi";
      DLOG(INFO) << "Reference Index found: " << ref_map << "\n";
    }
    else {
      ref_map=p0.string();
    };
    ref_htc=p0.string();
    DLOG(INFO) << "Reference FASTA found: " << ref_htc << "\n";
  }

  // For a single sample: Check if Output BAM Directory exists. If not, create:
  boost::filesystem::path p1(output_bam_path);
  boost::filesystem::path bamdir = p1.parent_path();
  if (boost::filesystem::is_directory(bamdir)==false && !bamdir.string().empty()) {
    boost::filesystem::create_directory(bamdir);   
  }

  // For a single sample: Check if Output VCF Directory exists. If not, create:
  boost::filesystem::path p2(output_vcf_path);
  boost::filesystem::path vcfdir = p2.parent_path();
  if (boost::filesystem::is_directory(vcfdir)==false && !vcfdir.string().empty()) {
    boost::filesystem::create_directory(vcfdir);
  }

  if (sampleList.empty()) {
    output_bam_path = check_output(output_bam_path, flag_f, true);
  }

  if (sampleList.empty() && output_vcf_path.empty()) {
    LOG(ERROR) << "Processing a single sample. VCF filename must be defined (--output-vcf)";
    throw invalidParam("");
  }

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

  // start execution
  std::string parts_dir;
  std::string temp_bam;
  std::string temp_dir_map = conf_temp_dir + "/align";
  create_dir(temp_dir_map);

  std::string temp_dir_htc = conf_temp_dir + "/htc";
  create_dir(temp_dir_htc);
  
  // check available space in temp dir
  namespace fs = boost::filesystem;
  //Executor executor("germline");
  Executor executor("Falcon Fast Germline", get_config<int>("gatk.htc.nprocs", "gatk.nprocs"));

  // Going through each line in the Sample Sheet:
  for (auto pair : sample_data) {
    std::string sample_id = pair.first;
    std::vector<SampleDetails> list = pair.second;
    std::vector<std::string> input_files_ ;

    // If sample sheet is defined, then output_path is the parent dir and for each sample in the sample sheet, 
    // a folder is created in the parent dir. 
    if (!sampleList.empty()) {
      DLOG(INFO) << "Creating : " + output_bam_path + "/" + sample_id;
      create_dir(output_bam_path + "/" + sample_id);
    } 

    // Every sample will have a temporal folder where each pair of FASTQ files will have its own
    // folder using the Read Group as label.
    DLOG(INFO) << "Creating " << temp_dir_map + "/" + sample_id;
    create_dir(temp_dir_map + "/" + sample_id);
    create_dir(temp_dir_htc + "/" + sample_id);

    // Define mergeBAM:
    std::string mergeBAM;

    // Loop through all the pairs of FASTQ files:
    for (int i = 0; i < list.size(); ++i) {
      fq1_path = list[i].fastqR1;
      fq2_path = list[i].fastqR2;
      read_group = list[i].ReadGroup;
      platform_id = list[i].Platform;
      library_id = list[i].LibraryID;

      parts_dir = temp_dir_map + "/" + sample_id + "/" + read_group ;

      if (!sampleList.empty()) {
        temp_bam = output_bam_path + "/" + sample_id  + "/" + sample_id + "_" + read_group + ".bam";
        if (list.size()==1) temp_bam = output_bam_path + "/" + sample_id  + "/" + sample_id + ".bam"; 
      } 
      else {
        temp_bam = output_bam_path;
      }

      DLOG(INFO) << "Putting sorted BAM parts in '" << parts_dir << "'";

      Worker_ptr map_worker(new Minimap2Worker(ref_map,
        fq1_path, fq2_path,
        parts_dir,
	temp_bam,
        extra_opts,
        sample_id, 
        read_group,
	platform_id, 
        library_id, 
	flag_align_only,
	flag_f)
      );

      executor.addTask(map_worker, sample_id, 0);

      DLOG(INFO) << "Alignment Completed for " << sample_id;

      // Once the sample reach its last pair of FASTQ files, we proceed to merge and mark duplicates (if requested):
      if (i == list.size()-1) {
        if (sampleList.empty()) {
	  mergeBAM = output_bam_path;
	  } else{
            // Sample Sheet : 
            temp_bam = output_bam_path + "/" + sample_id + "/";
	    mergeBAM = output_bam_path + "/" + sample_id + "/" + sample_id + ".bam";
	};

	SambambaWorker::Action ActionTag;
        if (list.size()<2) {
          ActionTag = SambambaWorker::INDEX;
        } else {
          ActionTag = SambambaWorker::MERGE;
        }
        Worker_ptr merger_worker(new SambambaWorker(temp_bam, mergeBAM, ActionTag, ".*/" + sample_id + "*.*", flag_f));
        executor.addTask(merger_worker, sample_id, true); 
      } // i == list.size()-1 completed

    }; // for (int i = 0; i < list.size(); ++i)  ends

    // Proceed to compute VCF file:
    // ============================
  
    std::string output_dir = temp_dir_htc + "/" + sample_id;
    std::string temp_gvcf_path = output_dir + "/" + get_basename(output_vcf_path);
    create_dir(output_dir);

     DLOG(INFO) << " VCF dir : " << output_dir << "\n";
     DLOG(INFO) << " VCF file: " << temp_gvcf_path << "\n";
  
     std::vector<std::string> output_files(get_config<int>("gatk.ncontigs"));
     std::vector<std::string> intv_paths;
     if (!intv_list.empty()) {
       intv_paths = split_by_nprocs(intv_list, "bed");
     }
     else {
       intv_paths = init_contig_intv(ref_path);
     }
     
     // start an executor for NAM
     Worker_ptr blaze_worker(new BlazeWorker(get_config<std::string>("blaze.nam_path"),get_config<std::string>("blaze.conf_path")));
    
     std::string tag;
     if (!sample_id.empty()) {
       tag = "blaze-nam-" + sample_id;
     }
     else {
       tag = "blaze-nam";
     }
    
     BackgroundExecutor bg_executor(tag, blaze_worker);
     for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {
       std::string file_ext = "vcf";
       if (!flag_vcf) {
     	  file_ext = "g." + file_ext;
       }
    
       std::string output_file = get_contig_fname(output_dir, contig, file_ext);
       Worker_ptr htc_worker(new HTCWorker(ref_htc,
     	   intv_paths[contig],
     	   mergeBAM,
     	   output_file,
     	   htc_extra_opts,
     	   contig,
     	   flag_vcf,
     	   flag_f,
     	   flag_f)
       );
       output_files[contig] = output_file;
       executor.addTask(htc_worker,sample_id,contig==0);
     }
  
     bool flag = true;
     bool flag_a = false;
     bool flag_bgzip = false;
  
     std::string output_vcf;
     if (!sampleList.empty()) {
       output_vcf = output_vcf_path + "/" + sample_id + "/" + sample_id + ".vcf";
     }
     else {
       output_vcf = output_vcf_path;
     } 
     DLOG(INFO) << output_vcf << "\n";     
     
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
