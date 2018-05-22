#include <limits.h>
#include "fcs-genome/SampleSheet.h"
#include "gtest/gtest.h"
#include <string>
#include <map>

namespace fcs = fcsgenome;

// Defining Sample Sheet Name:
std::string SampleSheetName = "SampleSheet.csv";
fcs::SampleSheet MySampleSheet(SampleSheetName);
std::string CheckName = MySampleSheet.get_fname();
// Determine if Sample Sheet exists:
int exist = 0;
int CheckFile = MySampleSheet.check_file();
// Determine if Sample Sheet is a File:
bool IsFile = MySampleSheet.is_file();
// Determine if Sample Sheet is a Folder:
bool IsDir = MySampleSheet.is_dir();

// Defining Map if Sample Sheet is a File:
std::map<std::string, std::vector<fcs::SampleDetails> > SampleData = MySampleSheet.extract_data_from_file();
// Defining Map if Sample Sheet is a Folder:
std::map<std::string, std::vector<fcs::SampleDetails> > SampleData2 = MySampleSheet.extract_data_from_folder();

// Running Test:
TEST(SampleSheetTest, GetName) {
  ASSERT_STREQ(SampleSheetName.c_str(),CheckName.c_str());
}

TEST(SampleSheetTest, FileExists) {
  EXPECT_EQ(exist, CheckFile);
}

TEST(SampleSheetTest, IsDirOrNot) {
  EXPECT_EQ(exist,IsDir);
}

TEST(SampleSheetTest, ExtractDataFromFile) {
  EXPECT_EQ(1,SampleData.empty());
}

TEST(SampleSheetTest, ExtractDataFromFolder) {
  EXPECT_EQ(1,SampleData2.empty());
}
