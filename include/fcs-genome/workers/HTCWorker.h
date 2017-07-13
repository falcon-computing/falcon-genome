#ifndef FCSGENOME_WORKERS_HTCWORKER_H
#define FCSGENOME_WORKERS_HTCWORKER_H

#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class HTCWorker : public Worker {
 public:
  HTCWorker(std::string ref_path,
      std::string intv_path,
      std::string input_path,
      std::string output_path,
      int contig,
      bool flag_vcf,
      bool &flag_f);

  void check();
  void setup();

 private:
  bool produce_vcf_;
  std::string ref_path_;
  std::string intv_path_;
  std::string input_path_;
  std::string output_path_;
};
} // namespace fcsgenome
#endif
