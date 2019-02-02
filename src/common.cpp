#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <time.h>
#include <unistd.h>
#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"

namespace fcsgenome {

Executor* g_executor = NULL;

Executor* create_executor(std::string job_name,  int num_workers) {
  Executor* executor = new Executor(job_name, num_workers);  
  g_executor = executor;

  return executor;
}

uint32_t getTid() {
  static bool lacks_gettid = false;

  if (!lacks_gettid) {
    pid_t tid = syscall(__NR_gettid);
    if (tid != -1) {
      return (uint32_t)tid;
    }
    // Technically, this variable has to be volatile, but there is a small
    // performance penalty in accessing volatile variables and there should
    // not be any serious adverse effect if a thread does not immediately see
    // the value change to "true".
    lacks_gettid = true;
  }

  // If gettid() could not be used, we use one of the following.
  return (uint32_t)getpid(); 
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
std::string check_input(std::string path, bool req) {
  if (!boost::filesystem::exists(path)) {
    if (req) {
      throw fileNotFound("Cannot find " + path);
    }
    else {
      LOG(WARNING) << "Cannot find " << path;
      return path;
    }
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
  else {
    // check if output dir is writable
    std::ofstream fout;
    fout.open(path.c_str());
    if (!fout) {
      throw fileNotFound("Cannot write to output path '"+path+"'");
    }
    fout.close();
    remove_path(path);
  }
  return get_absolute_path(path);
}

void list_dir(
    std::string path,
    std::vector<std::string> &list,
    std::string pattern,
    bool recursive) 
{
  // construct regex
  boost::regex regex_pattern(pattern, boost::regex::normal);

  // iterate directory, recursively 
  // TODO: sort
  boost::filesystem::directory_iterator end_iter;
  for (boost::filesystem::directory_iterator iter(path);
      iter != end_iter; iter++) {
    if (boost::filesystem::is_directory(iter->status()) && recursive) {
      list_dir(iter->path().string(), list, pattern, recursive);
    }
    else {
      std::string filename = iter->path().string(); 
      if (boost::regex_match(filename, regex_pattern)) {
        list.push_back(filename);
      }
    }
  }
}

void get_input_list(
    std::string path,
    std::vector<std::string> &list,
    std::string pattern,
    bool recursive) 
{
  if (!boost::filesystem::exists(path)) {
    throw fileNotFound("Path '" + path + "' does not exist");
  }
  if (!boost::filesystem::is_directory(path)) {
    list.push_back(path);
  }
  else {
    list_dir(path, list, pattern, recursive); 

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

std::vector<std::string> get_lines(std::string fname, std::string pattern) {
  std::vector<std::string> lines;
  std::ifstream fin;
  fin.open(fname);

  // construct regex
  boost::regex regex_pattern(pattern, boost::regex::normal);

  std::string line;
  while (std::getline(fin, line, '\n')) {
    if (boost::regex_match(line, regex_pattern)) {
      lines.push_back(line);
    }
  }
  return lines;
}

bool is_folder_writable(const char* str) {
  if (access(str, W_OK) == 0) {
    return true;
  } 
  else {
    return false;   
  }
}

std::string get_fname_by_ext(std::string fname, std::string ext) {
  boost::filesystem::path p(fname);
  std::string output_fname;
  if (!p.parent_path().string().empty()) {
    output_fname = output_fname + p.parent_path().string() + "/";
  }

  


  if ( boost::filesystem::exists(output_fname + p.stem().string() + "." + ext)){
    return output_fname + p.stem().string() + "." + ext;
  }
  if ( boost::filesystem::exists(output_fname + p.stem().string() + ".[a-z]." + ext)){
    return output_fname + p.stem().string() + "." + ext;
  }

  


}


std::string get_bucket_fname(std::string dir, int contig, std::string stem, std::string ext) {
    std::stringstream ss;
    ss << dir << "/" << stem << "-"
       << std::setw(6) << std::setfill('0') << contig
       << ext;

    return ss.str();
}

int count_files_in_dir(std::string directory, std::string ext) {
  boost::filesystem::path Path(directory);
  int files_with_ext = 0;
  // Default constructor for an iterator is the end iterator
  boost::filesystem::directory_iterator end_iter;
  for (boost::filesystem::directory_iterator iter(Path); iter != end_iter; ++iter)
     if (iter->path().extension() == ext)
       ++files_with_ext;

  return files_with_ext;
}

} // namespace fcsgenome
