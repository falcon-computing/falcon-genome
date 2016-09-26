#ifndef FCSGENOME_WORKERS_HTCWORKER_H
#define FCSGENOME_WORKERS_HTCWORKER_H

#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class HTCWorker : public Worker {
 public:
  HTCWorker(std::string ref_path,
      std::string intv_path,
      std::string input_path,
      std::string output_path,
      int contig,
      bool &flag_f);
};
} // namespace fcsgenome
#endif
