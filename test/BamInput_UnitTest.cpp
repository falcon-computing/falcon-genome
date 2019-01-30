
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
TEST_F(TestBamInputClass, CheckBamInput) {
  std::string BamInputPath="sample_test.bam";
  create_bam_name(BamInputPath);
  fcs::BamInput MyBam(BamInputPath);
  fcs::BamInputInfo BamData = MyBam.getInfo();

  EXPECT_EQ(0, BamData.bam_name.empty());

   remove_string(BamInputPath);
   BamInputPath.clear();
}


}  // namespace
