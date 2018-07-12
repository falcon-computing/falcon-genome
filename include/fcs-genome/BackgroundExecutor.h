#ifndef FCS_GENOME_BACKGROUNDEXECUTOR_H
#define FCS_GENOME_BACKGROUNDEXECUTOR_H
#include <boost/smart_ptr.hpp>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class BackgroundExecutor : public Executor {
 public:
  BackgroundExecutor(std::string job_name, Worker_ptr worker);
  ~BackgroundExecutor();
 
 private:
  int child_pid_;
  std::string job_script_;
};
} // namespace fcsgenome
#endif
