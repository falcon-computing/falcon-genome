#ifndef FCSGENOME_CONFIG_H
#define FCSGENOME_CONFIG_H

#include <glog/logging.h>
#include <string>

namespace fcsgenome {

extern std::string conf_project_dir;
extern std::string conf_bin_dir;
extern std::string conf_root_dir;
extern std::string conf_temp_dir;
extern std::string conf_log_dir;
extern std::string conf_default_ref;
extern std::string conf_bwa_call;
extern std::string conf_sambamba_call;
extern std::string conf_bcftools_call;
extern std::string conf_bgzip_call;
extern std::string conf_tabix_call;
extern std::string conf_java_call;
extern std::string conf_gatk_path;
extern std::string conf_gatk_call;
extern int         conf_bwa_maxrecords;
extern int         conf_markdup_maxfile;
extern int         conf_markdup_nthreads;
extern int         conf_markdup_overflowsize;
extern int         conf_gatk_ncontigs;
extern int         conf_gatk_nprocs;
extern int         conf_gatk_memory;
extern int         conf_gatk_nct;
extern int         conf_bqsr_nprocs;
extern int         conf_bqsr_memory;
extern int         conf_bqsr_nct;
extern int         conf_pr_nprocs;
extern int         conf_pr_memory;
extern int         conf_pr_nct;
extern int         conf_htc_nprocs;
extern int         conf_htc_memory;
extern int         conf_htc_nct;

int init(const char* argv);

std::vector<std::string> init_contig_intv(std::string ref_path);

} // namespace fcsgenome
#endif
