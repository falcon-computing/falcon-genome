#ifndef FCSGENOME_WORKER_H
#define FCSGENOME_WORKER_H

#include <boost/shared_ptr.hpp>
#include <string>
#include "config.h"
#include "common.h"

namespace fcsgenome {

// Base class of worker
class Worker {
 public:
  Worker(int num_proc = 1,
         int num_t = INT_MAX): 
    num_process_(num_proc),
    num_thread_(num_t) {}

  virtual void check() {}
  virtual void setup() {}
  virtual void teardown() {}

  std::string getCommand() { return cmd_; }

 protected:
  std::string cmd_;
  std::string log_fname_;

 private:
  int num_process_;
  int num_thread_;
};

typedef boost::shared_ptr<Worker> Worker_ptr;
} // namespace fcsgenome
#endif