#include <glog/logging.h>
#include <string>

namespace fcsgenome {

extern std::string conf_root_dir;
extern std::string conf_temp_dir;
extern std::string conf_log_dir;
extern std::string conf_default_ref;
extern std::string conf_bwa_call;
extern std::string conf_sambamba_call;
extern int         conf_bwa_maxrecords;
extern int         conf_markdup_maxfile;
extern int         conf_markdup_nthreads;
extern int         conf_markdup_overflowsize;


int init(const char* argv);

} // namespace fcsgenome
