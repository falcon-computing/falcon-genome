#ifndef FCSGENOME_SAMPLESHEET_H
#define FCSGENOME_SAMPLESHEET_H

#include <boost/algorithm/string.hpp>
#include <dirent.h>
#include <fstream>
#include <gtest/gtest_prod.h>
#include <iostream>
#include <map>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <vector>

namespace fcsgenome {

struct SampleDetails {
  std::string fastqR1;
  std::string fastqR2;
  std::string ReadGroup;
  std::string Platform;
  std::string LibraryID;
};

typedef std::map<std::string, std::vector<SampleDetails> > SampleSheetMap;

class SampleSheet {
 public:
  SampleSheet(std::string path);
    SampleSheetMap get();
 private:
    void extractDataFromFile(std::string);
    void extractDataFromFolder(std::string);
    std::map<int, std::string> header_;
    SampleSheetMap data_;
};

} // namespace fcsgenome

#endif
