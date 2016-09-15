#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <iostream>
#include <unistd.h>
#include "fcs-genome/common.h"
#include "fcs-genome/config.h"

namespace fcsgenome {

std::string get_basename_wo_ext(std::string path) {
  boost::filesystem::wpath file_path(path);
  return file_path.stem().string();
}

std::string get_absolute_path(std::string path) {
  boost::filesystem::wpath file_path(path);
  if (boost::filesystem::exists(file_path)) {
    return boost::filesystem::canonical(file_path).string();
  }
  else {
    if (file_path.is_absolute()) {
      return path;
    }
    else {
      file_path = boost::filesystem::current_path() / file_path;
      return file_path.string();
    }
  }
}

// Check input file exist and return absolute path
std::string check_input(std::string path) {
  if (!boost::filesystem::exists(path)) {
    throw fileNotFound("Cannot find " + path);
  }
  return get_absolute_path(path);
}

// Check output file exist with overwritting prompt 
// and return absolute path
std::string check_output(std::string path, bool f) {
  if (boost::filesystem::exists(path)) {
    if (!f) {
      std::string user_input; 
      std::cout << "Output file or directory '" << path
                << "' already exists, overwrite? (yes|no): ";
      std::cin >> user_input;
      while (user_input != "yes" && user_input != "no") {
        std::cout << "Please type \"yes\" or \"no\": ";
        std::cin >> user_input;
      }
      if (user_input == "no") {
        throw silentExit();
      }
    }
    // Remove output file
    boost::filesystem::remove_all(path);
  }
  return get_absolute_path(path);
}

std::string get_input_list(
    std::string path,
    std::string pattern,
    std::string prefix) {

  if (!boost::filesystem::exists(path)) {
    throw fileNotFound("Specified path " + path + " does not exist");
  }
  if (!boost::filesystem::is_directory(path)) {
    return prefix + path; 
  }

  std::string file_list;

  // construct regex
  boost::regex regex_pattern(pattern, boost::regex::normal);

  // iterate directory, recursively 
  boost::filesystem::directory_iterator end_iter;
  for (boost::filesystem::directory_iterator iter(path);
       iter != end_iter; iter++) {
    if (boost::filesystem::is_directory(iter->status())) {
      file_list += get_input_list(path, pattern, prefix);
    }
    else {
      std::string filename = iter->path().string(); 
      if (boost::regex_match(filename, regex_pattern)) {
        file_list += prefix + filename + " ";
      }
    }
  }
  return file_list;
}

std::string get_bin_dir() {
  std::stringstream ss;
  ss << "/proc/" << getpid() << "/exe";

  boost::filesystem::wpath bin_path(get_absolute_path(ss.str()));
  return bin_path.parent_path().string();
}
} // namespace fcsgenome
