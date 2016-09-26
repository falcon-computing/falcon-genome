#include <string>
#include "fcs-genome/common.h"
#include "fcs-genome/config.h"

namespace fcsgenome {

std::string conf_project_dir;
std::string conf_bin_dir;
std::string conf_root_dir;
std::string conf_temp_dir;
std::string conf_log_dir;
std::string conf_intv_dir;
std::string conf_default_ref;
std::string conf_bwa_call;
std::string conf_sambamba_call;
std::string conf_bcftools_call;
std::string conf_bgzip_call;
std::string conf_tabix_call;
std::string conf_java_call;
std::string conf_gatk_path;
std::string conf_gatk_call;

int conf_bwa_maxrecords = 10000000;
int conf_markdup_maxfile = 4096;
int conf_markdup_nthreads = 16;
int conf_markdup_overflowsize = 2000000;
int conf_gatk_ncontigs = 32;
int conf_gatk_nprocs = 16;
int conf_gatk_memory = 8;
int conf_gatk_nct = 4;

int conf_bqsr_nprocs = 16;
int conf_bqsr_memory = 8;
int conf_bqsr_nct = 4;
int conf_pr_nprocs = 16;
int conf_pr_memory = 8;
int conf_pr_nct = 4;
int conf_htc_nprocs = 16;
int conf_htc_memory = 8;
int conf_htc_nct = 4;

int init_config() {
  
  // First pass: static settings
  conf_bin_dir  = get_bin_dir();
  conf_root_dir = conf_bin_dir + "/..";
  conf_temp_dir = "/tmp";
  conf_log_dir  = get_absolute_path("./log");
  conf_project_dir  = get_absolute_path(".fcs-genome");

  conf_bwa_call = conf_root_dir + "/tools/bin/bwa-bin";
  conf_sambamba_call = conf_root_dir + "/tools/bin/sambamba";
  conf_bcftools_call = conf_root_dir + "/tools/bin/bcftools";
  conf_bgzip_call = conf_root_dir + "/tools/bin/bgzip";
  conf_tabix_call = conf_root_dir + "/tools/bin/tabix";
  conf_java_call = "java -d64 ";
  conf_gatk_path = conf_root_dir + "/tools/package/GenomeAnalysisTK.jar";
  conf_gatk_call = conf_java_call + "-jar " + conf_gatk_path;

  // Second pass: load settings from file
  conf_default_ref = "/space/scratch/genome/ref/human_g1k_v37.fasta";
  
  // Third pass: load settings from user's file

  // Fourth pass: load settings from ENV

  DLOG(INFO) << "conf_root_dir = " << conf_root_dir;
  DLOG(INFO) << "conf_log_dir = " << conf_log_dir;
  DLOG(INFO) << "conf_default_ref = " << conf_default_ref;
  DLOG(INFO) << "conf_bwa_call = " << conf_bwa_call;

  return 0;
}

int init(const char* argv) {
  // Intialize configs
  int ret = init_config();

  // Other starting procedures
  create_dir(conf_log_dir);

  return ret;
}

inline std::string get_intv_fname(int contig) {
  std::stringstream ss;
  ss << conf_intv_dir << "/intv" << contig+1 << ".list";
  return ss.str();
}

std::vector<std::string> init_contig_intv(std::string ref_path) {
  std::stringstream ss;
  ss << conf_project_dir << "/intv_" << conf_gatk_ncontigs;
  std::string intv_dir = ss.str();

  bool is_ready = true;
  std::vector<std::string> intv_paths(conf_gatk_ncontigs);
   
  for (int contig = 0; contig < conf_gatk_ncontigs; contig++) {
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
      << "-n " << conf_gatk_ncontigs;

    DLOG(INFO) << cmd.str();

    if (system(cmd.str().c_str())) {
      throw internalError("cannot initialize contig intervals");
    }
  }
  return intv_paths; 
}

} // namespace fcsgenome
