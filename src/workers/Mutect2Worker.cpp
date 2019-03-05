#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/BamInput.h"
#include "fcs-genome/workers/Mutect2Worker.h"

namespace fcsgenome {

Mutect2Worker::Mutect2Worker(std::string ref_path,
      std::vector<std::string> intv_path,
      std::string normal_path,
      std::string tumor_path,
      std::string output_path,
      std::vector<std::string> extra_opts,
      std::vector<std::string> &dbsnp_path,
      std::vector<std::string> &cosmic_path,
      std::string &germline_path,
      std::string &panels_of_normals,
      std::string &normal_name,
      std::string &tumor_name,
      int  contig,
      bool &flag_f,
      bool flag_gatk): Worker(1, get_config<int>("gatk.mutect2.nct", "gatk.nct"), extra_opts, "Generating Mutect2 VCF"),
  ref_path_(ref_path),
  intv_path_(intv_path),
  normal_path_(normal_path),
  tumor_path_(tumor_path),
  dbsnp_path_(dbsnp_path),
  cosmic_path_(cosmic_path),
  germline_path_(germline_path),
  panels_of_normals_(panels_of_normals),
  normal_name_(normal_name),
  tumor_name_(tumor_name),
  contig_(contig),
  flag_gatk_(flag_gatk)
{
  // check input/output files
  output_path_ = check_output(output_path, flag_f);
}

void Mutect2Worker::check() {
  ref_path_    = check_input(ref_path_);
  for (auto region : intv_path_) {
     region = check_input(region);
  }

  if (flag_gatk_ || get_config<bool>("use_gatk4") ) {
    if (!germline_path_.empty()){
      germline_path_ = check_input(germline_path_);
      check_vcf_index(germline_path_);
    }
    if (!panels_of_normals_.empty()){
      panels_of_normals_ = check_input(panels_of_normals_);
      check_vcf_index(panels_of_normals_);
    }
  } else {
     for (int i = 0; i < dbsnp_path_.size(); i++) {
        dbsnp_path_[i] = check_input(dbsnp_path_[i]);
        check_vcf_index(dbsnp_path_[i]);
     }
     for (int j = 0; j < cosmic_path_.size(); j++) {
        cosmic_path_[j] = check_input(cosmic_path_[j]);
        check_vcf_index(cosmic_path_[j]);
     }
  }

  BamInputInfo normal_data_ = normal_path_.getInfo();
  normal_data_ = normal_path_.merge_region(contig_);
  normal_data_.bam_name = check_input(normal_data_.bam_name);

  BamInputInfo tumor_data_ = tumor_path_.getInfo();
  tumor_data_ = tumor_path_.merge_region(contig_);
  tumor_data_.bam_name = check_input(tumor_data_.bam_name);

  // Compare if sets of part BED files are the same:
  if (normal_data_.partsBAM.size() != tumor_data_.partsBAM.size()) {
    LOG(ERROR) << "Normal and Tumor do not have the same number of parts BAM files in folders";
    throw silentExit();
  }
  if (normal_data_.mergedREGION.size() != tumor_data_.mergedREGION.size()) {
    LOG(ERROR) << "Normal and Tumor do not have the same number of parts BAM files in folders";
    throw silentExit();
  }

  for (int i=0; i<normal_data_.mergedREGION.size(); i++){
     DLOG(INFO) << compareFiles(normal_data_.mergedREGION[i], tumor_data_.mergedREGION[i]);
     if (!compareFiles(normal_data_.mergedREGION[i], tumor_data_.mergedREGION[i])) {
       LOG(ERROR) << "Normal and Tumor do not have the same coordinates in the BED files";
       throw silentExit();
     }
     else{
       std::string ext[2] = {"bed", "list"};
       for (int k=0; k<2; k++){
	  std::string from = get_fname_by_ext(normal_data_.mergedREGION[i], ext[k]);
	  std::string to = get_fname_by_ext(output_path_, ext[k]);
          if (boost::filesystem::exists(from)){
             boost::filesystem::copy(from, to);    
          }
       };
     }     
  }
}

void Mutect2Worker::setup() {

  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx" << get_config<int>("gatk.mutect2.memory", "gatk.memory") << "g ";

  if (flag_gatk_ || get_config<bool>("use_gatk4") ) {
    cmd << "-jar " << get_config<std::string>("gatk4_path") << " Mutect2 ";
  }
  else{
    cmd << "-jar " << get_config<std::string>("gatk_path") << " -T MuTect2 ";
  }

  cmd << "-R " << ref_path_ << " ";

  if (flag_gatk_ || get_config<bool>("use_gatk4")) {

    cmd << normal_path_.get_gatk_args(contig_);
    cmd << tumor_path_.get_gatk_args(contig_);
   
    cmd << " -normal " << normal_name_ << " "
        << " -tumor "  << tumor_name_ << " ";
    
    if (!germline_path_.empty()){
      cmd << " --germline-resource " << germline_path_  << "  ";
    }

    if (!panels_of_normals_.empty()){
      cmd << " -pon " << panels_of_normals_  << " ";
    }

    cmd << " --output " << output_path_ << " ";

  }
  else {

    std::string n=normal_path_.get_gatk_args(contig_, BamInput::NORMAL);
    std::string t=tumor_path_.get_gatk_args(contig_, BamInput::TUMOR);

    cmd << n << " " << t << " ";
    
    if (!extra_opts_.count("--variant_index_type")) {
       cmd << "--variant_index_type LINEAR ";
    }

    if (!extra_opts_.count("--variant_index_parameter")) {
       cmd << "--variant_index_parameter 128000 ";
    }

    cmd << "-nct " << get_config<int>("gatk.mutect2.nct", "gatk.nct") << " "
        << "-o " << output_path_ << " ";

    for (int i = 0; i < dbsnp_path_.size(); i++) {
        cmd << "--dbsnp " << dbsnp_path_[i] << " ";
    }
    for (int j = 0; j < cosmic_path_.size(); j++) {
        cmd << "--cosmic " << cosmic_path_[j] << " ";
    }

  } // End checking GATK version

  for (auto region: intv_path_){
    cmd << " -L " << region << " " ;
  }

  cmd << " -isr INTERSECTION ";

  for (auto it = extra_opts_.begin(); it != extra_opts_.end(); it++) {
      cmd << it->first << " ";
      for ( auto vec_iter = it->second.begin(); vec_iter != it->second.end(); vec_iter++) {
         if (!(*vec_iter).empty() && vec_iter == it->second.begin()) {
            cmd << *vec_iter << " ";
          }
          else if (!(*vec_iter).empty()) {
                  cmd << it->first << " " << *vec_iter << " ";
          }
      }
  }

  cmd_ = cmd.str();

  DLOG(INFO) << cmd_;
}

} // namespace fcsgenome
