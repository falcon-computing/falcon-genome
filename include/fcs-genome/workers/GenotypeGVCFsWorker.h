#ifndef FCSGENOME_WORKERS_GENOTYPEGVCFSWORKER_H
#define FCSGENOME_WORKERS_GENOTYPEGVCFSWORKER_H

#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class GenotypeGVCFsWorker : public Worker {
 public:
  GenotypeGVCFsWorker(std::string ref_path,
      std::string input_path,
      std::string output_path,
      std::vector<std::string> extra_opts,
      bool &flag_f);

  void check();
  void setup();

 private:
  std::string ref_path_;
  std::string input_path_;
  std::string output_path_;
};
} // namespace fcsgenome
#endif
