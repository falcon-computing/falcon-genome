#include "SampleSheet.h"
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

SampleSheet::SampleSheet(std::string a){fname=a;};
std::string SampleSheet::get_fname(){return fname;};

bool SampleSheet::is_file(){
  const char *path=fname.c_str();
  struct stat buf;
  stat(path, &buf);
  return S_ISREG(buf.st_mode);
}

bool SampleSheet::is_dir(){
  const char *path = fname.c_str();
  struct stat buf;
  stat(path, &buf);
  return S_ISDIR(buf.st_mode);
}

int SampleSheet::check_file(){
  int exist=0;
  std::ifstream ifile(fname.c_str());
  if (ifile) exist = 1;
  ifile.close();
  return exist;
};

std::map<std::string, std::vector<SampleDetails> > SampleSheet::extract_data_from_file(){
  std::ifstream file(fname.c_str());
  std::string value;
  getline(file,value);
  std::string header=value;
  //cout << "HEADER: \t" << header << endl;
  if (header.find('#')!=0){
     printf("Sample Sheet %s has problems\n",fname.c_str());
     return SampleData;
     exit(0);
  };

  int number_of_fields=count(header.begin(),header.end(),',');
  //cout << "There are " << to_string(number_of_fields+1) << " fields " << endl;

  std::vector<std::string> strs;
  boost::split(strs,header,boost::is_any_of(","));
  for (size_t i = 0; i < strs.size(); i++){
    if (strs[i].compare("sample_id") == 0) Header[i]="sample_id";
    if (strs[i].compare("fastq1") == 0) Header[i]="fastq1";
    if (strs[i].compare("fastq2") == 0) Header[i]="fastq2";
    if (strs[i].compare("rg") == 0) Header[i]="rg";
    if (strs[i].compare("platform_id") == 0) Header[i]="platform_id";
    if (strs[i].compare("library_id") == 0) Header[i]="library_id";
  }
  if (number_of_fields == strs.size()) {
     printf("Number of Fields in Sample Sheet : %4d\n",number_of_fields+1);
  };
  strs.clear();

  std::vector<SampleDetails> sampleInfoVect;
  std::string sampleName, read1, read2, rg, platform, library_id;
  SampleDetails sampleInfo;

  int check_fields;
  std::string grab_line_info;
  while (getline(file,grab_line_info)) {
     check_fields = count(grab_line_info.begin(),grab_line_info.end(),',')+1;
     if (check_fields != number_of_fields+1) {
        printf("ERROR: Number of Fields in Data (%4d) != Number of Fields in Header (%4d)\n",check_fields, number_of_fields);
     };
     boost::split(strs,grab_line_info,boost::is_any_of(","));
     for (size_t k=0; k<strs.size(); k++) {
       if (Header[k] == "sample_id") sampleName=strs[k];
       if (Header[k] == "fastq1") sampleInfo.fastqR1=strs[k];
       if (Header[k] == "fastq2") sampleInfo.fastqR2=strs[k];
       if (Header[k] == "rg") sampleInfo.ReadGroup=strs[k];
       if (Header[k] == "platform_id") sampleInfo.Platform=strs[k];
       if (Header[k] == "library_id") sampleInfo.LibraryID=strs[k];
     }
     strs.clear();

     //cout << "ORG:\t" << sampleName <<"\t" << sampleInfo.fastqR1 << "\t" << sampleInfo.fastqR2 << "\t" << sampleInfo.ReadGroup << "\t" << sampleInfo.Platform << "\t" << sampleInfo.LibraryID << endl;
     // Populating the Map SampleData:
     if (SampleData.find(sampleName) == SampleData.end()) {
     	 sampleInfoVect.push_back(sampleInfo);
         SampleData.insert(make_pair(sampleName, sampleInfoVect));
     } else {
     	 SampleData[sampleName].push_back(sampleInfo);
     }
     sampleInfoVect.clear();
  };
  file.close();
  return SampleData;
};

std::map<std::string, std::vector<SampleDetails> > SampleSheet::extract_data_from_folder(){
  std::cout << "Folder Path : " << fname.c_str() << " \n" << std::endl;
  DIR *target_dir;
  struct dirent *dir;
  std::vector<std::string> temp_vector;
  target_dir=opendir(fname.c_str());
  std::string mystring;
  if (target_dir) {
     while ((dir = readdir(target_dir)) != NULL) {
        int length = strlen(dir->d_name);
        int ext = strlen("1.fastq.gz");
        if (strncmp(dir->d_name + length - ext, "1.fastq.gz", ext)  ==  0) {
	  mystring=dir->d_name;
	  temp_vector.push_back(mystring);
	  std::cout << dir->d_name ;
        };
     };
     if (temp_vector.size() == 0) {
       std::cout << "Folder does not contain any FASTQ files" << std::endl;
       return SampleData;
       exit(0);
     }
     closedir(target_dir);
  } else {
    std::cout << "Folder does not exist" << std::endl; return SampleData; exit(0);
  };

  std::vector<SampleDetails> sampleInfoVect;
  std::string sampleName, read1, read2, rg, platform, library_id;
  platform="Illumina";
  SampleDetails sampleInfo;

  std::string target="1.fastq.gz";
  std::string task="2.fastq.gz";
  std::string delimiter="_";
  std::string new_delimiter=" ";
  std::string temp_id, sample_id;
  for (std::vector<std::string>::iterator it = temp_vector.begin(); it != temp_vector.end(); ++it ){
      read1=*it;
      read2=read1;
      temp_id=read1;
      size_t found = read2.find(target);
      read2.replace(read2.find(target),read2.length(),task);
   
      found = temp_id.find(delimiter);
      temp_id.replace(temp_id.find(delimiter), temp_id.length(), new_delimiter);
      std::vector<std::string> strs;
      boost::split(strs,temp_id,boost::is_any_of(" "));
      sampleName=strs[0];
      strs.clear();
 
     // Populating the Structure:
     sampleInfo.fastqR1=read1;
     sampleInfo.fastqR2=read2;
     sampleInfo.ReadGroup="RG";
     sampleInfo.Platform="Illumina";
     sampleInfo.LibraryID="LIB";
 
     int index=0;
     char number[4];
     std::string str;
     str = sprintf(number,"%03d",index);
     if (SampleData.find(sampleName) == SampleData.end()){
	rg="RG-"+sampleName+"_"+number;
        library_id="LIB"+sampleName+"_"+number;
        sampleInfo.ReadGroup=rg+number;
        sampleInfo.LibraryID=library_id;
        sampleInfoVect.push_back(sampleInfo);
        SampleData.insert(make_pair(sampleName, sampleInfoVect));
     } else {
        index=index+1;
	       str=sprintf(number,"%03d",index);
        rg="RG-"+sampleName+"_"+number;
        library_id="LIB"+sampleName+"_"+number;
        sampleInfo.ReadGroup=rg+number;
        sampleInfo.LibraryID=library_id;
        SampleData[sampleName].push_back(sampleInfo);
     }
     sampleInfoVect.clear();
 }
 return SampleData;

};

};
