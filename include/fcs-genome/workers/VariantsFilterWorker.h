#ifndef FCSGENOME_WORKERS_VARIANTSFILTERWORKER_H
#define FCSGENOME_WORKERS_VARIANTSFILTERWORKER_H

#include <string>
#include <vector>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class VariantsFilterWorker : public Worker {
 public:
  VariantsFilterWorker(std::string ref_path,
      std::string intv_path,
      std::string input_path,
      std::string output_path,
      std::vector<std::string> extra_opts,
      int contig,
      std::string filtering_par,
      std::string filter_name,
      bool flag_vcf,
      bool &flag_f,
      bool flag_gatk);

  void check();
  void setup();

 private:
  bool produce_vcf_;
  bool flag_gatk_;
  std::string ref_path_;
  std::string intv_path_;
  std::string input_path_;
  std::string output_path_;
  std::string filtering_par_;
  std::string filter_name_;
};
} // namespace fcsgenome
#endif
