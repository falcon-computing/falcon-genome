#include <glog/logging.h>
#include <string>
#include <vector>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers/HTCWorker.h"

namespace fcsgenome {

HTCWorker::HTCWorker(std::string ref_path,
      std::string intv_path,
      std::string input_path,
      std::string output_path,
      int  contig,
      bool &flag_f): Worker(conf_gatk_ncontigs, 1)
{
  // check input/output files
  ref_path    = check_input(ref_path);
  intv_path   = check_input(intv_path);
  input_path  = check_input(input_path);
  output_path = check_output(output_path, flag_f);

  // create cmd
  std::stringstream cmd;
  cmd << conf_java_call << " "
      << "-Xmx" << conf_htc_memory << "g "
      << "-jar " << conf_gatk_path << " "
      << "-T HaplotypeCaller "
      << "-R " << ref_path << " "
      << "-I " << input_path << " "
      << "--emitRefConfidence GVCF "
      << "--variant_index_type LINEAR "
      << "--variant_index_parameter 128000 "
      << "-L " << intv_path << " "
      << "-nct " << conf_htc_nct << " "
      << "-o " << output_path << " ";

  cmd_ = cmd.str();
  DLOG(INFO) << cmd_;
}

} // namespace fcsgenome
