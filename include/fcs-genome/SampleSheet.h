#ifndef FCSGENOME_SAMPLESHEET_H
#define FCSGENOME_SAMPLESHEET_H

#include <boost/algorithm/string.hpp>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <iostream>
#include <string.h>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <map>

namespace fcsgenome {

struct SampleDetails {
  std::string fastqR1;
  std::string fastqR2;
  std::string ReadGroup;
  std::string Platform;
  std::string LibraryID;
};

using SampleSheetMap = std::map<std::string, std::vector<SampleDetails> >;

class SampleSheet {
 public:
    std::string fname;
    SampleSheetMap SampleData;
    SampleSheet(std::string, SampleSheetMap);
    void getSampleSheet();
 private:
    void ExtractDataFromFile(std::string, SampleSheetMap &);
    void ExtractDataFromFolder(std::string, SampleSheetMap &);
    std::map<int, std::string> Header;
};

};

#endif
