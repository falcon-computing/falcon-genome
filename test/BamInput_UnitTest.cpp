
#include "gtest/gtest.h"
#include "iostream"
#include <limits.h>
#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <map>
#include <vector>

#include "fcs-genome/BamInput.h"
#include "BamInput_UnitTest.h"

namespace fcs = fcsgenome;
typedef std::map<int, std::vector<std::string> > BAMset;
typedef std::vector<std::string> MergedBEDset;

namespace {

class QuickTest : public testing::Test {
 protected:
  virtual void SetUp() {
    start_time_ = time(NULL);
  }
  time_t start_time_;
};

class TestBamInputClass : public QuickTest {

};

// Assuming the input is a BAM file: 
TEST_F(TestBamInputClass, CheckBamInputAsFile) {
  std::string BamInputPath="sample_test.bam";
  std::string BaiInputPath="sample_test.bai";
  create_bam_name(BamInputPath);
  create_bam_name(BaiInputPath);
  fcs::BamInput MyBam(BamInputPath);
  fcs::BamInputInfo BamData = MyBam.getInfo();

  EXPECT_EQ(0, BamData.bam_name.empty());
  EXPECT_EQ(1, BamData.baifiles_number);
  EXPECT_STREQ(BamInputPath.c_str(), BamData.bam_name.c_str());
  ASSERT_FALSE(BamData.bam_isdir);

  remove_string(BamInputPath);
  remove_string(BaiInputPath);
  BamInputPath.clear();
  BaiInputPath.clear();
}

// Assuming the input is a BAM folder:             
TEST_F(TestBamInputClass, CheckBamInputAsFolder) {
  std::string BamInputPath="sample";
  create_bamdir_with_data(BamInputPath);

  fcs::BamInput MyBam(BamInputPath);
  fcs::BamInputInfo BamData = MyBam.getInfo();

  EXPECT_EQ(1, BamData.bamfiles_number);
  EXPECT_EQ(1, BamData.baifiles_number);
  EXPECT_EQ(1, BamData.bedfiles_number);
  EXPECT_STREQ(BamInputPath.c_str(), BamData.bam_name.c_str());
  ASSERT_TRUE(BamData.bam_isdir);

  remove_string(BamInputPath);
  BamInputPath.clear();
  remove_folder(BamInputPath);
}

}  // namespace
