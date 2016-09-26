#ifndef FCSGENOME_WORKERS_PRWORKER_H
#define FCSGENOME_WORKERS_PRWORKER_H

#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class PRWorker : public Worker {
 public:
  PRWorker(std::string ref_path,
      std::string intv_path,
      std::string bqsr_path,
      std::string input_path,
      std::string output_path,
      int contig, bool &flag_f);

  void check();
  void setup();

 private:
  std::string ref_path_;
  std::string intv_path_;
  std::string bqsr_path_;
  std::string input_path_;
  std::string output_path_;
};
} // namespace fcsgenome
#endif
