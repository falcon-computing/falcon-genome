#include <string>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/BlazeWorker.h"

namespace fcsgenome {

BlazeWorker::BlazeWorker(std::string nam_path, std::string conf_path): 
      Worker(1, 1),
      nam_path_(nam_path),
      conf_path_(conf_path) 
{
  ;  
}

void BlazeWorker::check() {
  nam_path_  = check_input(nam_path_);
  conf_path_ = check_input(conf_path_);
}

void BlazeWorker::setup() {
  std::stringstream cmd;
  cmd << nam_path_ << " " << conf_path_;

  cmd_ = cmd.str();
}

} // namespace fcsgenome
