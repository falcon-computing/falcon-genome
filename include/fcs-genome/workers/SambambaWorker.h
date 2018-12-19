#ifndef FCSGENOME_WORKERS_SAMBAMBAWORKER_H
#define FCSGENOME_WORKERS_SAMBAMBAWORKER_H
#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class SambambaWorker : public Worker {
 public:
  typedef enum {
    MARKDUP,
    MERGE,
    SORT
  } Action;
  SambambaWorker(std::string input_path,
      std::string output_path, 
      Action action, 
      bool &flag_f
  );

  //  std::string getAction();
  void check();
  void setup();
 private:
  std::string input_path_;
  std::vector<std::string> input_files_;
  std::string output_file_;
  Action action_;
};

} // namespace fcsgenome
#endif
