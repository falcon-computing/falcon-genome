#ifndef FCSGENOME_WORKERS_MARKDUPWORKER_H
#define FCSGENOME_WORKERS_MARKDUPWORKER_H
#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class MarkdupWorker : public Worker {
 public:
  MarkdupWorker(std::string input_path,
      std::string output_path,
      bool &flag_f);

  void check();
  void setup();
 private:
  std::string input_path_;
  std::vector<std::string> input_files_;
  std::string output_file_;
};

} // namespace fcsgenome
#endif
