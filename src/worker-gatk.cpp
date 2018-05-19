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
  LOG(WARNING) << "Running original GATK without Falcon acceleration";

  cmd << get_config<std::string>("java_path") << " "
      << "-Xmx8g "
      << "-jar " << get_config<std::string>("gatk_path") << " ";
  for (int i = 1; i < argc; i++) {
    if ( boost::starts_with(argv[i],"-") || boost::starts_with(argv[i],"--"))
      cmd << argv[i] << " ";
    else
      cmd << "'" << argv[i] << "'" << " ";
  }
  DLOG(INFO) << cmd.str();
  return system(cmd.str().c_str());
}
} // namespace fcsgenome
