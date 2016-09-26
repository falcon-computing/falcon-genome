#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <iostream>
#include <iomanip>
#include <time.h>
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
std::string check_output(std::string path, bool &f, bool require_file) {
  if (boost::filesystem::exists(path)) {
    if (require_file && boost::filesystem::is_directory(path)) {
      throw (fileNotFound("Output path " + path + " is not a file"));
    }
    if (!f) {
      std::string user_input; 
      std::cout << "Output file or directory '" << path
                << "' already exists, overwrite? (yes|no|all): ";
      std::cin >> user_input;
      while (user_input != "yes" &&
             user_input != "no" && 
             user_input != "all") {
        std::cout << "Please type \"yes\" or \"no\" or \"all\": ";
        std::cin >> user_input;
      }
      if (user_input == "no") {
        throw silentExit();
      }
      else if (user_input == "all") {
        f = true;
      }
    }
    LOG(INFO) << "Overwritting output '"
              << path << "'";
    // Remove output file
    boost::filesystem::remove_all(path);
  }
  return get_absolute_path(path);
}

void get_input_list(
    std::string path,
    std::vector<std::string> &list,
    std::string pattern) 
{
  if (!boost::filesystem::exists(path)) {
    throw fileNotFound("Path '" + path + "' does not exist");
  }
  if (!boost::filesystem::is_directory(path)) {
    list.push_back(path);
  }
  else {
    // construct regex
    boost::regex regex_pattern(pattern, boost::regex::normal);

    // iterate directory, recursively 
    // TODO: sort
    boost::filesystem::directory_iterator end_iter;
    for (boost::filesystem::directory_iterator iter(path);
        iter != end_iter; iter++) {
      if (boost::filesystem::is_directory(iter->status())) {
        get_input_list(iter->path().string(), list, pattern);
      }
      else {
        std::string filename = iter->path().string(); 
        if (boost::regex_match(filename, regex_pattern)) {
          list.push_back(filename);
        }
      }
    }
    if (list.empty()) {
      throw fileNotFound("Cannot find input file(s) in '" + path + "'");
    }
    std::sort(list.begin(), list.end());
  }
}

std::string get_bin_dir() {
  std::stringstream ss;
  ss << "/proc/" << getpid() << "/exe";

  boost::filesystem::wpath bin_path(get_absolute_path(ss.str()));
  return bin_path.parent_path().string();
}

std::string get_log_name(std::string job_name, int idx) {
  time_t timestamp = ::time(0);
  struct ::tm tm_time;
  localtime_r(&timestamp, &tm_time);

  // manipulate stage_name to replace all spaces and to lower case
  std::string log_name = job_name;
  std::transform(log_name.begin(), log_name.end(), log_name.begin(),
      [](char c) {
        return c == ' ' ? '-' : ::tolower(c);
      });

  std::stringstream ss;
  ss << conf_log_dir << "/"
     << log_name << "-"
     << 1900 + tm_time.tm_year
     << std::setw(2) << std::setfill('0') << 1+tm_time.tm_mon
     << std::setw(2) << std::setfill('0') << tm_time.tm_mday
     << '-'
     << std::setw(2) << std::setfill('0') << tm_time.tm_hour
     << std::setw(2) << std::setfill('0') << tm_time.tm_min
     << std::setw(2) << std::setfill('0') << tm_time.tm_sec
     << ".log";
  if (idx >= 0) {
    ss << "." << idx;
  }
  return ss.str();
}

unsigned int l_distance(const std::string& s1, const std::string& s2) 
{
  const std::size_t len1 = s1.size(), len2 = s2.size();
  std::vector<unsigned int> col(len2+1), prevCol(len2+1);
  
  for (unsigned int i = 0; i < prevCol.size(); i++)
    prevCol[i] = i;
  for (unsigned int i = 0; i < len1; i++) {
    col[0] = i+1;
    for (unsigned int j = 0; j < len2; j++)
                        // note that std::min({arg1, arg2, arg3}) works only in C++11,
                        // for C++98 use std::min(std::min(arg1, arg2), arg3)
      col[j+1] = std::min({ prevCol[1 + j] + 1, col[j] + 1, prevCol[j] + (s1[i]==s2[j] ? 0 : 1) });
    col.swap(prevCol);
  }
  return prevCol[len2];
}

} // namespace fcsgenome
