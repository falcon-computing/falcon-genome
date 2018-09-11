#ifndef FCSGENOME_WORKERS_SAMBAMBAWORKER_H
#define FCSGENOME_WORKERS_SAMBAMBAWORKER_H
#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class SambambaWorker : public Worker {
 public:
  typedef enum {
    MARKDUP,
    MERGE
  } Action;
  SambambaWorker(std::string input_path,
		 std::string output_path, Action action, //std::string action,
      bool &flag_f);

  void check();
  void setup();
 private:
  std::string input_path_;
  std::vector<std::string> input_files_;
  std::string output_file_;
  //std::string action_;
  Action action_;
};

} // namespace fcsgenome
#endif
