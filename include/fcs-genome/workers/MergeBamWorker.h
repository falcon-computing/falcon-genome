#ifndef FCSGENOME_WORKERS_MERGEBAMWORKER_H
#define FCSGENOME_WORKERS_MERGEBAMWORKER_H
#include <string>
#include <sstream>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class MergeBamWorker : public Worker {
 public:
  MergeBamWorker(std::string input_path,
      std::string output_path,
      bool &flag_f);
  void setup();
 private:
  std::string inputPartsBAM_;
  std::string output_file_;
};

} // namespace fcsgenome
#endif
