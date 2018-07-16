#include <iostream>
#include <fstream>
#include <string>
#include <gtest/gtest.h>

class TestLog : public ::testing::Test {
  ;
};

TEST_F(TestLog, TestGatkLog ){
  std::vector<std::string> logs_;
  logs_.push_back("./resource/normal-gatk.log");
  string output;
  ASSERT_FALSE(fcs::findError(logs_, output));
  logs_.push_back("./resource/error-gatk.log");
  string target = "ERROR ------------------------------------------------------------------------------------------\nERROR A GATK RUNTIME ERROR has occurred (version 3.7-0-gcfedb67):\nERROR\nERROR This might be a bug. Please check the documentation guide to see if this is a known problem.\nERROR If not, please post the error message, with stack trace, to the GATK forum.\nERROR Visit our website and forum for extensive documentation and answers to\nERROR commonly asked questions https://software.broadinstitute.org/gatk\nERROR\nERROR MESSAGE: For input string: "0.000"\nERROR ------------------------------------------------------------------------------------------"
  ASSERT_TRUE(fcs::findError(logs_, output));
  ASSERT_EQ(target, output); 
}

TEST_F(TestLog, TestBwaLog ){
  std::vector<std::string> logs_;
  logs_.push_back("./resource/normal-bwa.log");
  string output;
  ASSERT_FALSE(fcs::findError(logs_, output));
  logs_.push_back("./resource/error-bwa.log");
  string target = "[E::mem_pestat] fail to locate the index files"
  ASSERT_TRUE(fcs::findError(logs_, output));
  ASSERT_EQ(target, output);
}
