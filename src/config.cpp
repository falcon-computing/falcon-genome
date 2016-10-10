#include <fstream>
#include <string>
#include <unistd.h>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"

namespace fcsgenome {

std::string conf_project_dir;
std::string conf_bin_dir;
std::string conf_root_dir;
std::string conf_temp_dir;

boost::program_options::variables_map config_vtable;

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
    "mpirun_path",
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

int init_config() {

  // static settings
  conf_bin_dir      = get_bin_dir();
  conf_root_dir     = conf_bin_dir + "/..";
  conf_project_dir  = get_absolute_path(".fcs-genome");

  std::string g_conf_fname = conf_root_dir + "/fcs-genome.conf";
  std::string l_conf_fname = "fcs-genome.conf";

  namespace po = boost::program_options;

  po::options_description conf_opt;
  po::options_description common_opt("common");
  po::options_description tools_opt("tools");

  std::string opt_str;
  int opt_int;

  common_opt.add_options() 
    arg_decl_string_w_def("temp_dir",        "/tmp",      "temp dir for fast access")
    arg_decl_string_w_def("log_dir",         "./log",     "log dir")
    arg_decl_string_w_def("ref_genome",      "",          "default reference genome path")
    arg_decl_string_w_def("java_path",       "java -d64", "java binary")
    arg_decl_string_w_def("mpirun_path",     "/usr/lib64/openmpi/bin/mpirun",      "binary for mpirun")
    arg_decl_string_w_def("bwa_path",        conf_root_dir+"/tools/bin/bwa-bin",   "path to bwa binary")
    arg_decl_string_w_def("sambamba_path",   conf_root_dir+"/tools/bin/sambamba",  "path to sambamba")
    arg_decl_string_w_def("bcftools_path",   conf_root_dir+"/tools/bin/bcftools",  "path to bcftools")
    arg_decl_string_w_def("bgzip_path",      conf_root_dir+"/tools/bin/bgzip",     "path to bgzip")
    arg_decl_string_w_def("tabix_path",      conf_root_dir+"/tools/bin/tabix",     "path to tabix")
    arg_decl_string_w_def("genomicsdb_path", conf_root_dir+"/tools/bin/vcf2tiledb", "path to GenomicsDB")
    arg_decl_string_w_def("gatk_path",       conf_root_dir+"/tools/package/GenomeAnalysisTK.jar", "path to gatk.jar")
    ;
  
  tools_opt.add_options()
    arg_decl_int_w_def("bwa.max_records",      10000000, "max num records in each BAM file")
    arg_decl_int_w_def("markdup.max_files",    4096,     "max opened files in markdup")
    arg_decl_int_w_def("markdup.nt",           16, "thread num in markdup")
    arg_decl_int_w_def("gatk.ncontigs",        32, "default contig partition num in GATK steps")
    arg_decl_int_w_def("gatk.nprocs",          16, "default process num in GATK steps")
    arg_decl_int_w_def("gatk.bqsr.nprocs",     16, "default process num in GATK BaseRecalibrator")
    arg_decl_int_w_def("gatk.bqsr.nct",        4,  "default thread num in  GATK BaseRecalibrator")
    arg_decl_int_w_def("gatk.bqsr.memory",     8,  "default heap memory in GATK BaseRecalibrator")
    arg_decl_int_w_def("gatk.pr.nprocs",       16, "default process num in GATK PrintReads")
    arg_decl_int_w_def("gatk.pr.nct",          4,  "default thread num in  GATK PrintReads")
    arg_decl_int_w_def("gatk.pr.memory",       8,  "default heap memory in GATK PrintReads")
    arg_decl_int_w_def("gatk.htc.nprocs",      16, "default process num in GATK HaplotypeCaller")
    arg_decl_int_w_def("gatk.htc.nct",         4,  "default thread num in  GATK HaplotypeCaller")
    arg_decl_int_w_def("gatk.htc.memory",      16, "default heap memory in GATK HaplotypeCaller")
    arg_decl_int_w_def("gatk.indel.nprocs",    16, "default process num in GATK IndelRealigner")
    arg_decl_int_w_def("gatk.indel.memory",    4,  "default heap memory in GATK IndelRealigner")
    arg_decl_int_w_def("gatk.rtc.nt",          16, "default thread num in GATK RealignerTargetCreator")
    arg_decl_int_w_def("gatk.rtc.memory",      48, "default heap memory in GATK RealignerTargetCreator")
    arg_decl_int_w_def("gatk.ug.nprocs",       16, "default process num in GATK UnifiedGenotyper")
    arg_decl_int_w_def("gatk.ug.nt",           4,  "default thread num in GATK UnifiedGenotyper")
    arg_decl_int_w_def("gatk.ug.memory",       8,  "default heap memory in GATK UnifiedGenotyper")
    arg_decl_int_w_def("gatk.joint.nprocs",    32, "default process num in GATK CombineGVCFs")
    arg_decl_int_w_def("gatk.genotype.memory", 4,  "default heap memory in GATK GenotypeGVCFs")
    arg_decl_bool("gatk.skip_pseudo_chr", "skip pseudo chromosome intervals")
    ;

  conf_opt.add(common_opt).add(tools_opt);

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
    LOG(ERROR) << "Failed to initialize config: " << e.what();
    throw silentExit();
  }

  conf_temp_dir = get_config<std::string>("temp_dir") +
                  "/fcs-genome-" +
                  std::string(std::getenv("USER")) +
                  "-" +
                  std::to_string((long long)getpid());

  // check tool files
  check_input(get_config<std::string>("bwa_path"));
  check_input(get_config<std::string>("sambamba_path"));
  check_input(get_config<std::string>("bcftools_path"));
  check_input(get_config<std::string>("bgzip_path"));
  check_input(get_config<std::string>("tabix_path"));
  check_input(get_config<std::string>("genomicsdb_path"));
  check_input(get_config<std::string>("gatk_path"));

  DLOG(INFO) << "conf_root_dir = " << conf_root_dir;
  DLOG(INFO) << "conf_temp_dir = " << conf_temp_dir;
  if (get_config<bool>("gatk.skip_pseudo_chr")) {
    DLOG(INFO) << "skipping pseudo chromosome intervals";
  }

  return 0;
}

int init(const char* argv) {
  // Intialize configs
  int ret = init_config();

  // Other starting procedures
  //create_dir(get_config<std::string>("log_dir"));
  create_dir(conf_temp_dir);

  return ret;
}

std::vector<std::string> init_contig_intv(std::string ref_path) {
  std::stringstream ss;
  ss << conf_project_dir << "/intv_" << get_config<int>("gatk.ncontigs");
  std::string intv_dir = ss.str();

  bool is_ready = true;
  std::vector<std::string> intv_paths(get_config<int>("gatk.ncontigs"));
   
  for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {
    std::stringstream ss;
    ss << intv_dir << "/intv" << contig+1 << ".list";
    intv_paths[contig] = ss.str();
    if (is_ready && !boost::filesystem::exists(ss.str())) {
      is_ready = false;
    }
  }
  if (!is_ready) {
    // remove existing intv_dir and create a new one
    remove_path(intv_dir);
    create_dir(intv_dir);

    ref_path = check_input(ref_path);

    std::stringstream cmd;
    cmd << conf_root_dir + "/bin/scripts/intvGen.sh "
        << "-r " << ref_path << " "
        << "-n " << get_config<int>("gatk.ncontigs");
    if (get_config<bool>("gatk.skip_pseudo_chr")) {
      cmd << " -l";
    }

    DLOG(INFO) << cmd.str();

    if (system(cmd.str().c_str())) {
      throw failedCommand("cannot initialize contig intervals");
    }
  }
  return intv_paths; 
}
} // namespace fcsgenome
