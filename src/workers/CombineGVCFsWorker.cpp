#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <fstream>
#include <json/json.h>
#include <string>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/CombineGVCFsWorker.h"

namespace fcsgenome {

CombineGVCFsWorker::CombineGVCFsWorker(
		std::string ref_path,
		std::string input_path,
		std::string output_path,
		bool &flag_f): 
	Worker(1),
	ref_path_(ref_path),
	input_path_(input_path)
{
  // check output files
  output_path_ = get_absolute_path(output_path);
  for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig ++) {
    check_output(get_contig_fname(output_path_, contig, "gvcf"), flag_f);
  }

  temp_dir_ = conf_temp_dir + "/joint";
  create_dir(temp_dir_);
}

void CombineGVCFsWorker::genVid() {
  // TODO: use ref.fai to gen
  vid_json_ = temp_dir_ + "/vid.json";
  std::string ref_fai = check_input(ref_path_ + ".fai");

  Json::Value vid;

  // 1. write vid fields
  Json::Value info_vec(Json::arrayValue);
  Json::Value format_vec(Json::arrayValue);
  Json::Value infoformat_vec(Json::arrayValue);
  info_vec.append("INFO");
  format_vec.append("FORMAT");
  infoformat_vec.append("INFO");
  infoformat_vec.append("FORMAT");

  vid["fields"]["PASS"]["type"] = "int";
  vid["fields"]["LowQual"]["type"] = "int";
  vid["fields"]["END"]["vcf_field_class"] = info_vec;
  vid["fields"]["END"]["type"] = "int";
  vid["fields"]["BaseQRankSum"]["vcf_field_class"] = info_vec;
  vid["fields"]["BaseQRankSum"]["type"] = "float";
  vid["fields"]["ClippingRankSum"]["vcf_field_class"] = info_vec;
  vid["fields"]["ClippingRankSum"]["type"] = "float";
  vid["fields"]["MQRankSum"]["vcf_field_class"] = info_vec;
  vid["fields"]["MQRankSum"]["type"] = "float";
  vid["fields"]["ReadPosRankSum"]["vcf_field_class"] = info_vec;
  vid["fields"]["ReadPosRankSum"]["type"] = "float";
  vid["fields"]["MQ"]["vcf_field_class"] = info_vec;
  vid["fields"]["MQ"]["type"] = "float";
  vid["fields"]["RAW_MQ"]["vcf_field_class"] = info_vec;
  vid["fields"]["RAW_MQ"]["type"] = "float";
  vid["fields"]["MQ0"]["vcf_field_class"] = info_vec;
  vid["fields"]["MQ0"]["type"] = "int";
  vid["fields"]["DP"]["vcf_field_class"] = infoformat_vec;
  vid["fields"]["DP"]["type"] = "int";
  vid["fields"]["GQ"]["vcf_field_class"] = format_vec;
  vid["fields"]["GQ"]["type"] = "int";
  vid["fields"]["MIN_DP"]["vcf_field_class"] = format_vec;
  vid["fields"]["MIN_DP"]["type"] = "int";
  vid["fields"]["SB"]["vcf_field_class"] = format_vec;
  vid["fields"]["SB"]["type"] = "int";
  vid["fields"]["SB"]["length"] = 4;
  vid["fields"]["GT"]["vcf_field_class"] = format_vec;
  vid["fields"]["GT"]["type"] = "int";
  vid["fields"]["GT"]["length"] = "P";
  vid["fields"]["AD"]["vcf_field_class"] = format_vec;
  vid["fields"]["AD"]["type"] = "int";
  vid["fields"]["AD"]["length"] = "R";
  vid["fields"]["PGT"]["vcf_field_class"] = format_vec;
  vid["fields"]["PGT"]["type"] = "char";
  vid["fields"]["PGT"]["length"] = "VAR";
  vid["fields"]["PID"]["vcf_field_class"] = format_vec;
  vid["fields"]["PID"]["type"] = "char";
  vid["fields"]["PID"]["length"] = "VAR";
  vid["fields"]["PL"]["vcf_field_class"] = format_vec;
  vid["fields"]["PL"]["type"] = "int";
  vid["fields"]["PL"]["length"] = "G";
  
  // 2. write vid contigs
  int num_contigs = 0;

  std::ifstream fin;
  fin.open(ref_fai);
  std::string line;
  while (std::getline(fin, line, '\n')) {
    if (get_config<bool>("gatk.skip_pseudo_chr") && num_contigs >= 25) {
      break; 
    }

    std::stringstream ss(line); 
    std::vector<std::string> fields;
    std::string field;
    while (std::getline(ss, field, '\t')) {
      fields.push_back(field);
    }
    uint64_t length = boost::lexical_cast<uint64_t>(fields[1]);
    uint64_t offset = boost::lexical_cast<uint64_t>(fields[2]);

    std::stringstream contig;
    contig << "[";
    contig << std::setw(2) << std::setfill('0') << num_contigs;
    contig << "]" << fields[0];

    vid["contigs"][contig.str()]["length"] = length;
    vid["contigs"][contig.str()]["tiledb_column_offset"] = offset;

    num_contigs ++;
  }

  std::stringstream json_ss;
  json_ss << vid;

  boost::regex expr("\\[[0-9]+\\]");
  std::string output_json = boost::regex_replace(json_ss.str(), expr, "");

  std::ofstream fout;
  fout.open(vid_json_);
  fout << output_json;
}

void CombineGVCFsWorker::genCallSet() {

  callset_json_ = temp_dir_ + "/callset.json";

  Json::Value callset;
  for (int i = 0; i < input_files_.size(); i++) {
    std::string filename = input_files_[i];
    std::string sample_id = get_basename_wo_ext(filename);
    callset["callsets"][sample_id]["row_idx"] = i;
    callset["callsets"][sample_id]["idx_in_file"] = 0;
    callset["callsets"][sample_id]["filename"] = filename;

    DLOG(INFO) << "Added " << filename << " to callset";
  }

  std::ofstream fout;
  fout.open(callset_json_);
  fout << callset;
  DLOG(INFO) << "Written callset to " << callset_json_;
}

void CombineGVCFsWorker::genLoader() {

  loader_json_ = temp_dir_ + "/loader.json";
  // get genome length
  Json::Value vid;

  std::ifstream fin;
  fin.open(vid_json_);
  if (fin.eof()) {
    throw fileNotFound("cannot find vid.json");
  }
  fin >> vid;

  uint64_t total_genome_length = 0;
  for (Json::ValueIterator it = vid["contigs"].begin(); 
       it != vid["contigs"].end(); it ++) {
    uint64_t length = (*it)["length"].asUInt64() + 
                      (*it)["tiledb_column_offset"].asUInt64();
    total_genome_length = std::max(total_genome_length, length);
  }
  DLOG(INFO) << "Total contig length = " << total_genome_length;

  int ncontigs = get_config<int>("gatk.ncontigs");

  uint64_t part_size = total_genome_length / ncontigs;
  uint64_t size_per_column_partition = 16*1024*input_files_.size();
  std::string temp_dir = conf_temp_dir + "/joint";

  // set fields in loader.json
  Json::Value loader;
  loader["row_based_partitioning"] = false;
  loader["produce_combined_vcf"] = true;

  uint64_t column_index = 0;
  for (int i = 0; i < ncontigs; i++) {
    Json::Value col_part; 
    col_part["begin"] = column_index;
    col_part["workspace"] = temp_dir;
    col_part["array"] = "partition-" + std::to_string((long long)i);
    col_part["vcf_output_filename"] = get_contig_fname(output_path_, i, "gvcf");
    column_index += part_size;
    loader["column_partitions"].append(col_part);
  }

  loader["callset_mapping_file"] = callset_json_;
  loader["vid_mapping_file"] = vid_json_;
  loader["treat_deletions_as_intervals"] = true;
  loader["reference_genome"] = ref_path_;
  loader["do_ping_pong_buffering"] = true;
  loader["size_per_column_partition"] = size_per_column_partition;
  loader["offload_vcf_output_processing"] = true;

  // write loader to string
  std::ofstream fout;
  fout.open(loader_json_);
  fout << loader;
  DLOG(INFO) << "Written loader to " << loader_json_;
}

void CombineGVCFsWorker::check() {

  // check all paths
  ref_path_ = check_input(ref_path_);
  input_path_ = check_input(input_path_);

  // 1. generate vid.json
  genVid();

  // get a vector of input files
  get_input_list(input_path_, input_files_, ".*\\.gvcf.gz");

  // 2. generate callset.json
  genCallSet();
  
  // 3. generate loader.json
  genLoader();
}
 
void CombineGVCFsWorker::setup() {
  // create output dir
  create_dir(output_path_);

  // create cmd
  std::stringstream cmd;
  cmd << get_config<std::string>("mpirun_path") << " " 
      << "-n " << get_config<int>("gatk.joint.nprocs") << " "
      << get_config<std::string>("genomicsdb_path") << " "
      << loader_json_;

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}
} // namespace fcsgenome
