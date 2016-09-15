#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <glog/logging.h>
#include <string>
#include <sys/syscall.h>
#include <sys/time.h>
#include <syscall.h>
#include <time.h>

namespace fcsgenome {

// Custom exceptions
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
#define arg_decl_string_w_def(arg, defval, msg) (arg, \
    po::value<std::string>(&str_opt)->default_value(defval), \
    msg)

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

template <>
inline bool get_argument<bool>(
    boost::program_options::variables_map &vm,
    const char* arg
) {
  return vm.count(arg);
}

std::string get_basename(std::string path);
std::string get_basename_wo_ext(std::string path);
std::string get_absolute_path(std::string path);
std::string check_input(std::string path);
std::string check_output(std::string path, bool f);
std::string get_input_list(std::string path, 
    std::string pattern = ".*",
    std::string prefix = "");
std::string get_bin_dir();

// Worker functions
int bwa_command(std::string& ref_path,
    std::string& fq1_path, std::string& fq2_path,
    std::string& output_path,
    std::string& sample_id, std::string& read_group,
    std::string& platform_id, std::string& library_id);

int markdup_command(std::string& input_files, 
    std::string& output_path);

int align_main(int argc, char** argv);
int markdup_main(int argc, char** argv);

} // namespace fcsgenome
