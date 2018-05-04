#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <string>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/workers.h"

namespace fcsgenome {

int gatk_main(int argc, char** argv,
    boost::program_options::options_description &opt_desc) 
{

  std::stringstream cmd;
  std::stringstream cmd_log;
  LOG(WARNING) << "Running original GATK without Falcon acceleration";

  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx8g "
      << "-jar " << get_config<std::string>("gatk_path") << " ";
  for (int i = 1; i < argc; i++) {
    cmd << argv[i] << " ";
    cmd_log << argv[i] << " ";
  }
  LOG(INFO) << "fcs-genome gatk " << cmd_log.str();
  DLOG(INFO) << cmd.str();
  return system(cmd.str().c_str());
}
} // namespace fcsgenome
