#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
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
  std::string database_name,
  //std::string intervals,
  bool &flag_f,
  bool flag_gatk): 
  Worker(
      get_config<bool>("latency_mode") ?  conf_host_list.size() : 1,
      get_config<int>("gatk.joint.ncontigs"), std::vector<std::string>(), "Combine GVCF"),
	ref_path_(ref_path),
        input_path_(input_path),
        database_name_(database_name),
        //intervals_(intervals),
        flag_gatk_(flag_gatk)
{
  // check database name if gatk4 is used:
  if (flag_gatk_ || get_config<bool>("use_gatk4")){
    //if (database_name_.empty() && intervals_.empty()) {
    if (database_name_.empty()) {
      LOG(ERROR) << "Database Name and Intervals must both be defined";     
      throw silentExit();
    }
  } 

  // check output files
  output_path_ = get_absolute_path(output_path);
  for (int contig = 0;contig < get_config<int>("gatk.joint.ncontigs"); contig ++) {
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

  int ncontigs = get_config<int>("gatk.joint.ncontigs");

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
    col_part["vcf_output_filename"] = get_contig_fname(output_path_, i, "gvcf.gz");
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
  loader["vcf_output_format"] = "z";

  // write loader to string
  std::ofstream fout;
  fout.open(loader_json_);
  fout << loader;
  DLOG(INFO) << "Written loader to " << loader_json_;
}

void CombineGVCFsWorker::genHostlist() {

  // set host list
  if (!conf_host_list.empty()) {
    // assume an even distribute of processes
    int total_slots = get_config<int>("gatk.joint.ncontigs");
    int slots_per_host = total_slots / conf_host_list.size();

    host_list_ = temp_dir_ + "/hostfile";
    std::ofstream fout;
    fout.open(host_list_);

    for (int i = 0; i < conf_host_list.size(); i++) {
      fout << conf_host_list[i] << " "
           << "slots=" << slots_per_host << "\n";
      DLOG(INFO) << conf_host_list[i] << " "
           << "slots=" << slots_per_host << "\n";
    }
  }
}

void CombineGVCFsWorker::check() {

  // check all paths
  ref_path_ = check_input(ref_path_);
  input_path_ = check_input(input_path_);

  // get a vector of input files
  get_input_list(input_path_, input_files_, ".*\\.gvcf.gz");

  // check if input files all have index
  for (auto file : input_files_) {
    std::string idx_fname = file + ".tbi";
    check_input(idx_fname);
  }

  if (!flag_gatk_ || !get_config<bool>("use_gatk4")) {
    // 1. generate vid.json
    genVid();
    
    // 2. generate callset.json
    genCallSet();
    
    // 3. generate loader.json
    genLoader();
    
    // 4. generate host list
    genHostlist();
  } 
}
 
void CombineGVCFsWorker::setup() {

  boost::filesystem::wpath path(ref_path_);
  path = path.replace_extension(".dict");
  std::string dict_path = check_input(path.string());

  // parse ref.dict files
  std::vector<std::string> dict_lines = get_lines(dict_path, "@SQ.*");
  std::vector<std::pair<std::string, uint64_t>> dict;

  std::string chr_name[25];
  for (int i = 0; i < dict_lines.size(); i++) {
    if (get_config<bool>("gatk.skip_pseudo_chr") && i >= 25) {
      break;
    }
    typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
    boost::char_separator<char> space_sep(" \t");
    boost::char_separator<char> colon_sep(":");

    std::string line = dict_lines[i];
    tokenizer tok_space{line, space_sep};

    int idx = 0;
    for (tokenizer::iterator it = tok_space.begin(); it != tok_space.end(); ++it) {
      // [0] @SQ, [1] SN:contig, [2] LN:length
      if (idx == 1) {
        tokenizer tok{*it, colon_sep};
	tokenizer::iterator field_it = tok.begin();
        chr_name[i] = *(++field_it);
      }
      idx ++;
    }
  }

  // create cmd
  std::stringstream cmd;
  if (flag_gatk_ || get_config<bool>("use_gatk4")) {
    cmd << get_config<std::string>("java_path") << " "
	<< "-Xmx" << 64 << "g ";

    cmd << "-jar " << get_config<std::string>("gatk4_path") << " GenomicsDBImport ";
    for (auto file : input_files_) {
       cmd << " -V " << file; 
    }
    cmd << " --genomicsdb-workspace-path " << database_name_ << " " ;

    for (int i=0; i<25; i++){
      //cmd << " --intervals " << i << " ";    
      cmd << " --intervals " << chr_name[i] << " ";
    }
    // Not working well according to GATK forum:
    //<< " --reader-threads " << get_config<int>("gatk.joint.ncontigs") << " "
    //<< " --max-num-intervals-to-import-in-parallel " << get_config<int>("gatk.joint.ncontigs") << " ";
  }
  else{
    // create output dir
    create_dir(output_path_);
    cmd << get_config<std::string>("mpi_path") << "/bin/mpirun " 
        << "--prefix " << get_config<std::string>("mpi_path") << " "
        << "--bind-to none "
        << "--mca pml ob1 " // same issue as in workers/BWAWorker.cpp
        << "--allow-run-as-root "
        << "-np " << get_config<int>("gatk.joint.ncontigs") << " ";
    if (!conf_host_list.empty()) {
      cmd << "--hostfile " << host_list_ << " ";
    }
    
    cmd << get_config<std::string>("genomicsdb_path") << " " << loader_json_;
  }
  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

} // namespace fcsgenome
