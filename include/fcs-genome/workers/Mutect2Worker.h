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
      std::vector<std::string> extra_opts,
      std::vector<std::string> &dbsnp_path,
      std::vector<std::string> &cosmic_path,
      std::string &germline_path,
      std::string &panels_of_normals,
      std::string &normal_name,
      std::string &tumor_name,
      int contig,
      bool &flag_f, bool flag_gatk);

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
  std::string &germline_path_;
  std::string &panels_of_normals_;
  std::string &normal_name_;
  std::string &tumor_name_;
  bool flag_gatk_;
};
} // namespace fcsgenome
#endif
