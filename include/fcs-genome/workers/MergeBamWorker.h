#ifndef FCSGENOME_WORKERS_MERGEBAMWORKER_H
#define FCSGENOME_WORKERS_MERGEBAMWORKER_H
#include <string>
#include <sstream>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class MergeBamWorker : public Worker {
 public:
  MergeBamWorker(std::stringstream input_path,
      std::string output_path,
      bool &flag_f);

  //void check();
  void setup();
 private:
  std::stringstream inputPartsBAM_;
  //std::vector<std::string> input_files_;
  std::string output_file_;
};

} // namespace fcsgenome
#endif
