#ifndef FCSGENOME_WORKERS_HTCWORKER_H
#define FCSGENOME_WORKERS_HTCWORKER_H

#include <string>
#include <vector>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class HTCWorker : public Worker {
 public:
  HTCWorker(std::string ref_path,
      std::string intv_path,
      std::string input_path,
      std::string output_path,
      std::vector<std::string> extra_opts,
      std::string &intv_list,
      int contig,
      bool flag_vcf,
      bool &flag_f,
      bool flag_gatk);

  void check();
  void setup();

 private:
  bool produce_vcf_;
  bool flag_gatk_;
  std::string ref_path_;
  std::string intv_path_;
  std::string input_path_;
  std::string output_path_;
  std::vector<std::string> intv_list_;
};
} // namespace fcsgenome
#endif
