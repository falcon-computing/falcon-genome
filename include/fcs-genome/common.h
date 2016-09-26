#ifndef FCSGENOME_COMMON_H
#define FCSGENOME_COMMON_H

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <glog/logging.h>
#include <iomanip>
#include <string>
#include <sys/syscall.h>
#include <sys/time.h>
#include <syscall.h>
#include <time.h>

#include "config.h"

namespace fcsgenome {

// Custom exceptions

class helpRequest: public std::runtime_error {
public:
  explicit helpRequest():
    std::runtime_error("") {;}
};

class silentExit: public std::runtime_error {
public:
  explicit silentExit():
    std::runtime_error("") {;}
};

class invalidParam : public std::runtime_error {
public:
  explicit invalidParam(const std::string& what_arg):
    std::runtime_error(what_arg) {;}
};

class fileNotFound : public std::runtime_error {
public:
  explicit fileNotFound(const std::string& what_arg):
    std::runtime_error(what_arg) {;}
};
class failedCommand : public std::runtime_error {
public:
  explicit failedCommand(const std::string& what_arg):
    std::runtime_error(what_arg) {;}
};

class internalError : public std::runtime_error {
public:
  explicit internalError(const std::string& what_arg):
    std::runtime_error(what_arg) {;}
};

// Macros for argument definition
#define arg_decl_string(arg, msg) (arg, \
    po::value<std::string>(), \
    msg)

inline uint64_t getTs() {
  struct timespec tr;
  clock_gettime(CLOCK_REALTIME, &tr);
  return (uint64_t)tr.tv_sec;
}

inline uint64_t getUs() {
  struct timespec tr;
  clock_gettime(CLOCK_REALTIME, &tr);
  return (uint64_t)tr.tv_sec*1e6 + tr.tv_nsec/1e3;
}

inline void log_time(std::string stage_name, uint64_t start_ts) {
  LOG(INFO) << stage_name << " finishes in "
            << getTs() - start_ts << " seconds";
}

inline void create_dir(std::string path) {
  if (!boost::filesystem::exists(path)) {
    if (!boost::filesystem::create_directories(path)) {
      throw fileNotFound("Cannot create dir: " + path);
    }
  }
}

inline void remove_path(std::string path) {
  if (boost::filesystem::exists(path)) {
    boost::filesystem::remove_all(path);
  }
}

template <class T>
inline T get_argument(
    boost::program_options::variables_map &vm,
    const char* arg
) {
  if (!vm.count(arg)) {
    throw invalidParam(arg);
  }
  else {
    return vm[arg].as<T>();
  }
}

template <class T>
inline T get_argument(
    boost::program_options::variables_map &vm,
    const char* arg,
    T def_val
) {
  if (!vm.count(arg)) {
    return def_val;
  }
  else {
    return vm[arg].as<T>();
  }
}

template <>
inline bool get_argument<bool>(
    boost::program_options::variables_map &vm,
    const char* arg
) {
  return vm.count(arg);
}

inline std::string get_contig_fname(
    std::string base_path,
    int contig,
    std::string ext = "bam") 
{
  int n_digits = (int)log10((double)conf_gatk_ncontigs)+1;
  std::stringstream ss;
  ss << base_path << "/part-" 
     << std::setw(n_digits) << std::setfill('0') << contig
     << "." << ext;
  return ss.str();
}

std::string get_basename(std::string path);
std::string get_basename_wo_ext(std::string path);
std::string get_absolute_path(std::string path);
std::string check_input(std::string path);
std::string check_output(std::string path, bool &f, bool req_file = false);
std::string get_bin_dir();
std::string get_log_name(std::string job_name, int idx = -1);
void get_input_list(std::string path, 
    std::vector<std::string> &list,
    std::string pattern = ".*");

} // namespace fcsgenome
#endif
