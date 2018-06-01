#ifndef FCSGENOME_WORKERS_DEPTHCOMBINEWORKER_H
#define FCSGENOME_WORKERS_DEPTHCOMBINEWORKER_H

#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class DepthCombineWorker : public Worker {
 public:
  DepthCombineWorker(
      std::vector<std::string> &input_files,
      std::string output_path,
      bool &flag_a,
      bool &flag_f);

  void check();
  void setup();
 private:
  std::vector<std::string> input_files_;
  std::string output_file_;
  bool flag_a_;
};
} //namespace fcsgenome
#endif
