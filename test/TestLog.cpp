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

TEST_F(TestLog, TestGatkLog ) {
  std::vector<std::string> logs_;
  logs_.push_back(fcsgenome::get_bin_dir() + "/../../test/resource/normal-gatk.log");
  fcsgenome::LogUtils logUtils;
  std::string output = logUtils.findError(logs_);
  ASSERT_TRUE(output.empty());
  logs_.push_back(fcsgenome::get_bin_dir() + "/../../test/resource/error-gatk.log");
  std::string target = "ERROR ------------------------------------------------------------------------------------------\nERROR A GATK RUNTIME ERROR has occurred (version 3.7-0-gcfedb67):\nERROR\nERROR This might be a bug. Please check the documentation guide to see if this is a known problem.\nERROR If not, please post the error message, with stack trace, to the GATK forum.\nERROR Visit our website and forum for extensive documentation and answers to\nERROR commonly asked questions https://software.broadinstitute.org/gatk\nERROR\nERROR MESSAGE: For input string: \"0.000\"\nERROR ------------------------------------------------------------------------------------------\n";
  output = logUtils.findError(logs_);
  ASSERT_FALSE(output.empty());
  ASSERT_EQ(target, output); 
}

TEST_F(TestLog, TestBwaLog ) {
  std::vector<std::string> logs_;
  logs_.push_back(fcsgenome::get_bin_dir() + "/../../test/resource/normal-bwa.log");
  fcsgenome::LogUtils logUtils;
  std::string output = logUtils.findError(logs_);
  ASSERT_TRUE(output.empty());
  logs_.push_back(fcsgenome::get_bin_dir() + "/../../test/resource/error-bwa.log");
  std::string target = "[E::mem_pestat] fail to locate the index files\n";
  output = logUtils.findError(logs_);
  ASSERT_FALSE(output.empty());
  ASSERT_EQ(target, output);
}

TEST_F(TestLog, TestMultipleGatkLog) {
  std::vector<std::string> logs_;
  fcsgenome::LogUtils logUtils;
  logs_.push_back(fcsgenome::get_bin_dir() + "/../../test/resource/processor1-gatk.log");
  logs_.push_back(fcsgenome::get_bin_dir() + "/../../test/resource/processor2-gatk.log");
  logs_.push_back(fcsgenome::get_bin_dir() + "/../../test/resource/processor3-gatk.log");
  logs_.push_back(fcsgenome::get_bin_dir() + "/../../test/resource/processor4-gatk.log");
  logs_.push_back(fcsgenome::get_bin_dir() + "/../../test/resource/processor5-gatk.log");
  logs_.push_back(fcsgenome::get_bin_dir() + "/../../test/resource/processor6-gatk.log");
  logs_.push_back(fcsgenome::get_bin_dir() + "/../../test/resource/processor7-gatk.log");
  logs_.push_back(fcsgenome::get_bin_dir() + "/../../test/resource/processor8-gatk.log");
  logs_.push_back(fcsgenome::get_bin_dir() + "/../../test/resource/processor9-gatk.log");
  logs_.push_back(fcsgenome::get_bin_dir() + "/../../test/resource/processor10-gatk.log");
  std::string output = logUtils.findError(logs_);
  std::string target = "ERROR ------------------------------------------------------------------------------------------\nERROR A GATK RUNTIME ERROR has occurred (version 3.7-0-gcfedb67):\nERROR\nERROR This might be a bug. Please check the documentation guide to see if this is a known problem.\nERROR If not, please post the error message, with stack trace, to the GATK forum.\nERROR Visit our website and forum for extensive documentation and answers to\nERROR commonly asked questions https://software.broadinstitute.org/gatk\nERROR\nERROR MESSAGE: For input string: \"0.000\"\nERROR ------------------------------------------------------------------------------------------\n";
  ASSERT_EQ(target, output);
}
