#ifndef FCSGENOME_WORKERS_VCFCONCATWORKER_H
#define FCSGENOME_WORKERS_VCFCONCATWORKER_H

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
  TabixWorker(std::string input_path);

  void check();
  void setup();
 private:
  std::string input_file_; 
};
} // namespace fcsgenome
#endif
