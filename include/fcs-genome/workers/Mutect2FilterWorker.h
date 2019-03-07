#ifndef FCSGENOME_WORKERS_Mutect2FilterWORKER_H
#define FCSGENOME_WORKERS_Mutect2FilterWORKER_H

#include <string>
#include <vector>
#include "fcs-genome/Worker.h"

namespace fcsgenome {
class Mutect2FilterWorker : public Worker {
 public:
  Mutect2FilterWorker(
     std::vector<std::string> intv_path,
     std::string input_path,
     std::string tumor_table,
     std::string output_path,
     std::vector<std::string> filtering_extra_opts,
     bool &flag_f, 
     bool flag_gatk);

  void check();
  void setup();

 private:
  std::vector<std::string> intv_path_;
  std::string input_path_;
  std::string tumor_table_;
  std::string output_path_;
  bool flag_gatk_;
};
} // namespace fcsgenome
#endif
