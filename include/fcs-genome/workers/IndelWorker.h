#ifndef FCSGENOME_WORKERS_INDELWORKER_H
#define FCSGENOME_WORKERS_INDELWORKER_H

#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class RTCWorker : public Worker {
 public:
  RTCWorker(std::string ref_path,
      std::vector<std::string> &known_indels,
      std::string input_path,
      std::string output_path);

  void check();
  void setup();

 private:
  std::string ref_path_;
  std::vector<std::string> known_indels_;
  std::vector<std::string> input_files_;
  std::string input_path_;
  std::string output_path_;
};

class IndelWorker : public Worker {
 public:
  IndelWorker(std::string ref_path,
      std::vector<std::string> &known_indels,
      std::string intv_path,
      std::string input_path,
      std::string target_path,
      std::string output_path,
      std::vector<std::string> extra_opts,
      bool &flag_f);

  void check();
  void setup();

 private:
  std::string ref_path_;
  std::vector<std::string> known_indels_;
  std::string intv_path_;
  std::string input_path_;
  std::string target_path_;
  std::string output_path_;
};


} // namespace fcsgenome
#endif
