#ifndef FCSGENOME_WORKERS_Mutect2WORKER_H
#define FCSGENOME_WORKERS_Mutect2WORKER_H

#include <string>
#include <vector>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class Mutect2Worker : public Worker {
 public:
  Mutect2Worker(std::string ref_path,
      std::string intv_path,
      std::string input_path1,
      std::string input_path2,
      std::string output_path,
      std::string dbsnp_path,
      int contig,
      bool &flag_f);

  void check();
  void setup();

 private:
  std::string ref_path_;
  std::string intv_path_;
  std::string input_path1_;
  std::string input_path2_;
  std::string output_path_;
  std::string dbsnp_path_;
};
} // namespace fcsgenome
#endif
