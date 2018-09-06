#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <string>

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

int align_main(int argc, char** argv,
    boost::program_options::options_description &opt_desc)
{
  namespace po = boost::program_options;

  std::string opt_str;

  // Define arguments
  po::variables_map cmd_vm;

  opt_desc.add_options()
    ("ref,r", po::value<std::string>()->required(), "reference genome path")
    ("fastq1,1", po::value<std::string>(), "input pair-end fastq file")
    ("fastq2,2", po::value<std::string>(), "input pair-end fastq file")
    ("output,o", po::value<std::string>()->required(), "output BAM file (if --align-only is set "
                                "the output will be a directory of BAM "
                                "files)")
    arg_decl_string("sample_sheet,F", "Sample Sheet or Folder")
    arg_decl_string_w_def("rg,R", "sample",   "read group id ('ID' in BAM header)")
    arg_decl_string_w_def("sp,S", "sample",   "sample id ('SM' in BAM header)")
    arg_decl_string_w_def("pl,P", "illumina", "platform id ('PL' in BAM header)")
    arg_decl_string_w_def("lb,L", "sample",   "library id ('LB' in BAM header)")
    ("align-only,l", "skip mark duplicates");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc), cmd_vm);

  if (cmd_vm.count("help")) {
    throw helpRequest();
  }

  // Check if required arguments are presented
  bool flag_f          = get_argument<bool>(cmd_vm, "force", "f");
  bool flag_align_only = get_argument<bool>(cmd_vm, "align-only", "l");

  std::string ref_path    = get_argument<std::string>(cmd_vm, "ref", "r");
  std::string sampleList  = get_argument<std::string>(cmd_vm, "sample_sheet", "F");
  std::string fq1_path    = get_argument<std::string>(cmd_vm, "fastq1", "1");
  std::string fq2_path    = get_argument<std::string>(cmd_vm, "fastq2", "2");
  std::string read_group  = get_argument<std::string>(cmd_vm, "rg", "R");
  std::string sample_id   = get_argument<std::string>(cmd_vm, "sp", "S");
  std::string platform_id = get_argument<std::string>(cmd_vm, "pl", "P");
  std::string library_id  = get_argument<std::string>(cmd_vm, "lb", "L");
  std::string output_path = get_argument<std::string>(cmd_vm, "output", "o");
  std::vector<std::string> extra_opts = get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");
  std::string master_outputdir = output_path;

  // finalize argument parsing
  po::notify(cmd_vm);

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
      sample_data.insert(make_pair(sample_id, sample_info_vect));
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
  std::string temp_dir = conf_temp_dir + "align";
  create_dir(temp_dir);

  // check available space in temp dir
  namespace fs = boost::filesystem;
  std::string output_path_temp;
  std::string BAMfile;
  // Going through each line in the Sample Sheet:
  for (auto pair : sample_data) {
    std::string sample_id = pair.first;
    std::vector<SampleDetails> list = pair.second;

    std::string inputBAMsforMerge;
    std::vector<std::string> input_files_ ;
    int counting_rg = 0;

    DLOG(INFO) << "Creating : " + output_path + "/" + sample_id;
    create_dir(output_path + "/" + sample_id);

    // Loop through all the pairs of FASTQ files:
    for (int i = 0; i < list.size(); ++i) {
      fq1_path = list[i].fastqR1;
      fq2_path = list[i].fastqR2;
      read_group = list[i].ReadGroup;
      platform_id = list[i].Platform;
      library_id = list[i].LibraryID;

      // Every sample will have a temporal folder where each pair of FASTQ files will have its own
      // folder using the Read Group as label.
      create_dir(temp_dir + "/" + sample_id);
      parts_dir = temp_dir + "/" + sample_id + "/" + read_group;

      DLOG(INFO) << "Putting sorted BAM parts in '" << parts_dir << "'";
      
      uint64_t start_align = getTs();
      Executor executor("bwa mem " + sample_id + " ReadGroup" + read_group);
      Worker_ptr worker(new BWAWorker(ref_path,
           fq1_path, fq2_path,
           parts_dir,
           extra_opts,
           sample_id, read_group,
           platform_id, library_id, flag_f));

      executor.addTask(worker);
      executor.run();

      DLOG(INFO) << "Alignment Completed for " << sample_id;

      // Preparing Parts BAM for merge:
      get_input_list(parts_dir, input_files_, ".*/part-[0-9].*", true);
      for (int n = 0; n < input_files_.size(); n++) {
          inputBAMsforMerge = inputBAMsforMerge + " " + input_files_[n];
      }      
      counting_rg++;

      // Once the sample reach its last pair of FASTQ files, we proceed to merge and mark duplicates (if requested): 
      if (i == list.size()-1){ 
	std::string mergeBAM;
        if (!flag_align_only){
          // Planning to mark duplicates, so this BAM file will go to the temporal folder:
	  mergeBAM = temp_dir + "/" + sample_id + "/" + sample_id + ".bam";
        } 
        else {
	  mergeBAM = output_path + "/" + sample_id + "/" + sample_id + ".bam";
	}

        int check_parts = 1;  // It is 1 if sample has multiple pairs of FASTQ files                                                                                         
	if (counting_rg == 1) check_parts = 0;             

        uint64_t start_merging = getTs();
	std::string log_filename_merge  = output_path + "/" + sample_id + "/" + sample_id + "_bwa.log";
	std::ofstream merge_log;
        merge_log.open(log_filename_merge, std::ofstream::out | std::ofstream::app);
        merge_log << sample_id << ":" << "Start Merging BAM Files " << std::endl;

        Executor merger_executor("Merge BAM files " + sample_id);
        Worker_ptr merger_worker(new MergeBamWorker(inputBAMsforMerge, mergeBAM, check_parts, flag_f));
        merger_executor.addTask(merger_worker);
        merger_executor.run();
        if (list.size() > 1){
	  DLOG(INFO) << "Merging BAM files  " << inputBAMsforMerge << " for " << sample_id << " completed " << std::endl;
        } else {
	  DLOG(INFO) << "BAM file " << inputBAMsforMerge << " for " << sample_id << " posted in main folder " << std::endl;
        }
        merge_log << sample_id << ":" << "Merging BAM files finishes in " << getTs() - start_merging << " seconds" << std::endl;
        merge_log.close(); merge_log.clear();

	if (!flag_align_only) {                                                                                                                                              
	  // Marking Duplicates:                                                                                                       
	  std::string markedBAM;                                                                                                                                             
	  markedBAM = output_path + "/" + sample_id + "/" + sample_id  + "_marked.bam";                                                                       
	  uint64_t start_markdup = getTs();                                                                                                                             
	  Executor executor("Mark Duplicates " + sample_id);                                                                                                                              
	  Worker_ptr worker(new MarkdupWorker(mergeBAM, markedBAM, flag_f));
          executor.addTask(worker);                                                                                                                                          
	  executor.run();                                                                                                                                                      
	}

        // Removing temporal data :
        remove_path(temp_dir + "/" + sample_id);
      }
     
    }; // for (int i = 0; i < list.size(); ++i)  ends
    
  }; //for (auto pair : SampleData)

  return 0;
}
} // namespace fcsgenome
