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

extern boost::program_options::variables_map config_vtable;

class internalError;

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

int init(const char* argv);

std::vector<std::string> init_contig_intv(std::string ref_path);

} // namespace fcsgenome
#endif
