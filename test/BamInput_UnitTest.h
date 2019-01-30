
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <string.h>

namespace fcs = fcsgenome;
typedef std::map<int, std::vector<std::string> > BAMset;
typedef std::vector<std::string> MergedBEDset;

  void create_bam_name(std::string fname) {
    std::ofstream outfile;
    outfile.open(fname);
    outfile << "This is Test" << std::endl;
    outfile.close(); outfile.clear();
  };


void create_bamdir(std::string dirname) {
  boost::filesystem::path MyDir=dirname.c_str();
  boost::filesystem::create_directory(MyDir);
};



void create_bamdir_with_data(std::string dirname) {
    create_bamdir(dirname);
    std::ofstream outfile;
    outfile.open(dirname+"/sampleA_r1.fastq.gz");
    outfile << "Data" << std::endl;
    outfile.close(); outfile.clear();
    outfile.open(dirname+"/sampleA_r2.fastq.gz");
    outfile.close();outfile.clear();
};

void remove_string(std::string my_path){
    remove(my_path.c_str());
};

void remove_folder(std::string my_path){
    boost::filesystem::remove_all(my_path);
};
 


