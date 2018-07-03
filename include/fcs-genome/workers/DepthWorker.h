#ifndef FCSGENOME_WORKERS_DepthWORKER_H
#define FCSGENOME_WORKERS_DepthWORKER_H

#include <string>
#include <vector>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class DepthWorker : public Worker {
 public:
  DepthWorker(std::string ref_path,
      std::vector<std::string> intv_path,
      std::string input_path,
      std::string output_path,
      std::vector<std::string> geneList_paths,
      int depthCutoff,
      std::vector<std::string> extra_opts,
      int contig,
      bool &flag_f,
      bool &flag_baseCoverage,
      bool &flag_intervalCoverage,
      bool &flag_sampleSummary);

  void check();
  void setup();

 private:
  std::string ref_path_;
  std::vector<std::string> intv_path_;
  std::string input_path_;
  std::string output_path_;
  std::vector<std::string> geneList_paths_;
  int depthCutoff_;
  bool flag_baseCoverage_;
  bool flag_intervalCoverage_;
  bool flag_sampleSummary_;
};
} // namespace fcsgenome
#endif
