#ifndef FCSGENOME_WORKERS_UGWORKER_H
#define FCSGENOME_WORKERS_UGWORKER_H

#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class UGWorker : public Worker {
 public:
  UGWorker(std::string ref_path,
      std::string input_path,
      std::string intv_path,
      std::string output_path,
      std::vector<std::string> extra_opts,
      std::vector<std::string> &intv_list,
      bool &flag_f);

  void check();
  void setup();

 private:
  std::string ref_path_;
  std::string intv_path_;
  std::string input_path_;
  std::string output_path_;
  std::vector<std::string> intv_list_;
};
} // namespace fcsgenome
#endif
