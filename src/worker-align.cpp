#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <sys/statvfs.h>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/workers.h"
#include "fcs-genome/SampleSheet.h"

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

  // finalize argument parsing
  po::notify(cmd_vm);

  if (fq1_path.empty()) {
     if (fq2_path.empty() && sampleList.empty()) {
         throw std::runtime_error("FASTQ filenames and Sample Sheet cannot be undefined at the same time");
     }
     if (!fq2_path.empty() && sampleList.empty()) {
         throw std::runtime_error("FASTQ filename R1 undefined and R2 defined");
     }
  } else {
     if (fq2_path.empty() && sampleList.empty()) {
         throw std::runtime_error("FASTQ filename R1 defined and R2 undefined");
     }
     if (!fq2_path.empty() && !sampleList.empty()) {
         throw std::runtime_error("FASTQ filenames and Sample Sheet cannot be defined at the same time");
     }
  };

  // finalize argument parsing
  //po::notify(cmd_vm);

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

  for (auto pair : SampleData) {
    std::string sample_id = pair.first;
    std::vector<SampleDetails> list = pair.second;
    for (int i = 0; i < list.size(); ++i) {
        fq1_path = list[i].fastqR1;
        fq2_path = list[i].fastqR2;
        read_group = list[i].ReadGroup;
        platform_id = list[i].Platform;
        library_id = list[i].LibraryID;

        // check space occupied by fastq files
        size_t size_fastq = 0;
        size_fastq += fs::file_size(fq1_path);
        size_fastq += fs::file_size(fq2_path);

        DLOG(INFO) << size_fastq;

        // print error message if there is not enough space in temp_dir
        std::string file_extension;
        file_extension = fs::extension(fq1_path);

        int threshold;
        if (file_extension == ".gz")
            threshold = 3;
        else
            threshold = 1;

        if (available < threshold * size_fastq) {
            LOG(ERROR) << "Not enough space in temporary storage: "
               << temp_dir << ", the size of the temporary folder should be at least "
               << threshold << " times the input FASTQ files";

            throw silentExit();
        }

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
        }
        else {
           // check output path before alignment
           output_path = check_output(output_path, flag_f, true);

           // require output to be a file
           parts_dir = temp_dir + "/" +
           get_basename(output_path) + ".parts";
        }
        DLOG(INFO) << "Putting sorted BAM parts in '" << parts_dir << "'";

        Executor executor("bwa mem");

        Worker_ptr worker(new BWAWorker(ref_path,
             fq1_path, fq2_path,
             parts_dir,
             extra_opts,
             sample_id, read_group,
             platform_id, library_id, flag_f));

        executor.addTask(worker);
        executor.run();
    };
    if (!flag_align_only) {
        Executor executor("Mark Duplicates");
        Worker_ptr worker(new MarkdupWorker(parts_dir, output_path, flag_f));
        executor.addTask(worker);
        executor.run();

        // Remove parts_dir
        remove_path(parts_dir);
        DLOG(INFO) << "Removing temp file in '" << parts_dir << "'";
    }
  };
  return 0;
}
} // namespace fcsgenome
