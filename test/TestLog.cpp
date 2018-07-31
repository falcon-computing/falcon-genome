#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <gtest/gtest.h>

#include "fcs-genome/LogUtils.h"
#include "fcs-genome/common.h"
#include "fcs-genome/config.h"

class TestLog : public ::testing::Test {
  ;
};

// helper function to get a log from a file or folder
std::string get_log(std::string path) {

  namespace bfs = boost::filesystem;
  std::vector<std::string> logs_;
  bfs::path p(path);

  if (bfs::is_directory(p)) {
    for (bfs::directory_iterator i(p); i != bfs::directory_iterator(); ++i) {
      DLOG(INFO) << "processing " << i->path();
      logs_.push_back(i->path().string());
    }
  }
  else {
    logs_.push_back(path);
  }
  fcsgenome::LogUtils logUtils;
  std::string output = logUtils.findError(logs_);

  return output;
}

TEST_F(TestLog, TestNormal) {
  ASSERT_TRUE(get_log(fcsgenome::get_bin_dir() + 
        "/resource/normal-gatk.log").empty());

  ASSERT_TRUE(get_log(fcsgenome::get_bin_dir() + 
        "/resource/normal-bwa.log").empty());
}

TEST_F(TestLog, TestBwaLog ) {
  std::string output = get_log(fcsgenome::get_bin_dir() + 
        "/resource/error-bwa.log");
  std::string target = "[E::mem_pestat] fail to locate the index files\n";

  ASSERT_FALSE(output.empty());
  ASSERT_EQ(target, output);
}

TEST_F(TestLog, TestGATKLog) {
  std::string output = get_log(fcsgenome::get_bin_dir() + 
        "/resource/gatk-error1");
  ASSERT_FALSE(output.empty());

  std::string target(
"##### ERROR ------------------------------------------------------------------------------------------\n"
"##### ERROR A USER ERROR has occurred (version 3.7-2-g53263cf): \n" 
"##### ERROR\n" 
"##### ERROR This means that one or more arguments or inputs in your command are incorrect.\n" 
"##### ERROR The error message below tells you what is the problem.\n" 
"##### ERROR\n" 
"##### ERROR If the problem is an invalid argument, please check the online documentation guide\n" 
"##### ERROR (or rerun your command with --help) to view allowable command-line arguments for this tool.\n" 
"##### ERROR\n" 
"##### ERROR Visit our website and forum for extensive documentation and answers to \n" 
"##### ERROR commonly asked questions https://software.broadinstitute.org/gatk\n" 
"##### ERROR\n"  
"##### ERROR Please do NOT post this error to the GATK forum unless you have really tried to fix it yourself.\n" 
"##### ERROR\n" 
"##### ERROR MESSAGE: Invalid command line: Cannot process the provided BAM/CRAM file(s) because they were not indexed.  The GATK does offer limited processing of unindexed BAM/CRAMs in --unsafe mode, but this feature is unsupported -- use it at your own risk!\n" 
"##### ERROR ------------------------------------------------------------------------------------------\n");
  ASSERT_STREQ(output.c_str(), target.c_str());
}
