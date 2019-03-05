#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <gtest/gtest.h>

#include <stdlib.h>

#include "fcs-genome/BackgroundExecutor.h"
#include "fcs-genome/config.h"
#include "fcs-genome/common.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/workers/BWAWorker.h"
#include "fcs-genome/workers/BQSRWorker.h"
#include "fcs-genome/workers/SambambaWorker.h"

namespace fcs = fcsgenome;
namespace fs = boost::filesystem;

class TestWorker : public ::testing::Test {
  ;
};

void touch(std::string path) {
  DLOG(INFO) << "touching " << path;
  fs::ofstream f(path);
  f.close();
}

#define CHECK_EXCEPTION try { \
  worker.check(); \
  FAIL() << "Expecting exception to be thrown"; \
  } \
  catch (...) {				\
  ; \
  }

#define CHECK_NOEXCEPTION try { \
  worker.check(); \
  } catch (...) { \
  FAIL() << "Not expecting exception to be thrown"; \
  }

#define CHECK_SETUP_EXCEPTION try { \
  worker.setup(); \
  FAIL() << "Expecting exception to be thrown";	\
  } \
  catch (...) { \
  ;	\
  }

#define CHECK_SETUP_NOEXCEPTION try { \
  worker.setup(); \
  } catch (...) { \
  FAIL() << "Not expecting exception to be thrown"; \
  }

TEST_F(TestWorker, Testing_check_vcf_index) {
  std::string temp_dir = "/tmp/fcs-genome-test-" +  std::to_string((long long)fcs::getTid());
  fcs::create_dir(temp_dir);
  std::string inputVCF   = temp_dir + "/input.vcf.gz";
  std::string inputIndex = temp_dir + "/input.vcf.gz.tbi";
  touch(inputIndex);
  touch(inputVCF);  
  try {
    fcs::check_vcf_index(inputVCF);
  }
  catch ( ... ){
    FAIL() << "VCF index was not checked";
  }  

  std::string inputVCF2   = temp_dir + "/input.vcf";
  std::string inputIndex2 = temp_dir + "/input.vcf.idx";
  touch(inputVCF2);
  touch(inputIndex2);
  try {
    fcs::check_vcf_index(inputVCF);
  }
  catch ( ... ){
    FAIL() << "VCF index was not checked";
  }
  fcs::remove_path(temp_dir);
}

TEST_F(TestWorker, Testing_get_input_list) {

  // Case: Folder with parts BAM with no extension:   
  std::string temp_dir = "/tmp/fcs-genome-test-" +  std::to_string((long long)fcs::getTid());
  fcs::create_dir(temp_dir + "/dirBAM/");
  fcs::create_dir(temp_dir + "/dirBAM/RG1/");
  touch(temp_dir + "/dirBAM/RG1/" + "part-0000");
  touch(temp_dir + "/dirBAM/RG1/" + "part-0001");  

  std::unordered_set<std::string> bam_set;

  std::vector<std::string> bam_list;
  std::string input  = temp_dir + "/dirBAM/";  
  try {
    fcs::get_input_list(input, bam_list, ".*/part-[0-9].*.*", true);
  }
  catch ( ... ){
    FAIL() << "Parts BAM with no extension may not be in array or input is incorrect";
  }
  for (auto bam : bam_list) {
    bam_set.insert(bam);
  }
  ASSERT_TRUE(bam_set.count(temp_dir + "/dirBAM/RG1/part-0000"));
  ASSERT_TRUE(bam_set.count(temp_dir + "/dirBAM/RG1/part-0001"));
  ASSERT_EQ(2, bam_list.size());

  bam_list.clear();
  bam_set.clear();
  fcs::remove_path(temp_dir);

  // Case: Folder with parts BAM with .bam extension:
  temp_dir = "/tmp/fcs-genome-test-" +  std::to_string((long long)fcs::getTid());
  fcs::create_dir(temp_dir + "/dirBAM/");
  fcs::create_dir(temp_dir + "/dirBAM/RG1/");
  touch(temp_dir + "/dirBAM/RG1/" + "part-0000.bam");
  touch(temp_dir + "/dirBAM/RG1/" + "part-0001.bam");
  try {
    fcs::get_input_list(input, bam_list, ".*/part-[0-9].*.*", true);
  }
  catch ( ... ){
    FAIL() << "Parts BAM with .bam extension may not be in array or input is incorrect";
  }
  for (auto bam : bam_list) {
    bam_set.insert(bam);
  }
  ASSERT_TRUE(bam_set.count(temp_dir + "/dirBAM/RG1/part-0000.bam"));
  ASSERT_TRUE(bam_set.count(temp_dir + "/dirBAM/RG1/part-0001.bam"));
  ASSERT_EQ(2, bam_list.size());

  bam_list.clear();
  bam_set.clear();
  fcs::remove_path(temp_dir);

  // Case : Folder is empty:
  temp_dir = "/tmp/fcs-genome-test-" +  std::to_string((long long)fcs::getTid());
  fcs::create_dir(temp_dir + "/dirBAM/");
  fcs::create_dir(temp_dir + "/dirBAM/RG1/");
  try {
    fcs::get_input_list(input, bam_list, ".*/part-[0-9].*.*", true);
  }
  catch (fcs::fileNotFound &e){
    EXPECT_EQ(e.what(), "Cannot find input file(s) in '" + input + "'");
  }
  catch (...) {
    FAIL() << "Expected filleNotEmpty()";
  }
  bam_list.clear();
  bam_set.clear();
  fcs::remove_path(temp_dir);

}

TEST_F(TestWorker, Test_check_output) {
  bool flag = true;
  bool flag_f = true;
  std::string temp_dir = "/tmp/fcs-genome-test-" +  std::to_string((long long)fcs::getTid());
  fcs::create_dir(temp_dir);
  std::string input  = temp_dir + "/";
  std::string outputBAI = temp_dir + "/" + "output.bam.bai";

  try {
    fcs::check_output(outputBAI, flag_f, flag);
  }
  catch (fcs::fileNotFound &e){
    EXPECT_EQ(e.what(), "Cannot write to output path '"+ outputBAI +"'"); 
  }

  fcs::remove_path(temp_dir);
}

TEST_F(TestWorker, SambambaWorkerActions) {
  std::string temp_dir = "/tmp/fcs-genome-test-" +  std::to_string((long long)fcs::getTid());
  fcs::create_dir(temp_dir);
  
  // Empty Folder: 
  std::string input  = temp_dir + "/";
  std::string output = temp_dir + "/" + "marked.bam";
  bool flag = true;

  std::string dirs[2]={input, input + "/dirBAM",  };

  fcs::create_dir(temp_dir + "/dirBAM/RG1/");                                                                                                                           
  touch(temp_dir + "/dirBAM/RG1/" + "part-0000");                                                                                                                       
  touch(temp_dir + "/dirBAM/RG1/" + "part-0001");                                                                                                                       
  fcs::create_dir(temp_dir + "/dirBAM/RG2");                                                                                                                            
  touch(temp_dir + "/dirBAM/RG2/" + "part-0000.bam");   
  touch(temp_dir + "/dirBAM/RG2/" + "part-0001.bam"); 

  fcs::SambambaWorker::Action myAction[2]={fcs::SambambaWorker::MARKDUP,fcs::SambambaWorker::MERGE};

  for (int i=0; i<2;i++) {
    for (int j=0; j<2;j++) {
     fcs::SambambaWorker worker(dirs[i], output, myAction[j], ".*/part-[0-9].*.*",flag);
     // Check Setup:
     CHECK_SETUP_NOEXCEPTION;
    }
  }
  fcs::remove_path(temp_dir);

}

TEST_F(TestWorker, TestBQSRWorker_check) {
  std::string temp_dir = "/tmp/fcs-genome-test-" +  std::to_string((long long)fcs::getTid());
  fcs::create_dir(temp_dir);

  // common input
  std::string ref    = temp_dir + "/" + "ref.fasta";
  std::string intv   = temp_dir + "/" + "intv.list";
  std::string input  = temp_dir + "/" + "input.bam";
  std::string input_bai = temp_dir + "/" + "input.bai";
  std::string output = temp_dir + "/" + "output.bam";
  std::vector<std::string> known;
  known.push_back(temp_dir + "/" + "known1.vcf");
  bool flag = true;
  bool flag_gatk4 = true;

  std::vector<std::string> interval;
  interval.push_back(intv);

  // create all files
  touch(ref);
  touch(intv);
  touch(input);
  touch(input_bai);
  touch(known[0]);
  touch(known[0]+".idx");

  fcs::BQSRWorker worker(ref, known, interval, input, output, std::vector<std::string>(), 0, flag, flag_gatk4);
  
  // no exception should be thrown
  CHECK_NOEXCEPTION;

  // test index
  boost::this_thread::sleep_for(boost::chrono::seconds(1));
  
  touch(known[0]); // now known will be older than idx
  CHECK_EXCEPTION;
  
  // test no index exception
  known.push_back(temp_dir + "/" + "known2.vcf.gz");
  {
     output = temp_dir + "/" + "output2.bam";
     fcs::BQSRWorker worker(ref, known, interval, input, output, std::vector<std::string>(), 0, flag, flag_gatk4);
     touch(known[1]);
     CHECK_EXCEPTION;
     touch(known[1] + ".idx");
     CHECK_EXCEPTION;
     touch(known[1] + ".tbi");
     CHECK_NOEXCEPTION;
  }
  
  // test wrong known site exception
  known.push_back(temp_dir + "/" + "known3.vcf1");
  {
     output = temp_dir + "/" + "output3.bam";
     fcs::BQSRWorker worker(ref, known, interval, input, output, std::vector<std::string>(), 0, flag, flag_gatk4);
     CHECK_EXCEPTION;
  }

  fcs::remove_path(temp_dir);
}

TEST_F(TestWorker, TestBWAWorker_check) {
  std::string temp_dir = "/tmp/fcs-genome-test-" +  std::to_string((long long)fcs::getTid());
  fcs::create_dir(temp_dir);
  std::string ref_path = temp_dir + "/" + "ref.fasta";
  std::string fq1_path = temp_dir + "/" + "fastq_1.gz";
  std::string fq2_path = temp_dir + "/" + "fastq_2.gz";
  std::string partdir_path = temp_dir + "/" + "part_dir";
  std::string output_path = temp_dir + "/" + "output";
  std::vector<std::string> extra_opts;
  std::string sample_id = "id";
  std::string read_group = "rg";
  std::string platform_id = "pl";
  std::string library_id = "lb";
  bool flag_align_only = false;
  bool flag_merge_bams = false;
  bool flag_f = false;
  
  bool flag_true = true;

  {
    fcs::BWAWorker worker(ref_path, fq1_path, fq2_path, 
      partdir_path, output_path, extra_opts, sample_id, 
      read_group, platform_id, library_id, flag_align_only,
      flag_merge_bams, flag_f);
    CHECK_EXCEPTION;
  }

  {
    touch(ref_path);
    touch(fq1_path);
    touch(fq2_path);
    fcs::BWAWorker worker(ref_path, fq1_path, fq2_path, 
      partdir_path, output_path, extra_opts, sample_id, 
      read_group, platform_id, library_id, flag_align_only,
      flag_merge_bams, flag_f);
    CHECK_NOEXCEPTION;
  }

  // check for partdir_path
  
  touch(partdir_path);
  {
    fcs::BWAWorker worker(ref_path, fq1_path, fq2_path, 
      partdir_path, output_path, extra_opts, sample_id, 
      read_group, platform_id, library_id, flag_align_only,
      flag_merge_bams, flag_true);
    CHECK_NOEXCEPTION;
  }
  fcs::remove_path(partdir_path);

  // check for output_path
  touch(output_path);
  {
    fcs::BWAWorker worker(ref_path, fq1_path, fq2_path, 
      partdir_path, output_path, extra_opts, sample_id, 
      read_group, platform_id, library_id, flag_align_only,
      flag_merge_bams, flag_f);
    CHECK_NOEXCEPTION;
  }
  {
    fcs::BWAWorker worker(ref_path, fq1_path, fq2_path, 
      partdir_path, output_path, extra_opts, sample_id, 
      read_group, platform_id, library_id, flag_align_only,
      flag_true, flag_true);
    CHECK_NOEXCEPTION;
  }
  fcs::remove_path(output_path);

  // check for bam header values
  sample_id = "";
  {
    fcs::BWAWorker worker(ref_path, fq1_path, fq2_path, 
      partdir_path, output_path, extra_opts, sample_id, 
      read_group, platform_id, library_id, flag_align_only,
      flag_merge_bams, flag_f);
    CHECK_EXCEPTION;
  }
  sample_id = "id";

  read_group = "";
  {
    fcs::BWAWorker worker(ref_path, fq1_path, fq2_path, 
      partdir_path, output_path, extra_opts, sample_id, 
      read_group, platform_id, library_id, flag_align_only,
      flag_merge_bams, flag_f);
    CHECK_EXCEPTION;
  }
  read_group = "rg";

  platform_id = "";
  {
    fcs::BWAWorker worker(ref_path, fq1_path, fq2_path, 
      partdir_path, output_path, extra_opts, sample_id, 
      read_group, platform_id, library_id, flag_align_only,
      flag_merge_bams, flag_f);
    CHECK_EXCEPTION;
  }
  platform_id = "pl";

  library_id = "";
  {
    fcs::BWAWorker worker(ref_path, fq1_path, fq2_path, 
      partdir_path, output_path, extra_opts, sample_id, 
      read_group, platform_id, library_id, flag_align_only,
      flag_merge_bams, flag_f);
    CHECK_EXCEPTION;
  }
  library_id = "lb";
}

TEST_F(TestWorker, TestBWAWorker_setup) {
  //std::cout << "doing TestBWAWorker_setup tests" << std::endl;
  std::string temp_dir = "/tmp/fcs-genome-test-" +  std::to_string((long long)fcs::getTid());
  fcs::create_dir(temp_dir);
  std::string ref_path = temp_dir + "/" + "ref.fasta";
  std::string fq1_path = temp_dir + "/" + "fastq_1.gz";
  std::string fq2_path = temp_dir + "/" + "fastq_2.gz";
  std::string partdir_path = temp_dir + "/" + "part_dir";
  std::string output_path = temp_dir + "/" + "output";
  std::vector<std::string> extra_opts;
  std::string sample_id = "id";
  std::string read_group = "rg";
  std::string platform_id = "pl";
  std::string library_id = "lb";
  bool flag_align_only = false;
  bool flag_merge_bams = false;
  bool flag_f = false;

  bool flag_c;

  // only do test on non-scaleout and non-latency mode
  if (!fcs::get_config<bool>("bwa.scaleout_mode") &&
      !fcs::get_config<bool>("latency_mode")) {
    return;
  }

  fcs::BWAWorker worker(ref_path, fq1_path, fq2_path, 
    partdir_path, output_path, extra_opts, sample_id, 
    read_group, platform_id, library_id, flag_align_only,
    flag_merge_bams, flag_f);

  CHECK_NOEXCEPTION
  CHECK_SETUP_NOEXCEPTION

  std::stringstream ss;
  ss.str("");
  ss << "bwa-fow mem " 
     << "-R \"@RG\\tID:" << "rg" << 
                "\\tSM:" << "id" << 
                "\\tPL:" << "pl" << 
                "\\tLB:" << "lb" << "\" "
     << "--logtostderr "
     << "--offload "
     << "--output_flag=1 "
     << "--chunk_size=2000 "
     << "--v=" << fcs::get_config<int>("bwa.verbose") << " "
     << "--temp_dir=\"" << temp_dir << "/" << "part_dir" << "\" "
     << "--output=\"" << temp_dir << "/" << "output" << "\" " 
     << "--merge_bams=" << "0" << " "; 
  
  flag_c = worker.getCommand().find(ss.str()) != std::string::npos;
  ASSERT_TRUE(flag_c);

  if (fcs::get_config<int>("bwa.nt") > 0) {
    ss.str("");
    ss << "--t=" << fcs::get_config<int>("bwa.nt");
    flag_c = worker.getCommand().find(ss.str()) != std::string::npos;
    ASSERT_TRUE(flag_c);
  }

  ss.str("");
  ss << "--disable_markdup=true";
  flag_c = worker.getCommand().find(ss.str()) == std::string::npos;
  ASSERT_TRUE(flag_c);
 
  if (fcs::get_config<bool>("bwa.enforce_order")) {
    ss.str(""); 
    ss << "--inorder_output";
    flag_c = worker.getCommand().find(ss.str()) != std::string::npos;
    ASSERT_TRUE(flag_c);
  }

  if (fcs::get_config<bool>("bwa.use_fpga") &&
      !fcs::get_config<std::string>("bwa.fpga.bit_path").empty())
  {
    ss.str("");
    ss  << "--use_fpga "
        << "--fpga_path=" << fcs::get_config<std::string>("bwa.fpga.bit_path") << " ";
    flag_c = worker.getCommand().find(ss.str()) != std::string::npos;
    ASSERT_TRUE(flag_c);
  }

  ss.str("");
  ss << temp_dir << "/" << "ref.fasta"
     << temp_dir << "/" << "fastq_1.gz"
     << temp_dir << "/" << "fastq_2.gz";
  flag_c = worker.getCommand().find(ss.str()) != std::string::npos;
  ASSERT_TRUE(flag_c);

  ss.str("");
}

TEST_F(TestWorker, TestSambambaWorker_check) {
  std::string temp_dir = "/tmp/fcs-genome-test-" +  std::to_string((long long)fcs::getTid());
  fcs::create_dir(temp_dir);
  std::string input_path = temp_dir + "/" + "input.bam";
  std::string output_path = temp_dir + "/" + "output.bam";
  fcs::SambambaWorker::Action action = fcs::SambambaWorker::INDEX;
  std::string common = ".*";
  bool flag_f;
  std::vector<std::string> files;

  // check if input file path exits
  {
    fcs::SambambaWorker worker(input_path, output_path, action,
                              common, flag_f, files);
    CHECK_EXCEPTION
  }
  touch(input_path);
  {
    fcs::SambambaWorker worker(input_path, output_path, action,
                              common, flag_f, files);
    CHECK_NOEXCEPTION
  }  
  fcs::remove_path(input_path);

  // check files vector
  std::string input1 = temp_dir + "/" + "input1.bam";
  std::string input2 = temp_dir + "/" + "input2.bam";
  std::string input3 = temp_dir + "/" + "input3.bam";
  files.push_back(input1);
  files.push_back(input2);
  files.push_back(input3);
  {
    fcs::SambambaWorker worker(input_path, output_path, action,
                              common, flag_f, files);
    CHECK_EXCEPTION
  }
  touch(input1);
  touch(input2);
  touch(input3);
  {
    fcs::SambambaWorker worker(input_path, output_path, action,
                              common, flag_f, files);
    
    CHECK_EXCEPTION
  }
  std::vector<std::string>().swap(files); // for clear vector

  // test the common regex
  input_path = temp_dir + "/" + "input";
  std::string temp1 = temp_dir + "/" + "input/" + "dododododotest1ppp.bbb.bam";
  std::string temp2 = temp_dir + "/" + "input/" + "skyskyskyskytest1lklk...klk.bam";
  std::string temp3 = temp_dir + "/" + "input/" + "WahahaWWExited...test1.In.tere.s.ting.bam";
  std::string temp4 = temp_dir + "/" + "input/" + "WahahaWWExited...test1.In.tere.s.ting";
  std::string temp5 = temp_dir + "/" + "input/" + "test1.bam";
  std::string temp6 = temp_dir + "/" + "input/" + "test2.bam";
  std::string temp7 = temp_dir + "/" + "input/" + "skyskyskyskytest2lklk...klk.bam";
  std::string temp8 = temp_dir + "/" + "input/" + "sdasdshshtest1sdaksdtest1dasda....";
  common = ".*/.*test1.*\\.bam$";
  fcs::create_dir(input_path);
  touch(temp1);
  touch(temp2);
  touch(temp3);
  touch(temp4);
  touch(temp5);
  touch(temp6);
  touch(temp7);
  touch(temp8);
  {
    fcs::SambambaWorker worker(input_path, output_path, action,
                              common, flag_f, files);
    CHECK_NOEXCEPTION
    ASSERT_EQ(4, worker.get_files().size());
  }
  files.push_back(input1);
  {
    fcs::SambambaWorker worker(input_path, output_path, action,
                              common, flag_f, files);
    CHECK_NOEXCEPTION
    ASSERT_EQ(5, worker.get_files().size());
  }
  fcs::remove_path(temp3);
  {
    fcs::SambambaWorker worker(input_path, output_path, action,
                              common, flag_f, files);
    CHECK_NOEXCEPTION
    ASSERT_EQ(4, worker.get_files().size());
  } 
  fcs::remove_path(temp6);
  {
    fcs::SambambaWorker worker(input_path, output_path, action,
                              common, flag_f, files);
    CHECK_NOEXCEPTION
    ASSERT_EQ(4, worker.get_files().size());
  }

  fcs::remove_path(temp1);
  fcs::remove_path(temp2);
  fcs::remove_path(temp3);
  fcs::remove_path(temp4);
  fcs::remove_path(temp5);
  fcs::remove_path(temp6);
  fcs::remove_path(temp7);
  fcs::remove_path(temp8);
  fcs::remove_path(input1);
  fcs::remove_path(input2);
  fcs::remove_path(input3);
}

TEST_F(TestWorker, TestSambambaWorker_setup) {
  std::string temp_dir = "/tmp/fcs-genome-test-" +  std::to_string((long long)fcs::getTid());
  fcs::create_dir(temp_dir);
  std::string input_path;
  std::string output_path;
  fcs::SambambaWorker::Action action;
  std::string common = ".*\\.bam$";
  bool flag_f;
  std::vector<std::string> files;
  bool flag_c;

  // check for Markdup
  {
    action = fcs::SambambaWorker::MARKDUP;
    input_path = temp_dir + "/" + "input.bam";
    output_path = temp_dir + "/" + "output.bam";
    flag_f = true;
    files.clear();
    touch(input_path);
    touch(output_path);
    fcs::SambambaWorker worker(input_path, output_path, action,
                              common, flag_f, files);
    CHECK_NOEXCEPTION
    CHECK_SETUP_NOEXCEPTION

    std::stringstream ss;
    ss.str("");
    ss << input_path;
    flag_c = worker.getCommand().find(ss.str()) != std::string::npos;
    ASSERT_FALSE(flag_c);
    ss.str("");

    ss << output_path;
    flag_c = worker.getCommand().find(ss.str()) != std::string::npos;
    ASSERT_TRUE(flag_c);
    ss.str("");

    ss << "--overflow-list-size=" << fcs::get_config<int>("markdup.overflow-list-size");
    flag_c = worker.getCommand().find(ss.str()) != std::string::npos;
    ASSERT_TRUE(flag_c);
    ss.str("");

    ss << "--tmpdir=" << fcs::get_config<std::string>("temp_dir");
    flag_c = worker.getCommand().find(ss.str()) != std::string::npos;
    ASSERT_TRUE(flag_c);
    ss.str("");    

    ss << "-l 1 " << "-t " << fcs::get_config<int>("markdup.nt");
    flag_c = worker.getCommand().find(ss.str()) != std::string::npos;
    ASSERT_TRUE(flag_c);
    ss.str("");

    fcs::remove_path(input_path);
    fcs::remove_path(output_path);
  }

  {
    action = fcs::SambambaWorker::MARKDUP;
    input_path = temp_dir + "/" + "input";
    output_path = temp_dir + "/" + "output.bam";
    std::string temp1 = input_path + "/test1.bam";
    std::string temp2 = input_path + "/test2.bam";
    std::string temp3 = input_path + "/skdksjaksdlajsdkjalsd.bam.bai";
    std::string temp4 = input_path + "/sk.dk.sj.ak.sd.laj.sdbam";
    fcs::create_dir(input_path);
    touch(output_path);
    touch(temp1);
    touch(temp2);
    touch(temp3);
    touch(temp4);
    flag_f = true;
    files.clear();

    {
      fcs::SambambaWorker worker(input_path, output_path, action,
                                common, flag_f, files);
      CHECK_NOEXCEPTION
      CHECK_SETUP_NOEXCEPTION

      ASSERT_EQ(2, worker.get_files().size());
    }
    {
      fcs::remove_path(temp1);
      fcs::SambambaWorker worker(input_path, output_path, action,
                                common, flag_f, files);
      CHECK_NOEXCEPTION
      CHECK_SETUP_NOEXCEPTION 

      ASSERT_EQ(1, worker.get_files().size());
    }
    {
      fcs::remove_path(temp3);
      fcs::SambambaWorker worker(input_path, output_path, action,
                                common, flag_f, files);
      CHECK_NOEXCEPTION
      CHECK_SETUP_NOEXCEPTION 

      ASSERT_EQ(1, worker.get_files().size());
    }
    {
      fcs::remove_path(temp4);
      fcs::SambambaWorker worker(input_path, output_path, action,
                                common, flag_f, files);
      CHECK_NOEXCEPTION
      CHECK_SETUP_NOEXCEPTION 

      ASSERT_EQ(1, worker.get_files().size());
    }
    {
      fcs::remove_path(temp2);
      fcs::SambambaWorker worker(input_path, output_path, action,
                                common, flag_f, files);
      CHECK_EXCEPTION // no input files, give error

    }

    fcs::remove_path(input_path);
    fcs::remove_path(output_path);
  }

  // check for MERGE
  {
    action = fcs::SambambaWorker::MERGE;
    input_path = temp_dir + "/" + "input";
    output_path = temp_dir + "/" + "output.bam";
    flag_f = true;
    files.clear();
    fcs::create_dir(input_path);
    touch(output_path);
    std::string temp1 = input_path + "/test1.bam";
    std::string temp2 = input_path + "/test2.bam";
    touch(temp1);
    touch(temp2);
    fcs::SambambaWorker worker(input_path, output_path, action,
                                common, flag_f, files);
    CHECK_NOEXCEPTION
    CHECK_SETUP_NOEXCEPTION

    std::stringstream ss;
    ss.str("");
    ss << temp1;
    flag_c = worker.getCommand().find(ss.str()) != std::string::npos;
    ASSERT_TRUE(flag_c);
    ss.str("");

    ss << temp2;
    flag_c = worker.getCommand().find(ss.str()) != std::string::npos;
    ASSERT_TRUE(flag_c);
    ss.str("");

    ss << output_path;
    flag_c = worker.getCommand().find(ss.str()) != std::string::npos;
    ASSERT_TRUE(flag_c);
    ss.str("");

    ss << "-l 1 " << "-t " << fcs::get_config<int>("mergebam.nt");
    flag_c = worker.getCommand().find(ss.str()) != std::string::npos;
    ASSERT_TRUE(flag_c);
    ss.str("");
    
    fcs::remove_path(temp1);
    fcs::remove_path(temp2);
    fcs::remove_path(input_path);
    fcs::remove_path(output_path);
  }

  {
    action = fcs::SambambaWorker::MERGE;
    input_path = temp_dir + "/" + "input";
    output_path = temp_dir + "/" + "output.bam";
    flag_f = true;
    files.clear();
    fcs::create_dir(input_path);
    fcs::create_dir(temp_dir + "/" + "input2");
    touch(output_path);
    std::string temp1 = temp_dir + "/" + "input2" + "/test1.bam";
    std::string temp2 = temp_dir + "/" + "input2" + "/test2.bam";
    std::string temp3 = temp_dir + "/" + "input2" + "/test3.bam";
    std::string temp4 = temp_dir + "/" + "input" + "/test1.bam";
    std::string temp5 = temp_dir + "/" + "input" + "/test2.bam";
    touch(temp1);
    touch(temp2);
    touch(temp3);
    touch(temp4);
    touch(temp5);
    files.push_back(temp1);
    files.push_back(temp2);
    {
      fcs::SambambaWorker worker(input_path, output_path, action,
                                  common, flag_f, files);
      CHECK_NOEXCEPTION
      CHECK_SETUP_NOEXCEPTION
      ASSERT_EQ(4, worker.get_files().size());
    }

    fcs::remove_path(temp1);
    {
      fcs::SambambaWorker worker(input_path, output_path, action,
                                  common, flag_f, files);
      CHECK_EXCEPTION // one input file in files is missing
    }

    fcs::remove_path(temp2);
    fcs::remove_path(temp3);
    fcs::remove_path(temp4);
    fcs::remove_path(temp5);
    fcs::remove_path(input_path);
    fcs::remove_path(output_path);
  }

  // check for INDEX
  {
    action = fcs::SambambaWorker::INDEX;
    input_path = temp_dir + "/" + "input.bam";
    output_path = "";
    flag_f = true;
    files.clear();
    touch(input_path);
    touch(output_path);

    fcs::SambambaWorker worker(input_path, output_path, action,
                                  common, flag_f, files);
    CHECK_NOEXCEPTION
    CHECK_SETUP_NOEXCEPTION

    std::stringstream ss;
    ss.str("");
    ss << input_path;
    flag_c = worker.getCommand().find(ss.str()) != std::string::npos;
    ASSERT_TRUE(flag_c);
    ss.str("");
  }

  // check for SORT
  {
    action = fcs::SambambaWorker::SORT;
    input_path = temp_dir + "/" + "input.bam";
    output_path = temp_dir + "/" + "output.bam";
    flag_f = true;
    files.clear();
    touch(input_path);
    touch(output_path);

    fcs::SambambaWorker worker(input_path, output_path, action,
                                  common, flag_f, files);
    CHECK_NOEXCEPTION
    CHECK_SETUP_NOEXCEPTION

    std::stringstream ss;
    ss.str("");
    ss << input_path;
    flag_c = worker.getCommand().find(ss.str()) != std::string::npos;
    ASSERT_TRUE(flag_c);
    ss.str("");

    ss << output_path;
    flag_c = worker.getCommand().find(ss.str()) != std::string::npos;
    ASSERT_TRUE(flag_c);
    ss.str("");
  }

}
