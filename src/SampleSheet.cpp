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

#include "SampleSheet3.h"

namespace fcsgenome {

SampleSheet::SampleSheet(std::string path){
  DLOG(INFO) << "Initializing SampleSheet Class for " << path;
  DLOG(INFO) << "Sample Sheet PATH : " << path;
  if (boost::filesystem::exists(path)){
    if (boost::filesystem::is_directory(path)){
      extractDataFromFolder(path);
    } else {
      extractDataFromFile(path);
    } 

  } else {
      LOG(ERROR) << "Input " << path  <<  " is neither a file nor directory";
      throw std::runtime_error("INVALID PATH");
  }
}

void SampleSheet::extractDataFromFile(std::string fname) {
  std::ifstream file(fname.c_str());
  std::string value;
  getline(file,value);
  std::string header = value;
  DLOG(INFO) << "HEADER: \t" << header;
  if (header.find('#')!=0){
     LOG(ERROR) << "Sample Sheet "<< fname.c_str() << " has problems\n";
     throw std::runtime_error("Check PATH : " + fname + " Maybe it is empty or has format issues");
  }

  int number_of_fields = count(header.begin(),header.end(),',');
  DLOG(INFO) << "There are " << std::to_string(number_of_fields+1) << " fields ";

  std::regex regex_sample_id("(.*)(sample_id)");
  std::regex regex_fastq1("(.*)(fastq1)");
  std::regex regex_fastq2("(.*)(fastq2)");
  std::regex regex_rg("(.*)(rg)");
  std::regex regex_platform_id("(.*)(platform_id)");
  std::regex regex_library_id("(.*)(library_id)");

  std::vector<std::string> strs;
  boost::split(strs,header,boost::is_any_of(","));
  for (size_t i = 0; i < strs.size(); i++){
    if (std::regex_match (strs[i],regex_sample_id)) header_[i] = "sample_id";    
    if (std::regex_match (strs[i],regex_fastq1)) header_[i] = "fastq1";
    if (std::regex_match (strs[i],regex_fastq2)) header_[i] = "fastq2";
    if (std::regex_match (strs[i],regex_rg)) header_[i] = "rg";
    if (std::regex_match (strs[i],regex_platform_id)) header_[i] = "platform_id";
    if (std::regex_match (strs[i],regex_library_id)) header_[i] = "library_id";
  }

  if (number_of_fields == strs.size()) {
    DLOG(INFO) << "Number of Fields in Sample Sheet :" << number_of_fields+1;
  }

  strs.clear();

  std::vector<SampleDetails> sampleInfoVect;
  std::string sampleName, read1, read2, rg, platform, library_id;
  SampleDetails sampleInfo;
  
  int check_fields;
  std::string grab_line_info;
  while (getline(file,grab_line_info)) {
     check_fields = count(grab_line_info.begin(),grab_line_info.end(),',') + 1;
     if (check_fields != number_of_fields+1) {
	DLOG(ERROR) << "Number of Fields in Data (" << check_fields 
                   << ") != Number of Fields in Header (" << number_of_fields <<")";
        throw std::runtime_error("Check PATH : " + fname + " :  Number of Fields in Data Block is inconsistent with that of in Header"); 
     };
     boost::split(strs,grab_line_info,boost::is_any_of(","));
     for (size_t k = 0; k < strs.size(); k++) {
       if (header_[k] == "sample_id") sampleName = strs[k];
       if (header_[k] == "fastq1") sampleInfo.fastqR1 = strs[k];
       if (header_[k] == "fastq2") sampleInfo.fastqR2 = strs[k];
       if (header_[k] == "rg") sampleInfo.ReadGroup = strs[k];
       if (header_[k] == "platform_id") sampleInfo.Platform = strs[k];
       if (header_[k] == "library_id") sampleInfo.LibraryID = strs[k];
     }
     strs.clear();

     DLOG(INFO) << "INPUT : \t" << sampleName <<"\t" << sampleInfo.fastqR1 << "\t"
      	       << sampleInfo.fastqR2 << "\t" << sampleInfo.ReadGroup << "\t"
     	       << sampleInfo.Platform << "\t" << sampleInfo.LibraryID;

     // Populating the Map SampleData:
     if (data_.find(sampleName) == data_.end()) {
     	 sampleInfoVect.push_back(sampleInfo);
         data_.insert(make_pair(sampleName, sampleInfoVect));
     } else {
     	 data_[sampleName].push_back(sampleInfo);
     }
     sampleInfoVect.clear();
  }

  if(data_.empty()){
     DLOG(ERROR) << "Map data_ for " + fname + " was not populated" ;
     throw std::runtime_error("Check MAP data_ for " + fname);
  }  

  file.close();
}
 
void SampleSheet::extractDataFromFolder(std::string fname){
   DLOG(INFO) << "Folder Path : " << fname.c_str();
   DIR *target_dir;
   struct dirent *dir;
   std::vector<std::string> temp_vector;
   target_dir = opendir(fname.c_str());
   std::string mystring;
   if (target_dir) {
      while ((dir = readdir(target_dir)) != NULL) {
         int length = strlen(dir->d_name);
         int ext = strlen("1.fastq.gz");
         if (strncmp(dir->d_name + length - ext, "1.fastq.gz", ext)  ==  0) {
 	     mystring = dir->d_name;
 	     temp_vector.push_back(mystring);
         };
      };
      if (temp_vector.size() == 0) {
          DLOG(ERROR) << "Folder " << fname.c_str() << " does not contain any FASTQ files";
          throw std::runtime_error("Check PATH : " + fname + " . Folder maybe empty or no FASTQ files");
      }
      closedir(target_dir);
   } else {
      DLOG(ERROR) << "Folder " <<  fname.c_str()  << " does not exist";
      throw std::runtime_error("Check PATH : " + fname + " . Folder does not exist");
   }
 
   std::vector<SampleDetails> sampleInfoVect;
   std::string sampleName, read1, read2, rg, platform, library_id;
   platform = "Illumina";
   SampleDetails sampleInfo;
 
   std::string target = "1.fastq.gz";
   std::string task = "2.fastq.gz";
   std::string delimiter = "_";
   std::string new_delimiter = " ";
   std::string temp_id, sample_id;
   for (std::vector<std::string>::iterator it = temp_vector.begin(); it != temp_vector.end(); ++it ){
       read1 = *it;
       read2 = read1;
       temp_id = read1;
       size_t found = read2.find(target);
       read2.replace(read2.find(target),read2.length(),task);
    
       found = temp_id.find(delimiter);
       temp_id.replace(temp_id.find(delimiter), temp_id.length(), new_delimiter);
       std::vector<std::string> strs;
       boost::split(strs,temp_id,boost::is_any_of(" "));
       sampleName = strs[0];
       strs.clear();
  
      // Populating the Structure:
      sampleInfo.fastqR1 = read1;
      sampleInfo.fastqR2 = read2;
      sampleInfo.ReadGroup = "RG";
      sampleInfo.Platform = "Illumina";
      sampleInfo.LibraryID = "LIB";
  
      int index=0;
      char number[3];
      if (data_.find(sampleName) == data_.end()){
	 sprintf(number,"%02d",index);
 	 rg = "RG-"+sampleName+"_"+number;
         library_id = "LIB"+sampleName+"_"+number;
         sampleInfo.ReadGroup = rg+number;
         sampleInfo.LibraryID = library_id;
         sampleInfoVect.push_back(sampleInfo);
         data_.insert(make_pair(sampleName, sampleInfoVect));
      } else {
         index = index + 1;
 	 sprintf(number,"%02d",index);
         rg = "RG-" + sampleName + "_" + number;
         library_id = "LIB" + sampleName+"_" + number;
         sampleInfo.ReadGroup = rg+number;
         sampleInfo.LibraryID = library_id;
         data_[sampleName].push_back(sampleInfo);
      }
      sampleInfoVect.clear();
   }
 
   if(data_.empty()){
     DLOG(ERROR)<< "Map data_ for " + fname + " was not populated" ;
     throw std::runtime_error("Check MAP data_ for " + fname);
   }
}
   
SampleSheetMap SampleSheet::get(){    
   return data_;
}

} // namespace fcsgenome
