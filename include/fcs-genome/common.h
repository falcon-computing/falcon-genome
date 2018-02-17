#ifndef FCSGENOME_COMMON_H
#define FCSGENOME_COMMON_H

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <iomanip>
#include <string>
#include <sys/syscall.h>
#include <sys/time.h>
#include <syscall.h>
#include <time.h>

#ifdef NDEBUG
#define LOG_HEADER "fcs-genome"
#endif
#include <glog/logging.h>

#include "config.h"
#include "Executor.h"

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
#define arg_decl_int(arg, msg) (arg, \
    po::value<int>(), \
    msg)

#define arg_decl_string(arg, msg) (arg, \
    po::value<std::string>(), \
    msg)

#define arg_decl_bool(arg, msg) (arg, \
    po::value<bool>(), \
    msg)

#define arg_decl_bool_w_def(arg, msg) (arg, \
    po::value<bool>(&opt_bool)->default_value(true), \
    msg)

#define arg_decl_string_w_def(arg, val, msg) (arg, \
    po::value<std::string>(&opt_str)->default_value(val), \
    msg)

#define arg_decl_int_w_def(arg, val, msg) (arg, \
    po::value<int>(&opt_int)->default_value(val), \
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
    //if (!boost::filesystem::exists(path)) {
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
  //if (!vm.count(arg)) {
  //  return false;
  //}
  //else {
  //  return vm[arg].as<bool>();
  //}
}

template <>
inline std::string get_argument<std::string>(
    boost::program_options::variables_map &vm,
    const char* arg,
    std::string def_val
) {
  if (!vm.count(arg)) {
    if (def_val.empty()) {
      throw invalidParam(arg);
    }
    else {
      return def_val;
    }
  }
  else {
    return vm[arg].as<std::string>();
  }
}

inline std::string get_contig_fname(
    std::string base_path,
    int contig,
    std::string ext = "bam",
    std::string prefix = "part-") 
{
  int n_digits = (int)log10((double)get_config<int>("gatk.ncontigs"))+1;
  std::stringstream ss;
  ss << base_path << "/" << prefix
     << std::setw(n_digits) << std::setfill('0') << contig
     << "." << ext;
  return ss.str();
}

inline std::string get_basename(std::string path) {
  boost::filesystem::wpath file_path(path);
  return file_path.filename().string();
}

inline std::string get_basename_wo_ext(std::string path) {
  boost::filesystem::wpath file_path(path);
  return file_path.stem().string();
}

Executor* create_executor(std::string job_name, int num_workers = 1);
std::string get_absolute_path(std::string path);
std::string check_input(std::string path);
std::string check_output(std::string path, bool &f, bool req_file = false);
std::string get_bin_dir();
std::string get_log_name(std::string job_name, int idx = -1);
void get_input_list(std::string path, 
    std::vector<std::string> &list,
    std::string pattern = ".*",
    bool recursive = false);

std::vector<std::string> get_lines(std::string fname,
    std::string pattern = ".*");

class Executor;
class Worker;

extern Executor* g_executor;

} // namespace fcsgenome
#endif
