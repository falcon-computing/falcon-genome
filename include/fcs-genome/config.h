#ifndef FCSGENOME_CONFIG_H
#define FCSGENOME_CONFIG_H

#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <glog/logging.h>
#include <string>
#include "fcs-genome/common.h"

namespace fcsgenome {

extern std::string conf_project_dir;
extern std::string conf_bin_dir;
extern std::string conf_root_dir;
extern std::string conf_temp_dir;
extern std::vector<std::string> conf_host_list;
extern int main_tid;

extern boost::program_options::variables_map config_vtable;

class internalError;

inline bool check_config(std::string arg) {
  return config_vtable.count(arg);
}

template <typename T>
inline T get_config(std::string arg) {
  if (!config_vtable.count(arg)) {
    throw internalError("missing conf '"+arg+"'");
  }
  else {
    return config_vtable[arg].as<T>();
  }
}

template <>
inline bool get_config(std::string arg) {
  if (!config_vtable.count(arg)) {
    return false;
  }
  else {
    return config_vtable[arg].as<bool>();
  }
}

// get config with fall-back default config
template <typename T>
inline T get_config(std::string arg, std::string def_arg) {
  if (!config_vtable.count(arg)) {
    return get_config<T>(def_arg);
  }
  else {
    return get_config<T>(arg);
  }
}

// set config when it's not set
template <typename T>
void set_config(std::string arg, std::string def_arg) {
  if (!config_vtable.count(arg)) {
    if (!config_vtable.count(def_arg)) {
      throw internalError("missing conf '"+def_arg+"'");
    }
    config_vtable.insert(std::make_pair(arg, config_vtable[def_arg]));
    DLOG(INFO) << "setting " << arg << " with value of " << get_config<T>(def_arg);

    boost::program_options::notify(config_vtable);
  }
}

static inline int get_sys_memory() {
  uint64_t pages     = sysconf(_SC_PHYS_PAGES);
  uint64_t page_size = sysconf(_SC_PAGE_SIZE);
  return pages * page_size / 1024 / 1024 / 1024 + 1;
}

static int global_cpu_num = boost::thread::hardware_concurrency();
static int global_memory_size = get_sys_memory();

void calc_gatk_default_config(int & nprocs, int & memory,
    int cpu_num = global_cpu_num,
    int memory_size = global_memory_size);

int check_nprocs_config(std::string key, int cpu_num = global_cpu_num);
int check_memory_config(std::string key, int memory_size = global_memory_size);

int init(char** argv, int argc);
int init_config(boost::program_options::options_description conf_opt);

std::vector<std::string> init_contig_intv(std::string ref_path);
std::vector<std::string> split_ref_by_nprocs(std::string ref_path);
std::vector<std::string> split_by_nprocs(std::string intervalFile, std::string filetype);
void check_vcf_index(std::string inputVCF);

template <typename T>
std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b){
    assert(a.size() == b.size());
    std::vector<T> result;
    result.reserve(a.size());
    std::transform(a.begin(), a.end(), b.begin(), std::back_inserter(result), std::plus<T>());
    return result;
}

} // namespace fcsgenome
#endif
