#ifndef FCSGENOME_WORKERS_DEPTHCOMBINEWORKER_H
#define FCSGENOME_WORKERS_DEPTHCOMBINEWORKER_H

#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class DepthCombineWorker : public Worker {
 public:
  DepthCombineWorker(
      std::vector<std::string> &input_files,
      std::string output_path,
      bool &flag_baseCoverage,
      bool &flag_intervalCoverage,
      bool &flag_sampleSummary,
      bool &flag_f);

  void check();
  void merge_sample_summary(std::string file_type);
  void merge_gene_summary(std::string file_type);
  void merge_interval_summary(std::string file_type);
  void merge_base_summary(std::string file_type);
  void setup();
 private:
  std::vector<std::string> input_files_;
  std::string output_file_;
  bool flag_baseCoverage_;
  bool flag_intervalCoverage_;
  bool flag_sampleSummary_;
};
} //namespace fcsgenome
#endif
