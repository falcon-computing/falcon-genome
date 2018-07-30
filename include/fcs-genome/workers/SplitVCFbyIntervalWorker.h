#ifndef FCSGENOME_WORKERS_SPLITVCFBYINTERVALWORKER_H
#define FCSGENOME_WORKERS_SPLITVCFBYINTERVALWORKER_H

#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class SplitVCFbyIntervalsWorker : public Worker(
 public:
   SplitVCFbyIntervalsWorker(
      std::vector<std::string> inputVCF,
      std::vector<std::string> intervalSet,
      std::vector<std::string> &vcfSets,
      std::string commonString, bool &flag_f
   );
   void check();
   void setup();
 private:
   std::vector<std::string> inputVCF_;
   std::vector<std::vector> intervalSet_;
   std::string commonString_;
};

} // namespace fcsgenome
#endif
