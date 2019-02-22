#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <dirent.h>
#include <fstream>
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

#ifdef NDEBUG
#define LOG_HEADER "fcs-genome"
#endif
#include <glog/logging.h>

#include "fcs-genome/config.h"
#include "fcs-genome/BamInput.h"

namespace fcsgenome {

static inline std::string get_input_type(BamInput::InputType input) {
  switch (input) {
  case BamInput::DEFAULT:
    return " -I ";
  case BamInput::NORMAL :
    return " -I:normal ";
  case BamInput::TUMOR :
    return " -I:tumor ";
  default:
    LOG(ERROR) << "InputType not available";
    throw silentExit();
  }
}

BamInput::BamInput(std::string dir_path) {
  DLOG(INFO) << "Initializing ReadBamInput Class for " << dir_path;
  DLOG(INFO) << "BAM FOLDER PATH : " << dir_path;
  if (boost::filesystem::exists(dir_path)) {
    if (boost::filesystem::is_directory(dir_path)) {
      data_.bam_name = dir_path;
      data_.bam_isdir = true;
      data_.bamfiles_number = files_in_dir(dir_path, ".bam");
      data_.baifiles_number = files_in_dir(dir_path, ".bai");
      data_.bedfiles_number = files_in_dir(dir_path, ".bed");
      data_.listfiles_number = files_in_dir(dir_path, ".list");
    } else {
      if (boost::filesystem::is_regular_file(dir_path)){
        data_.bam_name = dir_path;
        data_.bam_isdir = false;
        data_.bamfiles_number = 1;
	std::string bai_path = get_fname_by_ext(dir_path, "bai");
	if (boost::filesystem::exists(bai_path) || boost::filesystem::exists(dir_path + ".bai")) {
	  data_.baifiles_number = 1;
	}
	else {
          LOG(ERROR) << "Input BAM File " << dir_path  <<  " does not have an index file (bai) " << bai_path ;
	  throw silentExit();
	}
        data_.bedfiles_number = 0; 
        data_.listfiles_number = 0;        
      }
    }
  } else {
      LOG(ERROR) << "Input " << dir_path  <<  " does not exist";
      throw silentExit();
  }
}

int BamInput::files_in_dir(std::string dir_path, std::string ext){
  boost::filesystem::path Path(dir_path);
  int files_with_ext = 0;
  // Default constructor for an iterator is the end iterator                                                                                                                         
  boost::filesystem::directory_iterator end_iter;
  for (boost::filesystem::directory_iterator iter(Path); iter != end_iter; ++iter)
    if (iter->path().extension() == ext)
      ++files_with_ext;

  return files_with_ext;
}

BamInputInfo BamInput::merge_region(int contig){
  if (data_.bam_isdir) {
    // Check the existence of BED or list files:
    int region_file_number;
    std::string ext;
    if (data_.bedfiles_number==0) {
      if (data_.listfiles_number==0) {
        throw std::runtime_error("No BED or list files in " + data_.bam_name);
      }
      else {
        if (data_.listfiles_number<get_config<int>("gatk.ncontigs")) {
	  throw std::runtime_error("Number of List Files less than ncontig");
	}
        region_file_number=data_.listfiles_number;
        ext = "list";
      }
    }
    else {
      if (data_.bedfiles_number<get_config<int>("gatk.ncontigs")) {
        throw std::runtime_error("Number of BED Files less than ncontig");
      }
      region_file_number=data_.bedfiles_number;
      ext = "bed";
    }

    int my_num = (int) region_file_number/get_config<int>("gatk.ncontigs");

    int first,last;
    first=contig*my_num;
    last=(contig+1)*my_num;
    std::vector<std::string> BAMvector;   
    if (last>data_.bamfiles_number) {
      if (data_.bamfiles_number%2==0){
        last=data_.bamfiles_number;
      } 
      else {
        last=data_.bamfiles_number-1;
      }
    }

    std::string my_name = conf_temp_dir + "/part-" + std::to_string(first) + "_" + std::to_string(last-1) + ".bed" ;   

    // If more than 1 pair (BAM, BED) goes to 1 gatk process, the BED files need to be merged.
    // Otherwise the process will fail due to no overlapping regions.
    std::ofstream merge_region;
    merge_region.open(my_name,std::ofstream::out | std::ofstream::app);
    for (int i=first; i<last;++i) {
       std::string input = get_bucket_fname(data_.bam_name, i);
       std::string input_region = get_fname_by_ext(input, ext);
       if (boost::filesystem::exists(input_region)) {
         if (abs(first-last)==1) {
    	    // for 1 pair of (BAM, REGION) per GATK process:
    	    data_.mergedREGION.push_back(input_region);
         }
         else {
    	    // for multiple pairs of (BAM, REGION) per GATK process:
    	    std::ifstream single_region(input_region);
    	    merge_region << single_region.rdbuf();
         }
       }
       // Pushing BAM files
       BAMvector.push_back(input);
    }
    merge_region.close();
    data_.partsBAM.insert(std::pair<int, std::vector<std::string> >(contig,BAMvector));  
   
    // Pushing the merged REGION File:
    if (abs(first-last)>1) data_.mergedREGION.push_back(my_name);

  }
  else {
    std::vector<std::string> singleBAM;
    singleBAM.push_back(data_.bam_name);
    data_.partsBAM.insert(std::pair<int, std::vector<std::string> >(contig,singleBAM));
  }
  return data_;
} 

BamInputInfo BamInput::getInfo(){
  return data_;
};

std::string BamInput::get_gatk_args(int index, BamInput::InputType input){
  std::string gatk_command_;

  for (auto bam : data_.partsBAM[index]) {
    //gatk_command_ = gatk_command_ + " -I " + bam;
    gatk_command_ = gatk_command_ + get_input_type(input) + bam;

  }

  for (auto region : data_.mergedREGION) {
    gatk_command_ = gatk_command_ + " -L " + region;
  }
  return gatk_command_;
};

} // namespace fcsgenome
