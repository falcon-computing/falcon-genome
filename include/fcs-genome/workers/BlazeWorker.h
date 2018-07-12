#ifndef FCSGENOME_WORKERS_BLAZEWORKER_H
#define FCSGENOME_WORKERS_BLAZEWORKER_H

#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class BlazeWorker : public Worker {
 public:
  BlazeWorker(std::string nam_path, std::string conf_path);
  void check();
  void setup();

 private:
  std::string nam_path_;
  std::string conf_path_;
};
} // namespace fcsgenome
#endif
