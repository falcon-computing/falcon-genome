#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <sys/resource.h>
#include <sys/statvfs.h>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/workers.h"
#include "fcs-genome/SampleSheet.h"

#include <iostream>
#include <fstream>
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
  po::store(po::parse_command_line(argc, argv, opt_desc),
      cmd_vm);

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
  std::vector<std::string> extra_opts =
          get_argument<std::vector<std::string>>(cmd_vm, "extra-options", "O");

  std::string master_outputdir = output_path;

  // finalize argument parsing
  po::notify(cmd_vm);

  if (fq1_path.empty()) {
     if (fq2_path.empty() && sampleList.empty()) {
         throw std::runtime_error("FASTQ filenames and Sample Sheet cannot be undefined at the same time. Set either FASTQ filenames (fastq1, fastq2) or Sample Sheet.");
     }
     if (!fq2_path.empty() && sampleList.empty()) {
         throw std::runtime_error("FASTQ filenames (fastq1, fastq2) = (undefined, defined). Both fastq1 and fastq2 must be defined.");
     }
     if (!fq2_path.empty() && !sampleList.empty()) {
         throw std::runtime_error("FASTQ filenames (fastq1, fastq2) = (undefined, defined) and Sample Sheet defined. Set either FASTQ filenames or Sample Sheet");
     }
  } else {
     if (fq2_path.empty() && sampleList.empty()) {
         throw std::runtime_error("FASTQ filenames (fastq1, fastq2) = (defined, undefined). Both fastq1 and fastq2 must be defined.");
     }
     if (fq2_path.empty() && !sampleList.empty()) {
         throw std::runtime_error("FASTQ filenames (fastq1, fastq2) = (defined, undefined) and Sample Sheet defined. Set either FASTQ filenames or Sample Sheet");
     }
  };

  SampleSheetMap SampleData;
  std::vector<SampleDetails> SampleInfoVect;
  if (sampleList.empty()){
     SampleDetails SampleInfo;
     SampleInfo.fastqR1 = fq1_path;
     SampleInfo.fastqR2 = fq2_path;
     SampleInfo.ReadGroup = read_group;
     SampleInfo.Platform = platform_id;
     SampleInfo.LibraryID = library_id;
     SampleInfoVect.push_back(SampleInfo);
     SampleData.insert(make_pair(sample_id, SampleInfoVect));
  }else{
     SampleSheet my_sheet(sampleList);
     SampleData=my_sheet.get();
  };

  // start execution
  std::string parts_dir;
  std::string temp_dir = conf_temp_dir + "/align";

  create_dir(temp_dir);

  // check available space in temp dir
  namespace fs = boost::filesystem;

  struct statvfs diskData;
  statvfs(temp_dir.c_str(), &diskData);
  unsigned long long available = (diskData.f_bavail * diskData.f_frsize);
  DLOG(INFO) << available;

  std::string output_path_temp;
  std::string BAMfile;
  for (auto pair : SampleData) {
    std::string sample_id = pair.first;
    std::vector<SampleDetails> list = pair.second;
    size_t size_fastq = 0;
    for (int i = 0; i < list.size(); ++i) {
        fq1_path = list[i].fastqR1;
        fq2_path = list[i].fastqR2;
        // check space occupied by fastq files
        size_fastq += fs::file_size(fq1_path);
        size_fastq += fs::file_size(fq2_path);

        DLOG(INFO) << size_fastq;

        // print error message if there is not enough space in temp_dir
        std::string file_extension;
        file_extension = fs::extension(fq1_path);

        if (file_extension == ".gz" ) {
            size_fastq += 3*fs::file_size(fq1_path);
            size_fastq += 3*fs::file_size(fq2_path);
        }

        if (available < size_fastq) {
            LOG(ERROR) << "Not enough space in temporary storage: "
              << temp_dir << ", the size of the temporary folder should be at least 3 times the input FASTQ files";

            throw silentExit();
        }

    } // Checking FASTQ files sizes ends

    for (int i = 0; i < list.size(); ++i) {
        fq1_path = list[i].fastqR1;
        fq2_path = list[i].fastqR2;
        read_group = list[i].ReadGroup;
        platform_id = list[i].Platform;
        library_id = list[i].LibraryID;

        if (list.size() >1) {
           if (boost::filesystem::exists(output_path) &&
              !boost::filesystem::is_directory(output_path)) {
              throw fileNotFound("Output path '" +
              output_path +
              "' is not a directory");
           }
           parts_dir = output_path + "/" +
           sample_id + "/" +
           read_group;
           // workaround for output check
           if (i == 0) {
              create_dir(output_path + "/" + sample_id);
              output_path_temp = output_path + "/" + sample_id + "/" + sample_id + ".bam";
           }

        } else {

           if (flag_align_only) {
              // Check if output in path already exists but is not a dir
              if (boost::filesystem::exists(output_path) &&
                 !boost::filesystem::is_directory(output_path)) {
                 throw fileNotFound("Output path '" +
                 output_path +
                 "' is not a directory");
              }
              parts_dir = output_path + "/" +
              sample_id + "/" +
              read_group;

              // workaround for output check
              create_dir(output_path+"/"+sample_id);
           }  else {
              if (sampleList.empty()) {
                 // check output path before alignment
                 output_path = check_output(output_path, flag_f, true);

                 // require output to be a file
                 parts_dir = temp_dir + "/" +
                 get_basename(output_path) + ".parts";
              } else {
                 // Check if output in path already exists but is not a dir
                 if (boost::filesystem::exists(output_path) &&
                    !boost::filesystem::is_directory(output_path)) {
                    throw fileNotFound("Output path '" +
                    output_path +
                    "' is not a directory");
                 }
                 // require output to be a file
                 std::string sample_dir = output_path + "/" + sample_id;
                 if (!boost::filesystem::exists(sample_dir)) create_dir(sample_dir);

                 BAMfile = sample_dir + "/" + sample_id + ".bam";;

                 parts_dir = temp_dir + "/" +
                 get_basename(BAMfile) + ".parts";
                 output_path_temp=BAMfile;
              } // Check if sampleList is empty or not

            } // Check if align-only was requested or not
        } // Check if sample has more than one pair of FASTQ files
        DLOG(INFO) << "Putting sorted BAM parts in '" << parts_dir << "'";

        uint64_t start_align = getTs();

        Executor executor("bwa mem");
        Worker_ptr worker(new BWAWorker(ref_path,
             fq1_path, fq2_path,
             parts_dir,
             extra_opts,
             sample_id, read_group,
             platform_id, library_id, flag_f));

        executor.addTask(worker);
        executor.run();

        // Generating Log File for each sample:
        if (!sampleList.empty()) {
            std::string log_filename  = output_path + "/" + sample_id + "/" + sample_id + "_bwa.log";
            std::ofstream outfile;
            outfile.open(log_filename, std::fstream::in | std::fstream::out | std::fstream::app);
            outfile << sample_id << ":" << read_group << ": Start doing bwa mem " << std::endl;
            outfile << sample_id << ":" << read_group << ": bwa mem finishes in " << getTs() - start_align << " seconds" << std::endl;
            outfile.close(); outfile.clear();
        }

    }; // end loop for list.size()
    std::cout << "Alignment Completed " << sample_id << std::endl;

    if (!flag_align_only) {
        std::string temp = output_path;
        if (!sampleList.empty()) output_path = output_path_temp;
        if (list.size() >1) parts_dir = temp + "/" + sample_id;

        uint64_t start_markdup = getTs();
        Executor executor("Mark Duplicates");
        Worker_ptr worker(new MarkdupWorker(parts_dir, output_path, flag_f));
        executor.addTask(worker);
        executor.run();

        output_path = temp;

        if (!sampleList.empty()) {
            std::string log_filename_md  = output_path + "/" + sample_id + "/" + sample_id + "_bwa.log";
            std::ofstream bwa_log;
            bwa_log.open(log_filename_md, std::ofstream::out | std::ofstream::app);
            bwa_log << sample_id << ": " << "Start doing Mark Duplicates " << std::endl;
            bwa_log << sample_id << ": " << "Mark Duplicates finishes in " << getTs() - start_markdup << " seconds" << std::endl;
            bwa_log.close(); bwa_log.clear();
        }

        // Remove parts_dir
        if (list.size() >1) {
            for (int k = 0; k < list.size(); ++k) {
                 parts_dir = temp + "/" + sample_id + "/" + list[k].ReadGroup;
                 remove_path(parts_dir);
                 DLOG(INFO) << "Removing temp file in '" << parts_dir << "'";
            }
        } else {
            remove_path(parts_dir);
            DLOG(INFO) << "Removing temp file in '" << parts_dir << "'";
        }

    } else {
        // Merging parts BAM Files if align_only is set:
        // create sambamba command to merge parts BAM files
        std::stringstream cmd;
        cmd << get_config<std::string>("sambamba_path") << " merge "
            << "-l 1 "
            << "-t " << get_config<int>("markdup.nt") << " ";
        std::string mergeBAM = output_path + "/" + sample_id + "/" + sample_id + ".bam";
        cmd << mergeBAM;
        for (int m = 0; m < list.size(); m++) {
             parts_dir = output_path + "/" + sample_id + "/" + list[m].ReadGroup;
             std::vector<std::string> input_files_ ;
             get_input_list(parts_dir, input_files_, ".*/part-[0-9].*", true);
             for (int n = 0; n < input_files_.size(); n++) {
                  cmd << input_files_[n] << " ";
             }
         }
         DLOG(INFO) << "Merging Parts BAM Files for " << sample_id << std::endl;
         cmd_ = cmd.str();
         DLOG(INFO) << cmd_ << std::endl;

    }


  }; // end loop for Index Map
  return 0;
}
} // namespace fcsgenome
