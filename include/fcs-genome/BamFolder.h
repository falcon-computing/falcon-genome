#ifndef FCSGENOME_BAMFOLDER_H
#define FCSGENOME_BAMFOLDER_H

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
typedef std::vector<std::string> MergedBEDset;

struct BamFolderInfo {
  std::string bam_name;
  bool bam_isdir;
  int bamfiles_number;
  int baifiles_number;
  int bedfiles_number;
  BAMset partsBAM;
  MergedBEDset mergedBED;
};

class BamFolder {
 public:
    BamFolder(std::string dir_path);
    BamFolderInfo getInfo();
    BamFolderInfo merge_bed(int);
 private:
    int files_in_dir(std::string, std::string);
    BamFolderInfo data_;
};

} // namespace fcsgenome

#endif
