#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <unordered_set>
#include <gtest/gtest.h>

#include <stdlib.h>

#include "fcs-genome/BackgroundExecutor.h"
#include "fcs-genome/config.h"
#include "fcs-genome/common.h"
#include "fcs-genome/Executor.h"
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
  sleep(1);
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
  sleep(1);
  touch(inputIndex2);
  try {
    fcs::check_vcf_index(inputVCF2);
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
  CHECK_NOEXCEPTION;
  
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
