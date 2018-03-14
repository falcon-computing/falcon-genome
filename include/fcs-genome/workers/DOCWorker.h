#ifndef FCSGENOME_WORKERS_HTCWORKER_H
#define FCSGENOME_WORKERS_HTCWORKER_H

#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class DOCWorker : public Worker {
 public:
  DOCWorker(std::string ref_path,
      std::string input_path,
      std::string output_path,
      std::string geneList,
      std::string intervalList,
      int depthCutoff,
      bool &flag_f,
      bool &flag_baseCoverage,
      bool &flag_intervalCoverage,
      bool &flag_sampleSummary);

  void check();
  void setup();

 private:
  std::string ref_path_;
  std::string input_path_;
  std::string output_path_;
  std::string geneList_;
  std::string intervalList_;
  int depthCutoff_;
  bool flag_baseCoverage_;
  bool flag_intervalCoverage_;
  bool flag_sampleSummary_;
};
} // namespace fcsgenome
#endif
