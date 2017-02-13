#ifndef FCSGENOME_WORKERS_HTCWORKER_H
#define FCSGENOME_WORKERS_HTCWORKER_H

#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class DOCWorker : public Worker {
 public:
  DOCWorker(std::string ref_path,
      std::string input_path,
      std::string output_path,
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
