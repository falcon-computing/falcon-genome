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
    ("align-only,l", "skip mark duplicates")
    ("disable-merge", "give bucket bams instead of the whole bam");

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc), cmd_vm);

  if (cmd_vm.count("help")) {
    throw helpRequest();
  }

  // Check if required arguments are presented
  bool flag_f          = get_argument<bool>(cmd_vm, "force", "f");
  bool flag_align_only = get_argument<bool>(cmd_vm, "align-only", "l");
  bool flag_disable_merge  = get_argument<bool>(cmd_vm, "disable-merge");

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

  // For a single sample: Check if Output Directory exists. If not, create:
  boost::filesystem::path p(output_path);
  boost::filesystem::path dir = p.parent_path();
  if (boost::filesystem::is_directory(dir)==false && !dir.string().empty()) {
    boost::filesystem::create_directory(dir);   
  }

  if (sampleList.empty() && !flag_disable_merge) {
    output_path = check_output(output_path, flag_f, true);
  } else {
    // check when output suppose to be a directory
    if (boost::filesystem::exists(output_path) && 
        !boost::filesystem::is_directory(output_path)) {
      throw (fileNotFound("Output path " + output_path + " is not a directory"));
    }
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
  std::string temp_dir = conf_temp_dir + "/align";
  create_dir(temp_dir);

  // check available space in temp dir
  namespace fs = boost::filesystem;
  Executor executor("align", get_config<int>("sort.nprocs", "gatk.nprocs"));

  // Going through each line in the Sample Sheet:
  for (auto pair : sample_data) {
    std::string sample_id = pair.first;
    std::vector<SampleDetails> list = pair.second;
    std::vector<std::string> input_files_ ;
    std::string parts_dir;
    std::string temp_bam;

    // If sample sheet is defined, then output_path is the parent dir and for each sample in the sample sheet, 
    // a folder is created in the parent dir. 

    std::string output_path_dir = boost::filesystem::change_extension(output_path, "").string();
    temp_bam = output_path_dir + "/" + sample_id;
    create_dir(temp_bam);

    // Every sample will have a temporal folder where each pair of FASTQ files will have its own
    // folder using the Read Group as label.
    // DLOG(INFO) << "Creating " << temp_dir + "/" + sample_id;
    parts_dir = temp_dir + "/" + sample_id;
    create_dir(parts_dir);

    // Loop through all the pairs of FASTQ files:
    for (int i = 0; i < list.size(); ++i) {
      fq1_path = list[i].fastqR1;
      fq2_path = list[i].fastqR2;
      read_group = list[i].ReadGroup;
      platform_id = list[i].Platform;
      library_id = list[i].LibraryID;

      std::string parts_dir_rg = parts_dir + "/" + sample_id + "_" + read_group;
      
      std::string temp_bam_rg;
      if (sampleList.empty()) temp_bam_rg = output_path;
      else temp_bam_rg = (list.size()==1)?(temp_bam + "/../" + sample_id + ".bam"):
                         (temp_bam + "/" + sample_id + "_" + read_group + ".bam");

      create_dir(parts_dir_rg);
      if (flag_disable_merge) {
        create_dir(temp_bam_rg);
      }

      Worker_ptr worker(new BWAWorker(
        ref_path, 
        fq1_path, 
        fq2_path, 
        parts_dir_rg, 
        temp_bam_rg, 
        extra_opts, 
        sample_id, 
        read_group, 
        platform_id, 
        library_id, 
        flag_align_only, 
        !flag_disable_merge, 
        flag_f)
      );
      executor.addTask(worker, sample_id, 1);

      // if no-merge-bams chose, we sort each bucket after bwa of each pair of fastq
      if (flag_disable_merge) {
        for (int i = 0; i < get_config<int>("bwa.num_buckets"); i++) {
          std::string samb_input = get_bucket_fname(parts_dir_rg, i);
          std::string samb_output = get_bucket_fname(temp_bam_rg, i);
          bool flag = true;
          Worker_ptr worker(new SambambaWorker(
            samb_input, samb_output,
            SambambaWorker::SORT, "", flag));
          executor.addTask(worker, sample_id, i == 0);
        }
      }
    } // for (int i = 0; i < list.size(); ++i)  ends
    
    if (!flag_disable_merge) {
      // if bams are merged by bwa, we only merge between RGs or index
      std::string mergeBAM;

      if (sampleList.empty()) {
        mergeBAM = output_path;
      } else {
        mergeBAM = output_path + "/" + sample_id + ".bam";
      }

      SambambaWorker::Action ActionTag;
      if (list.size()<2) {
        ActionTag = SambambaWorker::INDEX;
        temp_bam = mergeBAM; // the Index worker do not use input file 
                             // but it complains if file does not exist.
      } else {
        ActionTag = SambambaWorker::MERGE;
      }

      Worker_ptr merger_worker(new SambambaWorker(
        temp_bam, mergeBAM,
        ActionTag, 
        ".*/" + sample_id + "*.*", flag_f));
      
      executor.addTask(merger_worker, sample_id, true); 
    }
    else if (list.size() != 1) {
      // when bams are not merged by bwa, we merge each bucket of all RGs.
      // when only one pair of fastqs, not merge is needed. Buckets already in output_path/{id}.bam
      std::string mergeBAM;
      if (sampleList.empty()) {
        mergeBAM = output_path;
      } else {
        mergeBAM = output_path + "/" + sample_id + ".bam";
      }
      create_dir(mergeBAM);
      for (int i = 0; i < get_config<int>("bwa.num_buckets"); i++) {
        std::vector<std::string> input_files;
        for (int j = 0; j < list.size(); j++) {
          std::string read_group = list[j].ReadGroup;
          std::string temp_bam_rg = temp_bam + "/" + sample_id + "_" + read_group + ".bam";
          std::string file_path = get_bucket_fname(temp_bam_rg, i);
          input_files.push_back(file_path);
        }
        std::string output_folder;
        Worker_ptr merger_worker(new SambambaWorker(
          "", get_bucket_fname(mergeBAM, i),
          SambambaWorker::MERGE, "", flag_f, input_files));
        executor.addTask(merger_worker, sample_id, i == 0);
      }
    }

    executor.run();
  } //for (auto pair : SampleData)

  return 0;
}
} // namespace fcsgenome
