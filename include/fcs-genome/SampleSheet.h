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

class SampleSheet {
  std::string fname;
  private:
    std::map<int, std::string> Header;
    std::map<std::string, std::vector<SampleDetails> > SampleData;
  public:
    SampleSheet(std::string);
    std::string get_fname();
    int check_file();
    bool is_file();
    bool is_dir();
    std::map<std::string, std::vector<SampleDetails> > extract_data_from_file();
    std::map<std::string, std::vector<SampleDetails> > extract_data_from_folder();
};

};

#endif
