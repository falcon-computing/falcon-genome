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
                                "the output will be a directory of BAM files. if --sample-sheet is set "
                                " output will be the folder where all outputs of the samples defined in --sample-sheet will be placed)")
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

  // finalize argument parsing
  po::notify(cmd_vm);
  
  if (sampleList.empty()) {
    output_path = check_output(output_path, flag_f, true);
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
  std::string temp_dir = conf_temp_dir + "/align";
  create_dir(temp_dir);

  // check available space in temp dir
  namespace fs = boost::filesystem;
  Executor executor("align");

  // Going through each line in the Sample Sheet:
  for (auto pair : sample_data) {
    std::string sample_id = pair.first;
    std::vector<SampleDetails> list = pair.second;
    std::vector<std::string> input_files_ ;

    // If sample sheet is defined, then output_path is the parent dir and for each sample in the sample sheet, 
    // a folder is created in the parent dir. 
    if (!sampleList.empty()) {
      DLOG(INFO) << "Creating : " + output_path + "/" + sample_id  + ".bam";
      create_dir(output_path + "/" + sample_id);
    } 

    // Every sample will have a temporal folder where each pair of FASTQ files will have its own
    // folder using the Read Group as label.
    DLOG(INFO) << "Creating " << temp_dir + "/" + sample_id;
    create_dir(temp_dir + "/" + sample_id);

    // Loop through all the pairs of FASTQ files:
    for (int i = 0; i < list.size(); ++i) {
      fq1_path = list[i].fastqR1;
      fq2_path = list[i].fastqR2;
      read_group = list[i].ReadGroup;
      platform_id = list[i].Platform;
      library_id = list[i].LibraryID;

      parts_dir = temp_dir + "/" + sample_id + "/" + read_group;
      
      DLOG(INFO) << "Putting sorted BAM parts in '" << parts_dir << "'";

      Worker_ptr worker(new BWAWorker(ref_path,
           fq1_path, fq2_path,
           parts_dir,
           extra_opts,
           sample_id, 
           read_group,
	   platform_id, 
           library_id, 
	   flag_align_only,
	   flag_f)
      );

      executor.addTask(worker, sample_id, 0);

      DLOG(INFO) << "Alignment Completed for " << sample_id;

      // Once the sample reach its last pair of FASTQ files, we proceed to merge (if requested):
      if (i == list.size()-1) {
	std::string mergeBAM;
      	if (!flag_align_only) {
      	  // Marking Duplicates:
      	  //std::string markedBAM;
      	  //markedBAM = output_path + "/" + sample_id + "/" + sample_id  + "_marked.bam";
          //if (sampleList.empty()) markedBAM = output_path;
      	  //Worker_ptr markdup_worker(new SambambaWorker(temp_dir + "/" + sample_id, markedBAM, SambambaWorker::MARKDUP, flag_f));
          //executor.addTask(markdup_worker, sample_id, true);
	  mergeBAM = output_path + "/" + sample_id + "/" + sample_id + "_marked.bam";
      	} 
        else {
      	  mergeBAM = output_path + "/" + sample_id + "/" + sample_id + ".bam";
        }
        if (sampleList.empty()) mergeBAM = output_path;
	Worker_ptr merger_worker(new SambambaWorker(temp_dir + "/" + sample_id, mergeBAM, SambambaWorker::MERGE, flag_f));
	executor.addTask(merger_worker, sample_id, true);
     
        // Removing temporal data :
        remove_path(temp_dir + "/" + sample_id);
      }

    }; // for (int i = 0; i < list.size(); ++i)  ends
 
    executor.run();
  }; //for (auto pair : SampleData)

  return 0;
}
} // namespace fcsgenome
