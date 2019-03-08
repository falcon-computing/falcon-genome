#ifndef FCSGENOME_BAMINPUT_H
#define FCSGENOME_BAMINPUT_H

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

typedef std::map<int, std::vector<std::string> > BAMset;
typedef std::vector<std::string> MergedRegionSet;

struct BamInputInfo {
  std::string bam_name;
  bool bam_isdir;
  int bamfiles_number;
  int baifiles_number;
  int bedfiles_number;
  int listfiles_number;
  BAMset partsBAM;
  MergedRegionSet mergedREGION;
};

class BamInput {
 public:
    typedef enum {
      DEFAULT,
      NORMAL,
      TUMOR
    } InputType;
    BamInput(std::string dir_path);
    BamInputInfo merge_region(int);
    BamInputInfo getInfo();
    std::string get_gatk_args(int, BamInput::InputType = DEFAULT);
 private:
    int files_in_dir(std::string, std::string);
    std::string get_input_type(BamInput::InputType input);    
    BamInputInfo data_;
};

} // namespace fcsgenome

#endif
