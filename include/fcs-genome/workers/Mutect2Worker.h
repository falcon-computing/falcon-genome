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
      std::string normal_path,
      std::string tumor_path,
      std::string output_path,
      std::vector<std::string> &dbsnp_path,
      std::vector<std::string> &cosmic_path,
      int contig,
      bool &flag_f);

  void check();
  void setup();

 private:
  std::string ref_path_;
  std::string intv_path_;
  std::string normal_path_;
  std::string tumor_path_;
  std::string output_path_;
  std::vector<std::string> &dbsnp_path_;
  std::vector<std::string> &cosmic_path_;
};
} // namespace fcsgenome
#endif
