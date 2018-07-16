#include <algorithm>
#include <bits/stdc++.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/tokenizer.hpp>
#include <boost/thread.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"

namespace fcsgenome {

std::string conf_project_dir;
std::string conf_bin_dir;
std::string conf_root_dir;
std::string conf_temp_dir;
int main_tid = -1;

boost::program_options::variables_map config_vtable;

std::vector<std::string> conf_host_list;
std::set<std::string> config_list{
    "temp_dir",
    "log_dir",
    "ref_genome"
    "bwa_path",
    "sambamba_path",
    "bcftools_path",
    "bgzip_path",
    "tabix_path",
    "genomicsdb_path",
    "mpi_path",
    "java_path",
    "gatk_path"
};

static std::string env_name_mapper(std::string key) {
  // entire string to lowercase
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);

  // check if has 'fcs_' prefix
  if (key.length() > 4 && key.substr(0, 4).compare("fcs_") == 0) {
    // check if defined in list
    if (config_list.count(key.substr(4))) {
      return key.substr(4);
    }
  }
  return std::string();
}

void calc_gatk_default_config(
    int & nprocs,
    int & memory,
    int cpu_num,
    int memory_size) {

  nprocs = 32;
  memory = 4;

  // allow some extra room for JVM memory, very empirical
  double memory_margin = 0.05;

  while (nprocs > cpu_num) {
    nprocs /= 2;
  }
  // first increase memory if necessary
  while (nprocs * (memory+2) < memory_size * (1+memory_margin)
      && memory < 16)
  {
    memory += 2;
  }
  // then decrease nprocs if necessary
  while (nprocs * memory > memory_size * (1+memory_margin)) {
    // TODO: decrease by 2x could be too aggresive
    nprocs /= 2;
  }
}

int check_nprocs_config(std::string key, int cpu_num) {
  int val = get_config<int>("gatk." + key + ".nprocs");
  if (val > cpu_num) {
    LOG(WARNING) << "Using too many threads for current command.";
    LOG(WARNING) << "- number of cpu cores = " << cpu_num;
    LOG(WARNING) << "- gatk." << key << ".nprocs = " << val;

    return 1;
  }
  return 0;
}

int check_memory_config(std::string key, int memory_size) {
  int nprocs = get_config<int>("gatk." + key + ".nprocs");
  int memory = get_config<int>("gatk." + key + ".memory");
  int total_memory = nprocs*memory;

  if (memory < 4) {
    LOG(WARNING) << "gatk." << key << ".memory(" << memory << ") "
                 << "value is too low, recommand value is 4gb";
    return 1;
  }
  else if (total_memory> memory_size*(1+0.05)) {
    LOG(WARNING) << "Using too much memory for current command, "
                 << "processes may be killed by OS.";
    LOG(WARNING) << "  - memory size = " << memory_size << "gb";
    LOG(WARNING) << "  - gatk." << key << ".nprocs = " << nprocs;
    LOG(WARNING) << "  - gatk." << key << ".memory = " << memory << "gb";

    return 1;
  }
  return 0;
}

int init_config(boost::program_options::options_description conf_opt) {

  std::string g_conf_fname = conf_root_dir + "/fcs-genome.conf";
  std::string l_conf_fname = "fcs-genome.conf";

  namespace po = boost::program_options;

  try {
    // 1st priority: get conf from environment variables
    po::store(po::parse_environment(conf_opt, env_name_mapper), config_vtable);

    // 2nd priority: get conf from user specified path
    {
      std::ifstream conf_file{l_conf_fname.c_str()};
      if (conf_file) {
        po::store(po::parse_config_file(conf_file, conf_opt), config_vtable);
        DLOG(INFO) << "Load config from " << l_conf_fname;
      }
    }
    // 3rd priority: get conf from global conf path
    {
      std::ifstream conf_file{g_conf_fname.c_str()};
      if (conf_file) {
        po::store(po::parse_config_file(conf_file, conf_opt), config_vtable);
        DLOG(INFO) << "Load config from " << g_conf_fname;
      }
    }
    po::notify(config_vtable);
  }
  catch (po::error &e) {
    std::cerr << "fcs-genome configuration options:" << std::endl;
    std::cerr << conf_opt << std::endl;
    LOG(ERROR) << "Failed to initialize config: " << e.what();

    throw silentExit();
  }

  // set parameters based on dependency
  set_config<bool>("bwa.scaleout_mode", "latency_mode");
  set_config<bool>("gatk.scaleout_mode", "latency_mode");

  set_config<int>("gatk.bqsr.nprocs",  "gatk.nprocs");
  set_config<int>("gatk.pr.nprocs",    "gatk.nprocs");
  set_config<int>("gatk.htc.nprocs",   "gatk.nprocs");
  set_config<int>("gatk.mutect2.nprocs",   "gatk.nprocs");
  set_config<int>("gatk.indel.nprocs", "gatk.nprocs");
  set_config<int>("gatk.ug.nprocs",    "gatk.nprocs");
  set_config<int>("gatk.depth.nprocs", "gatk.nprocs");

  set_config<int>("gatk.bqsr.nct", "gatk.nct");
  set_config<int>("gatk.pr.nct",   "gatk.nct");
  set_config<int>("gatk.htc.nct",  "gatk.nct");
  set_config<int>("gatk.mutect2.nct",  "gatk.nct");
  set_config<int>("gatk.ug.nt",    "gatk.nct");
  set_config<int>("gatk.depth.nct", "gatk.nct");

  set_config<int>("gatk.bqsr.memory",  "gatk.memory");
  set_config<int>("gatk.pr.memory",    "gatk.memory");
  set_config<int>("gatk.htc.memory",   "gatk.memory");
  set_config<int>("gatk.mutect2.memory",   "gatk.memory");
  set_config<int>("gatk.indel.memory", "gatk.memory");
  set_config<int>("gatk.ug.memory",    "gatk.memory");
  set_config<int>("gatk.depth.memory", "gatk.memory");

  // create temp dir
  std::string username("");
  username = std::getenv("USER");
  conf_temp_dir = get_config<std::string>("temp_dir") +
                  "/fcs-genome-" +
                  username +
                  "-" +
                  std::to_string((long long)getpid());

  // check tool files
  check_input(get_config<std::string>("bwa_path"), false);
  check_input(get_config<std::string>("sambamba_path"), false);
  check_input(get_config<std::string>("bcftools_path"), false);
  check_input(get_config<std::string>("bgzip_path"), false);
  check_input(get_config<std::string>("tabix_path"), false);
  check_input(get_config<std::string>("genomicsdb_path"), false);
  check_input(get_config<std::string>("gatk_path"), false);

  // parse host list if scaleout_mode is selected
  if (get_config<bool>("bwa.scaleout_mode") ||
      get_config<bool>("gatk.scaleout_mode") ||
      get_config<bool>("latency_mode")) {
    std::string hosts = get_config<std::string>("hosts");

    typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
    boost::char_separator<char> sep(", ");
    tokenizer tok{hosts, sep};

    conf_host_list.insert(conf_host_list.end(), tok.begin(), tok.end());
  }

  // log debug info
  DLOG(INFO) << "conf_root_dir = " << conf_root_dir;
  DLOG(INFO) << "conf_temp_dir = " << conf_temp_dir;
  if (get_config<bool>("gatk.skip_pseudo_chr")) {
    DLOG(INFO) << "skipping pseudo chromosome intervals";
  }

  if (!conf_host_list.empty()) {
    DLOG(INFO) << "Hosts list: ";
    for (int i = 0; i < conf_host_list.size(); i++) {
      DLOG(INFO) << conf_host_list[i];
    }
  }

  return 0;
}

int init(char** argv, int argc) {
  // static settings
  conf_bin_dir      = get_bin_dir();
  conf_root_dir     = conf_bin_dir + "/..";
  conf_project_dir  = get_absolute_path(".fcs-genome");
  namespace po = boost::program_options;

  po::options_description conf_opt;
  po::options_description common_opt("common");
  po::options_description tools_opt("tools");

  std::string opt_str;
  int opt_int;
  bool opt_bool;

  // system info
  int cpu_num     = boost::thread::hardware_concurrency();
  int memory_size = get_sys_memory();

  DLOG(INFO) << "System has " << cpu_num << " total threads on this node";
  DLOG(INFO) << "System has " << memory_size << " gb memory";

  // calculate default num_cpu and memory
  int def_ncontigs = 32;
  int def_nprocs   = 32;
  int def_memory   = 4;

  calc_gatk_default_config(def_nprocs, def_memory, cpu_num, memory_size);

  DLOG(INFO) << "Default gatk.nprocs = " << def_nprocs;
  DLOG(INFO) << "Default gatk.memory = " << def_memory;

  common_opt.add_options()
    arg_decl_string_w_def("temp_dir",        "/tmp",      "temp dir for fast access")
    arg_decl_string_w_def("log_dir",         "./log",     "log dir")
    arg_decl_string_w_def("ref_genome",      "",          "(deprecated) default reference genome path")
    arg_decl_string_w_def("java_path",       "java -d64", "java binary")
    arg_decl_string_w_def("mpi_path",        "/usr/lib64/openmpi",                  "path to mpi installation")
    arg_decl_string_w_def("bwa_path",        conf_root_dir+"/tools/bin/bwa-bin",    "path to bwa binary")
    arg_decl_string_w_def("sambamba_path",   conf_root_dir+"/tools/bin/sambamba",   "path to sambamba")
    arg_decl_string_w_def("bcftools_path",   conf_root_dir+"/tools/bin/bcftools",   "path to bcftools")
    arg_decl_string_w_def("bgzip_path",      conf_root_dir+"/tools/bin/bgzip",      "path to bgzip")
    arg_decl_string_w_def("tabix_path",      conf_root_dir+"/tools/bin/tabix",      "path to tabix")
    arg_decl_string_w_def("genomicsdb_path", conf_root_dir+"/tools/bin/vcf2tiledb", "path to GenomicsDB")
    arg_decl_string_w_def("gatk_path",       conf_root_dir+"/tools/package/GenomeAnalysisTK.jar", "path to gatk.jar")
    arg_decl_string_w_def("hosts", "",       "host list for scale-out mode")
    arg_decl_bool_w_def("latency_mode", false, "enable sorting in bwa-mem")
    ;

  tools_opt.add_options()
    arg_decl_int_w_def("bwa.verbose",              0,     "verbose level of bwa output")
    arg_decl_int_w_def("bwa.nt",                   -1,    "number of threads for bwa-mem")
    arg_decl_int_w_def("bwa.num_batches_per_part", 40,    "max num records in each BAM file")
    arg_decl_bool_w_def("bwa.use_fpga",            true,  "option to enable FPGA for bwa-mem")
    arg_decl_bool_w_def("bwa.use_sort",            true,  "enable sorting in bwa-mem")
    arg_decl_bool_w_def("bwa.enforce_order",       false,  "enforce strict sorting ordering")
    arg_decl_string_w_def("bwa.fpga.bit_path",     conf_root_dir+"/tools/bitstreams/bitstream.xclbin", "path to FPGA bitstream for bwa")
    arg_decl_string_w_def("bwa.fpga.pac_path",     "",    "(deprecated) path to PAC reference used by FPGA for bwa")
    arg_decl_string("bwa.mpi_if", "network interface to use mpi connection")
    arg_decl_bool("bwa.scaleout_mode", "enable scale-out mode for bwa")

    arg_decl_int_w_def("markdup.max_files",    4096, "max opened files in markdup")
    arg_decl_int_w_def("markdup.nt",           (16 > cpu_num ? cpu_num : 16),   "thread num in markdup")
    arg_decl_int_w_def("markdup.overflow-list-size", 2000000, "overflow list size in markdup")

    arg_decl_int_w_def("mergebam.max_files",    4096, "max opened files in mergebam")
    arg_decl_int_w_def("mergebam.nt",           (16 > cpu_num ? cpu_num : 16),   "thread num in mergebam")

    arg_decl_bool("gatk.scalout_mode", "enable scale-out mode for gatk")
    arg_decl_string_w_def("gatk.intv.path",    "", "default path to existing contig intervals")
    arg_decl_int_w_def("gatk.ncontigs", def_ncontigs, "default contig partition num in GATK steps")
    arg_decl_int_w_def("gatk.nprocs",   def_nprocs,   "default process num in GATK steps")
    arg_decl_int_w_def("gatk.memory",   def_memory,   "default heap memory in GATK steps")
    arg_decl_int_w_def("gatk.nct",             1,  "default thread number in GATK steps (deprecated)")
    arg_decl_int("gatk.bqsr.nprocs",               "default process num in GATK BaseRecalibrator")
    arg_decl_int("gatk.bqsr.nct",                  "default thread num in  GATK BaseRecalibrator")
    arg_decl_int("gatk.bqsr.memory",               "default heap memory in GATK BaseRecalibrator")
    arg_decl_int("gatk.pr.nprocs",                 "default process num in GATK PrintReads")
    arg_decl_int("gatk.pr.nct",                    "default thread num in  GATK PrintReads")
    arg_decl_int("gatk.pr.memory",                 "default heap memory in GATK PrintReads")
    arg_decl_int("gatk.htc.nprocs",                "default process num in GATK HaplotypeCaller")
    arg_decl_int("gatk.htc.nct",                   "default thread num in  GATK HaplotypeCaller")
    arg_decl_int("gatk.htc.memory",                "default heap memory in GATK HaplotypeCaller")
    arg_decl_int("gatk.mutect2.nprocs",            "default process num in GATK Mutect2")
    arg_decl_int("gatk.mutect2.nct",               "default thread num in  GATK Mutect2")
    arg_decl_int("gatk.mutect2.memory",            "default heap memory in GATK Mutect2")
    arg_decl_int("gatk.indel.nprocs",              "default process num in GATK IndelRealigner")
    arg_decl_int("gatk.indel.memory",              "default heap memory in GATK IndelRealigner")
    arg_decl_int("gatk.ug.nprocs",                 "default process num in GATK UnifiedGenotyper")
    arg_decl_int("gatk.ug.nt",                     "default thread num in GATK UnifiedGenotyper")
    arg_decl_int("gatk.ug.memory",                 "default heap memory in GATK UnifiedGenotyper")
    arg_decl_int_w_def("gatk.rtc.nt",          (16 > cpu_num ? cpu_num : 16), "default thread num in GATK RealignerTargetCreator")
    arg_decl_int_w_def("gatk.rtc.memory",      (48 > memory_size ? memory_size: 48), "default heap memory in GATK RealignerTargetCreator")
    arg_decl_int_w_def("gatk.joint.ncontigs",  32, "default contig partition num in joint genotyping")
    arg_decl_int_w_def("gatk.combine.nprocs",  def_nprocs, "default process num in GATK CombineGVCFs")
    arg_decl_int_w_def("gatk.genotype.nprocs", def_nprocs, "default process num in GATK GenotypeGVCFs")
    arg_decl_int_w_def("gatk.genotype.memory", def_memory, "default heap memory in GATK GenotypeGVCFs")
    arg_decl_int("gatk.depth.nprocs",            "default process num in GATK DepthOfCoverage")
    arg_decl_int("gatk.depth.nct",               "default thread num in  GATK DepthOfCoverage")
    arg_decl_int("gatk.depth.memory",            "default heap memory in GATK DepthOfCoverage")
    arg_decl_bool("gatk.skip_pseudo_chr", "skip pseudo chromosome intervals")
    arg_decl_bool_w_def("gatk.skip_pseudo_chr", true, "skip pseudo chromosome intervals")
    arg_decl_string_w_def("blaze.nam_path", conf_root_dir+"/tools/blaze/bin/nam", "path to nam in blaze")
    arg_decl_string_w_def("blaze.conf_path",conf_root_dir+"/tools/blaze/conf",    "path to nam configuration file")
    ;

  conf_opt.add(common_opt).add(tools_opt);

  // Intialize configs
  int ret = init_config(conf_opt);

  if (argc > 1 && ::strcmp(argv[1], "conf") == 0) {
    std::cerr << "fcs-genome configuration options:" << std::endl;
    std::cerr << conf_opt << std::endl;
    throw silentExit();
  }

  // Other starting procedures
  //create_dir(get_config<std::string>("log_dir"));
  create_dir(conf_temp_dir);

  // set main thread pid
  // works because this function is only called by main()
  main_tid = getpid();

  return ret;
}

static inline void write_contig_intv(std::ofstream& fout,
    std::string chr,
    uint64_t lbound, uint64_t ubound) {
  fout << chr << ":" << lbound << "-" << ubound << std::endl;
}

std::vector<std::string> init_contig_intv(std::string ref_path) {
  int ncontigs = get_config<int>("gatk.ncontigs");

  std::stringstream ss;
  ss << conf_temp_dir << "/intv_" << ncontigs;
  std::string intv_dir = ss.str();
  create_dir(intv_dir);

  // record the intv paths
  std::vector<std::string> intv_paths(ncontigs);
  for (int i = 0; i < ncontigs; i++) {
       intv_paths[i] = get_contig_fname(intv_dir, i, "list", "intv");
  }

  // TODO: temporary to use old partition method, need to check
  // if num_contigs = 32
  std::string org_intv_dir = get_config<std::string>("gatk.intv.path");
  if (ncontigs == 32 && !org_intv_dir.empty()) {
    DLOG(INFO) << "Use original interval files";
    // copy intv files
    for (int i = 0; i < ncontigs; i++) {
      std::string org_intv = get_contig_fname(org_intv_dir, i, "list", "intv");
      if (boost::filesystem::exists(intv_paths[i])) {
        break;
      }

      boost::filesystem::copy_file(org_intv, intv_paths[i]);
    }
    return intv_paths;
  }

  // read ref.dict file to get contig lengths
  ref_path = check_input(ref_path);
  boost::filesystem::wpath path(ref_path);
  path = path.replace_extension(".dict");
  std::string dict_path = check_input(path.string());

  // parse ref.dict files
  std::vector<std::string> dict_lines = get_lines(dict_path, "@SQ.*");
  std::vector<std::pair<std::string, uint64_t>> dict;
  uint64_t dict_length = 0;
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
    std::string chr_name;
    uint64_t    chr_length = 0;
    for (tokenizer::iterator it = tok_space.begin();
         it != tok_space.end(); ++it) {
      // [0] @SQ, [1] SN:contig, [2] LN:length
      if (idx == 1) {
        tokenizer tok{*it, colon_sep};
        tokenizer::iterator field_it = tok.begin();
        chr_name = *(++field_it);
      }
      else if (idx == 2) {
        tokenizer tok{*it, colon_sep};
        tokenizer::iterator field_it = tok.begin();
        chr_length = boost::lexical_cast<uint64_t>(*(++field_it));
      }
      idx ++;
    }
    dict.push_back(std::make_pair(chr_name, chr_length));
    dict_length += chr_length;
  }

  // generate intv.list
  int contig_idx = 0;

  // positions per contig part
  uint64_t contig_npos = (dict_length+ncontigs-1)/ncontigs;
  uint64_t remain_npos = contig_npos;   // remaining positions for partition

  DLOG(INFO) << "contig_npos = " << contig_npos;

  uint64_t lbound = 1;
  uint64_t ubound = contig_npos;

  std::ofstream fout;
  fout.open(intv_paths[0]);

  for (int i = 0; i < dict.size(); i++) {
    std::string chr_name = dict[i].first;
    uint64_t chr_length = dict[i].second;
    uint64_t npos = chr_length;

    // if the number of positions in one chr is larger than one contig part
    while (npos > remain_npos) {
      ubound = remain_npos + lbound - 1;

      write_contig_intv(fout, chr_name, lbound, ubound);

      lbound = ubound + 1;
      npos -= remain_npos;
      remain_npos = contig_npos;

      fout.close();
      fout.open(intv_paths[++contig_idx]);
    }
    // write remaining positions in the chr to the current contig
    if (npos > 0) {
      write_contig_intv(fout, chr_name, lbound, chr_length);

      remain_npos -= npos;
      lbound = 1;
    }
  }
  fout.close();

  return intv_paths;
}

int roundUp(int numToRound, int multiple){
    if (multiple == 0) return numToRound;
    int remainder = abs(numToRound) % multiple;
    if (remainder == 0) return numToRound;
    if (numToRound < 0)
        return -(abs(numToRound) - remainder);
    else
        return numToRound + multiple - remainder;
}

// Split Reference File :
std::vector<std::string> split_ref_by_nprocs(std::string ref_path) {
  int ncontigs = get_config<int>("gatk.ncontigs");

  std::stringstream ss;
  ss << conf_temp_dir << "/intv_" << ncontigs;
  std::string intv_dir = ss.str();
  create_dir(intv_dir);

  // record the intv paths
  std::vector<std::string> intv_paths(ncontigs);
  for (int i = 0; i < ncontigs; i++) {
       intv_paths[i] = get_contig_fname(intv_dir, i, "list", "intv");
  }

  std::string org_intv_dir = get_config<std::string>("gatk.intv.path");
  if (ncontigs == 32 && !org_intv_dir.empty()) {
    DLOG(INFO) << "Use original interval files";
    // copy intv files
    for (int i = 0; i < ncontigs; i++) {
      std::string org_intv = get_contig_fname(org_intv_dir, i, "list", "intv");
      if (boost::filesystem::exists(intv_paths[i])) {
        break;
      }
      boost::filesystem::copy_file(org_intv, intv_paths[i]);
    }
    return intv_paths;
  }

  // read ref.dict file to get contig lengths
  ref_path = check_input(ref_path);
  boost::filesystem::wpath path(ref_path);
  path = path.replace_extension(".dict");
  std::string dict_path = check_input(path.string());

  // parse ref.dict files
  std::vector<std::string> dict_lines = get_lines(dict_path, "@SQ.*");
  std::vector<std::pair<std::string, uint64_t>> dict;
  uint64_t dict_length = 0;
  int max_value=0;

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
    std::string chr_name;
    uint64_t    chr_length = 0;
    for (tokenizer::iterator it = tok_space.begin();
      it != tok_space.end(); ++it) {
      // [0] @SQ, [1] SN:contig, [2] LN:length
      if (idx == 1) {
        tokenizer tok{*it, colon_sep};
        tokenizer::iterator field_it = tok.begin();
        chr_name = *(++field_it);
      }
      else if (idx == 2) {
        tokenizer tok{*it, colon_sep};
        tokenizer::iterator field_it = tok.begin();
        chr_length = boost::lexical_cast<uint64_t>(*(++field_it));
      }
      idx ++;
    }
    dict.push_back(std::make_pair(chr_name, chr_length));

    // Find the chromosome with the largest number of bases:
    if (max_value < chr_length) max_value = chr_length;
    DLOG(INFO) << "Chromosome: " << chr_name << " has " << chr_length << " bases"<< std::endl;
    dict_length += chr_length;
  }

  uint64_t factor = int(max_value/ncontigs);
  uint64_t nearest_multiple = roundUp(factor,ncontigs);

  uint64_t lbound;
  uint64_t ubound;
  int intCounter=1;
  std::vector<std::string> splitted_ref;
  for (int i = 0; i < dict.size(); i++) {
    std::string chr_name = dict[i].first;
    uint64_t chr_length = dict[i].second;
    ubound=nearest_multiple;
    if( ubound > chr_length) {
       ubound=chr_length;
       splitted_ref.push_back(chr_name+":"+ std::to_string(intCounter) + "-" + std::to_string(ubound));
       DLOG(INFO) << chr_name << "\t" << chr_length << "\t" << intCounter << "\t" << ubound << "\n";
       intCounter=1;
       continue;
    }

    // if the number of positions in one chr is larger than one contig part
    while (ubound < chr_length) {
      if (intCounter == 1){
         lbound = intCounter;
         ubound = nearest_multiple;
      }else{
	       lbound = (intCounter-1)*nearest_multiple;
         ubound = lbound + nearest_multiple;
      }

      if (ubound < chr_length){
         DLOG(INFO) << chr_name << "\t" << chr_length  << "\t" <<  lbound << "\t" << ubound << "\n";
	       splitted_ref.push_back(chr_name+":"+ std::to_string(lbound).c_str() + "-" + std::to_string(ubound).c_str()) ;
	       intCounter++;
      } else {
	       ubound = chr_length;
	       DLOG(INFO) << chr_name << "\t" << chr_length  << "\t" <<  lbound << "\t" << ubound << "\n";
	       splitted_ref.push_back(chr_name+":"+ std::to_string(lbound).c_str() + "-" + std::to_string(ubound).c_str()) ;
         intCounter=1;
      }
    }

  }

  int intervals_per_file = int(splitted_ref.size()/ncontigs);
  DLOG(INFO) << "intervals_per_file "  << intervals_per_file << "\t" << ncontigs << "\n";
  DLOG(INFO) << "splitted_ref.size() " << splitted_ref.size() << "\n";
  int count_lines = 0;
  int contig_idx = 0;
  std::ofstream fout;
  fout.open(intv_paths[contig_idx]);
  DLOG(INFO) << "Open " << intv_paths[contig_idx] << "\n";
  for (auto it=splitted_ref.begin();it!=splitted_ref.end();it++){
    std::string data_line = *it;
    // This condition adds 1 to the Start Position of the first line in the intv list file.
    // This avoids duplicate coverage computation since each intv file goes to a different thread:
    if (count_lines == 0  && contig_idx >0){
	      std::vector <std::string> resultArray;
	      boost::algorithm::split_regex( resultArray, data_line,  boost::regex( ":|-" ));
        if (std::stoi(resultArray[1]) != 1){
	         for (int k=0; k<resultArray.size();k++){
	              if (k == 0){
	                  data_line = resultArray[k] + ":";
	              } else if (k == 1){
                    int temp = std::stoi(resultArray[k])+1;
	                  data_line = data_line + std::to_string(temp) + "-";
                } else if (k ==2){
                    data_line = data_line + resultArray[k];
                }
           }
        }
    }

    DLOG(INFO) << data_line << "\n";
    fout << data_line << "\n";
    ++count_lines;
    if (count_lines == intervals_per_file && contig_idx < ncontigs-1){
	      fout.close();
        DLOG(INFO) << "Closing " << intv_paths[contig_idx] << "\n";
        fout.open(intv_paths[++contig_idx]);
        DLOG(INFO) << "Open " << intv_paths[contig_idx] << "\n";
        count_lines=0;
    }
  }
  DLOG(INFO) << "Closing " << intv_paths[contig_idx] << "\n";
  fout.close();
  return intv_paths;

}

// Spliting Files begins here :
unsigned int FileRead(std::istream &is, std::vector <char> & buff) {
    is.read(&buff[0], buff.size());
    return is.gcount();
}

unsigned int CountLines(const std::vector <char> &buff, int sz) {
    int newlines = 0;
    const char * p = &buff[0];
    for (int i = 0; i < sz; i++) {
        if ( p[i] == '\n' ) {
            newlines++;
        }
    }
    return newlines;
}

std::vector<std::string> split_by_nprocs(std::string intervalFile, std::string filetype) {

  const int SZ = 1024*1024;
  std::vector <char> buff( SZ );
  std::ifstream ifs( intervalFile );
  int n = 0;
  while( int cc = FileRead( ifs, buff ) ) {
      n += CountLines( buff, cc );
  }
  DLOG(INFO) << "Number of Genes Intervals : " << n << std::endl;

  int ncontigs = get_config<int>("gatk.ncontigs");
  int chunk = int(n/ncontigs);
  int nearest_multiple = roundUp(chunk,ncontigs);

  std::stringstream ss;
  ss << conf_temp_dir << "/intv_" << ncontigs;
  std::string intv_dir = ss.str();
  create_dir(intv_dir);

  std::string inputData[n];
  std::ifstream in_file(intervalFile);
  std::string str;
  int index=0;
  while (std::getline(in_file, str)) {
        inputData[index] = str;
        ++index;
  }

  std::ofstream myfile;
  // record the intv paths
  std::vector<std::string> intv_paths(ncontigs);
  for (int i = 0; i < ncontigs; i++) {
      if (filetype=="list") {
          intv_paths[i] = get_contig_fname(intv_dir, i, "list", "intv");
          //DLOG(INFO) << "LIST: " << intv_paths[i] << std::endl;
      } else {
          intv_paths[i] = get_contig_fname(intv_dir, i, "bed", "intv");
          //DLOG(INFO) << "BED: " << intv_paths[i] << std::endl;
      }

      myfile.open(intv_paths[i]);
      int start = i*nearest_multiple;
      int last  = start + nearest_multiple;
      if (last > n) last = n;
      for (int j = start; j < last; ++j) {
           myfile <<  inputData[j] << std::endl;
      }
      myfile.close(); myfile.clear();
  }

  std::string org_intv_dir = get_config<std::string>("gatk.intv.path");
  if (ncontigs == 32 && !org_intv_dir.empty()) {
    DLOG(INFO) << "Use original interval files";
    // copy intv files
    for (int i = 0; i < ncontigs; i++) {
      std::string org_intv = get_contig_fname(org_intv_dir, i, "list", "intv");
      if (boost::filesystem::exists(intv_paths[i])) {
        break;
      }
      boost::filesystem::copy_file(org_intv, intv_paths[i]);
  }
  return intv_paths;
}

}

} // namespace fcsgenome
