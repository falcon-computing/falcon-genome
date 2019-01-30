
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
//#include "BamInput_UnitTest.h"

namespace fcs = fcsgenome;
typedef std::map<int, std::vector<std::string> > BAMset;
typedef std::vector<std::string> MergedBEDset;

namespace {

void create_bam_name(std::string fname) {
  std::ofstream outfile;
  outfile.open(fname);
  outfile << "This is Test" << std::endl;
  outfile.close(); outfile.clear();
};

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


//  //std::cout << "Check if SampleSheet.csv file is not empty (5 lines: 1 header and 4 inputs)" << std::endl;
//  EXPECT_EQ(0, SampleSheetPath.empty());
//  //std::cout << "Check if SampleData Map is not empty" << std::endl;
//  EXPECT_EQ(1, !SampleData.empty());
//  //std::cout << "Check if SampleData Map contains 4 inputs" << std::endl;
//  EXPECT_EQ(4, SampleData.size());
//  //std::cout << "Check if each sample in SampleData Map contains 6 fields" << std::endl;
//  int count=0;
//  for (auto pair : SampleData) {
//    std::string sample_id = pair.first;
//    EXPECT_STREQ(SAMPLE_ID[count].c_str(), sample_id.c_str());
//    std::vector<fcs::SampleDetails> list = pair.second;
//    for (int i = 0; i < list.size(); ++i) {
//       std::string fastq1 = list[i].fastqR1;
//       std::string fastq2 = list[i].fastqR2;
//       std::string rg = list[i].ReadGroup;
//       std::string platform = list[i].Platform;
//       std::string library_id= list[i].LibraryID;
//       EXPECT_STREQ(FASTQ_R1[count].c_str(), fastq1.c_str());
//       EXPECT_STREQ(FASTQ_R2[count].c_str(), fastq2.c_str());
//       EXPECT_STREQ(READ_GROUP[count].c_str(), rg.c_str());
//       EXPECT_STREQ(PLATFORM.c_str(), platform.c_str());
//       EXPECT_STREQ(LIB[count].c_str(), library_id.c_str());
//    }
//    count++;
//  }

//   remove_list(BamInputPath);
   //SampleData.clear();
}


// TEST_F(TestSampleSheetClass, CheckSampleSheetPATH) {
//   SampleSheetPath="fastq";
//   create_dir_with_data(SampleSheetPath);
//   fcs::SampleSheet MySampleSheet(SampleSheetPath);
//   SampleData = MySampleSheet.get();
//   //std::cout << "Check if "<< SampleSheetPath << " folder is not empty (5 lines: 1 header and 4 inputs)" << std::endl;
//   EXPECT_EQ(0, SampleSheetPath.empty());
//   //std::cout << "Check if SampleData Map is not empty" << std::endl;
//   EXPECT_EQ(1, !SampleData.empty());
//   //std::cout << "Check if SampleData Map contains 4 inputs" << std::endl;
//   EXPECT_EQ(4, SampleData.size());
//   //std::cout << "Check if each sample in SampleData Map contains 6 fields" << std::endl;
//   int count=0;
//   for (auto pair : SampleData) {
//       std::string sample_id = pair.first;
//       EXPECT_STREQ(SAMPLE_ID[count].c_str(), sample_id.c_str());
//       std::vector<fcs::SampleDetails> list = pair.second;
//       for (int i = 0; i < list.size(); ++i) {
// 	 std::string fastq1 = list[i].fastqR1;
// 	 std::string fastq2 = list[i].fastqR2;
// 	 std::string rg = list[i].ReadGroup;
// 	 std::string platform = list[i].Platform;
// 	 std::string library_id= list[i].LibraryID;
// 	 EXPECT_STREQ((SampleSheetPath + "/" + FASTQ_R1[count]).c_str(), fastq1.c_str());
// 	 EXPECT_STREQ((SampleSheetPath + "/" + FASTQ_R2[count]).c_str(), fastq2.c_str());
// 	 EXPECT_STREQ(READ_GROUP_FOLDER[count].c_str(), rg.c_str());
// 	 EXPECT_STREQ(PLATFORM.c_str(), platform.c_str());
// 	 EXPECT_STREQ(LIB_FOLDER[count].c_str(), library_id.c_str());
//       }
//       count++;
//   }
//   remove_dir(SampleSheetPath);
//   SampleData.clear();
// }

}  // namespace
