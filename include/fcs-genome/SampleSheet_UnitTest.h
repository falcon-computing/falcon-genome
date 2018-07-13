
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <string.h>

namespace fcs = fcsgenome;
typedef std::map<std::string, std::vector<fcs::SampleDetails> > SampleSheetMap;

void create_file(std::string fname) {
    std::ofstream outfile;
    outfile.open(fname);
    outfile << "#fastq1,fastq2,rg,sample_id,platform_id,library_id" << std::endl;
    outfile << "sampleA_r1.fastq.gz,sampleA_r2.fastq.gz,ABC123,sampleA,Illumina,RD001" << std::endl;
    outfile << "sampleB_r1.fastq.gz,sampleB_r2.fastq.gz,ABC456,sampleB,Illumina,RD002" << std::endl;
    outfile << "sampleC_r1.fastq.gz,sampleC_r2.fastq.gz,ABC789,sampleC,Illumina,RD003" << std::endl;
    outfile << "sampleD_r1.fastq.gz,sampleD_r2.fastq.gz,ADD123,sampleD,Illumina,RD004" << std::endl;
    outfile.close(); outfile.clear();
};

void create_file_with_header_only(std::string fname) {
    std::ofstream outfile;
    outfile.open(fname);
    outfile << "#fastq1,fastq2,rg,sample_id,platform_id,library_id" << std::endl;
    outfile.close(); outfile.clear();
};

void create_file_with_no_header(std::string fname) {
    std::ofstream outfile;
    outfile.open(fname);
    outfile << "sampleA_r1.fastq.gz,sampleA_r2.fastq.gz,ABC123,sampleA,Illumina,RD001" << std::endl;
    outfile << "sampleB_r1.fastq.gz,sampleB_r2.fastq.gz,ABC456,sampleB,Illumina,RD002" << std::endl;
    outfile << "sampleC_r1.fastq.gz,sampleC_r2.fastq.gz,ABC789,sampleC,Illumina,RD003" << std::endl;
    outfile << "sampleD_r1.fastq.gz,sampleD_r2.fastq.gz,ADD123,sampleD,Illumina,RD004" << std::endl;
    outfile.close(); outfile.clear();
};

void create_file_with_wrong_format(std::string fname) {
    std::ofstream outfile;
    outfile.open(fname);
    outfile << "#fastq1,fastq2,rg,sample_id,platform_id,library_id" << std::endl;
    outfile << "sampleA_r1.fastq.gz,sampleA_r2.fastq.gz,ABC123,sampleA,Illumina" << std::endl;
    outfile << "sampleB_r1.fastq.gz,sampleB_r2.fastq.gz,ABC456,sampleB,Illumina,RD002" << std::endl;
    outfile.close(); outfile.clear();
};

void create_dir(std::string dirname) {
    const char *dir_path = dirname.c_str();
    boost::filesystem::path dir(dir_path);
    boost::filesystem::create_directory(dir);                    
};

void create_dir_with_data(std::string dirname) {
    const char *dir_path = dirname.c_str();
    boost::filesystem::path dir(dir_path);
    boost::filesystem::create_directory(dir);
    std::ofstream outfile;
    outfile.open(dirname+"/sampleA_r1.fastq.gz");
    outfile << "Data" << std::endl;
    outfile.close(); outfile.clear();
    outfile.open(dirname+"/sampleA_r2.fastq.gz");
    outfile <<"Data" << std::endl;
    outfile.close();outfile.clear();
    outfile.open(dirname+"/sampleB_r1.fastq.gz");
    outfile << "Data" << std::endl;
    outfile.close(); outfile.clear();
    outfile.open(dirname+"/sampleB_r2.fastq.gz");
    outfile <<"Data" << std::endl;
    outfile.close();outfile.clear();
    outfile.open(dirname+"/sampleC_r1.fastq.gz");
    outfile << "Data" << std::endl;
    outfile.close(); outfile.clear();
    outfile.open(dirname+"/sampleC_r2.fastq.gz");
    outfile << "Data" << std::endl;
    outfile.close(); outfile.clear();
    outfile.open(dirname+"/sampleD_r1.fastq.gz");
    outfile << "Data" << std::endl;
    outfile.close(); outfile.clear();
    outfile.open(dirname+"/sampleD_r2.fastq.gz");
    outfile <<"Data" << std::endl;
    outfile.close();outfile.clear();
};

void remove_list(std::string my_path){
    remove(my_path.c_str());
};

void remove_dir(std::string my_path){
    boost::filesystem::remove_all(my_path);
};
 


