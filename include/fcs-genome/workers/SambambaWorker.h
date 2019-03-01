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
    INDEX,
    SORT
  } Action;

  SambambaWorker(std::string input_path,
      std::string output_path, 
      Action action, 
      std::string common,
      bool &flag_f,
      std::vector<std::string> files = std::vector<std::string>() /* other files you want to 
                                                                  add except from files in input_path */
  );

  //  std::string getAction();
  void check();
  void setup();

  std::vector<std::string> get_files() {
    return input_files_;
  }

 private:
  std::string input_path_;
  std::vector<std::string> input_files_;
  std::string output_file_;
  Action action_;
  std::string common_;
  bool flag_f_;
};

} // namespace fcsgenome
#endif
