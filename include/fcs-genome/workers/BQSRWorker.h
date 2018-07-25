#ifndef FCSGENOME_WORKERS_BQSRWORKER_H
#define FCSGENOME_WORKERS_BQSRWORKER_H

#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class BQSRWorker : public Worker {
 public:
  BQSRWorker(std::string ref_path,
      std::vector<std::string> &known_sites,
      std::string intv_path,
      std::string input_path,
      std::string output_path,
      std::vector<std::string> extra_opts,
      std::vector<std::string> &intv_list,
      int contig,
      bool &flag_f,
      bool flag_gatk);

  void check();
  void setup();

 private:
  std::vector<std::string> known_sites_;
  std::string ref_path_;
  std::string intv_path_;
  std::string input_path_;
  std::string output_path_;
  std::vector<std::string> intv_list_;
  bool flag_gatk_;
};

class BQSRGatherWorker : public Worker {
 public:
  BQSRGatherWorker(std::vector<std::string> &input_files,
      std::string output_file,
      bool &flag_f, bool flag_gatk);

  void check();
  void setup();
 private:
  std::vector<std::string> input_files_;
  std::string output_file_;
};

class PRWorker : public Worker {
 public:
  PRWorker(std::string ref_path,
      std::string intv_path,
      std::string bqsr_path,
      std::string input_path,
      std::string output_path,
      std::vector<std::string> extra_opts,
      std::vector<std::string> &intv_list,
      int contig, bool &flag_f, bool flag_gatk);

  void check();
  void setup();

 private:
  std::string ref_path_;
  std::string intv_path_;
  std::string bqsr_path_;
  std::string input_path_;
  std::string output_path_;
  std::vector<std::string> intv_list_;
  bool flag_gatk_;
};
} // namespace fcsgenome
#endif
