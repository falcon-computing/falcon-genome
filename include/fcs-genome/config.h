#ifndef FCSGENOME_CONFIG_H
#define FCSGENOME_CONFIG_H

#include <boost/program_options.hpp>
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

int init(char** argv, int argc);
int init_config();

std::vector<std::string> init_contig_intv(std::string ref_path);

} // namespace fcsgenome
#endif
