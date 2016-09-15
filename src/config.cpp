#include <string>
#include "fcs-genome/common.h"
#include "fcs-genome/config.h"

namespace fcsgenome {

std::string conf_bin_dir;
std::string conf_root_dir;
std::string conf_temp_dir;
std::string conf_log_dir;
std::string conf_default_ref;
std::string conf_bwa_call;
std::string conf_sambamba_call;

int conf_bwa_maxrecords = 10000000;
int conf_markdup_maxfile = 4096;
int conf_markdup_nthreads = 16;
int conf_markdup_overflowsize = 2000000;

int init_config() {
  
  // First pass: static settings
  conf_bin_dir  = get_bin_dir();
  conf_root_dir = conf_bin_dir + "/..";
  conf_temp_dir = "/tmp";
  conf_log_dir  = get_absolute_path("./log");
  conf_bwa_call = conf_root_dir + "/tools/bin/bwa-bin";
  conf_sambamba_call = conf_root_dir + "/tools/bin/sambamba";

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
} // namespace fcsgenome
