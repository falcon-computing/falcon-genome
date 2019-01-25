#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <dirent.h>
#include <fstream>
#include <glog/logging.h>
#include <iostream>
#include <map>
#include <regex>
#include <stdexcept>
#include <stdlib.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <vector>

#include "fcs-genome/config.h"
#include "fcs-genome/BamFolder.h"

namespace fcsgenome {

BamFolder::BamFolder(std::string dir_path) {
  DLOG(INFO) << "Initializing ReadBamFolder Class for " << dir_path;
  DLOG(INFO) << "BAM FOLDER PATH : " << dir_path;
  if (boost::filesystem::exists(dir_path)) {
    if (boost::filesystem::is_directory(dir_path)) {
      data_.bam_name = dir_path;
      data_.bam_isdir = true;
      data_.bamfiles_number = files_in_dir(dir_path, ".bam");
      data_.baifiles_number = files_in_dir(dir_path, ".bai");
      data_.bedfiles_number = files_in_dir(dir_path, ".bed");
       
    } else {
      if (boost::filesystem::is_regular_file(dir_path)){
        data_.bam_name = dir_path;
        data_.bam_isdir = false;
        data_.bamfiles_number = 1;
        data_.baifiles_number = 1;
        data_.bedfiles_number = 0;         
      }
    }
  } else {
      LOG(ERROR) << "Input " << dir_path  <<  " is neither a file nor directory";
      throw std::runtime_error("INVALID PATH");
  }
}

int BamFolder::files_in_dir(std::string dir_path, std::string ext){
  boost::filesystem::path Path(dir_path);
  int files_with_ext = 0;
  // Default constructor for an iterator is the end iterator                                                                                                                         
  boost::filesystem::directory_iterator end_iter;
  for (boost::filesystem::directory_iterator iter(Path); iter != end_iter; ++iter)
    if (iter->path().extension() == ext)
      ++files_with_ext;

  return files_with_ext;
}

BamFolderInfo BamFolder::merge_bed(int contig){
  if (data_.bam_isdir) {
    int my_num = (int) data_.bedfiles_number/contig;  
    int first,last;
    for (int i=0; i<contig; i++) {
       first=i*my_num;
       last=(i+1)*my_num;
   
       std::vector<std::string> BAMvector;
   
       if (last>data_.bedfiles_number) last=data_.bedfiles_number-1;
       std::string my_name = data_.bam_name + "/part-" + std::to_string(first) + "_" + std::to_string(last-1) + ".bed" ;
   
       // If more than 1 pair (BAM, BED) goes to 1 gatk process, the BED files need to be merged.
       // Otherwise the process will fail due to no overlapping regions.
       std::ofstream merge_bed;
       merge_bed.open(my_name,std::ofstream::out | std::ofstream::app);
       for (int i=first; i<last;++i) {
          std::string input = get_bucket_fname(data_.bam_name, i);
          std::string input_bed = get_fname_by_ext(input, "bed");
          if (boost::filesystem::exists(input_bed)) {
            if (abs(first-last)==1) {
   	    // for 1 pair of (BAM, BED) per GATK process:
   	    data_.mergedBED.push_back(input_bed);
            }
            else {
   	    // for multiple pairs of (BAM, BED) per GATK process:
   	    std::ifstream single_bed(input_bed);
   	    merge_bed << single_bed.rdbuf();
            }
          }
          // Pushing BAM files
          BAMvector.push_back(input);
       }
       merge_bed.close();
       data_.partsBAM.insert(std::pair<int, std::vector<std::string> >(i,BAMvector));  
   
       // Pushing the merged BED File:
       if (abs(first-last)>1) data_.mergedBED.push_back(my_name);
   
    } // End of for (int i=0; i<contig; i++)
  }
  else {
    for (int i=0; i<contig; i++) {
      std::vector<std::string> singleBAM;
      singleBAM.push_back(data_.bam_name);
      data_.partsBAM.insert(std::pair<int, std::vector<std::string> >(i,singleBAM));
    }
  }
  return data_;
} 

BamFolderInfo BamFolder::getInfo(){
  return data_;
};

} // namespace fcsgenome
