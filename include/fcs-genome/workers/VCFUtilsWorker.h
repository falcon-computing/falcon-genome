#ifndef FCSGENOME_WORKERS_VCFUTILSWORKER_H
#define FCSGENOME_WORKERS_VCFUTILSWORKER_H

#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class VCFConcatWorker : public Worker {
 public:
  VCFConcatWorker(
      std::vector<std::string> &input_files,
      std::string output_path,
      bool &flag_f);

  void check();
  void setup();
 private:
  std::vector<std::string>& input_files_;
  std::string output_file_; 
};

class ZIPWorker : public Worker {
 public:
  ZIPWorker(std::string input_path,
      std::string output_path,
      bool &flag_f);

  void check();
  void setup();
 private:
  std::string input_file_; 
  std::string output_file_; 
};

class TabixWorker : public Worker {
 public:
  TabixWorker(std::string path):
    Worker(1, 1), path_(path) {}

  void check();
  void setup();
 private:
  std::string path_; 
};

class VCFSortWorker : public Worker {
 public:
  VCFSortWorker(std::string path): 
    Worker(1, 1), path_(path) {}

  void check();
  void setup();
 private:
  std::string path_; 
};
} // namespace fcsgenome
#endif
