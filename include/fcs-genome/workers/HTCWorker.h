#ifndef FCSGENOME_WORKERS_HTCWORKER_H
#define FCSGENOME_WORKERS_HTCWORKER_H

#include <string>
#include <vector>
#include "fcs-genome/BamInput.h"
#include "fcs-genome/Worker.h"

namespace fcsgenome {
class HTCWorker : public Worker {
 public:
  HTCWorker(std::string ref_path,
      std::vector<std::string> intv_paths,
      //std::vector<std::string> input_paths,
      std::string input_paths,
      std::string output_path,
      std::vector<std::string> extra_opts,
      int contig,
      bool flag_vcf,
      bool &flag_f,
      bool flag_gatk);

  void check();
  void setup();

 private:
  int  contig_;       // which bam parts are we working on in this worker
  bool produce_vcf_;  // whether we produce vcf of gvcf
  bool flag_gatk_;    // whether we use GATK4
  std::string ref_path_;
  std::vector<std::string> intv_paths_; 
  BamInput input_paths_;
  std::string output_path_;
};
} // namespace fcsgenome
#endif
