#include <algorithm>
#include <glog/logging.h>
#include <string>
#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers.h"

// use flexlm
#ifdef USELICENSE
#include "license.h"
#endif

#ifdef USELICENSE
void licence_check_out() {

  Feature feature = FALCON_RT;

  // initialize for licensing. call once
  fc_license_init();

  // get a feature
  fc_license_checkout(feature, 1);

  printf("\n");
}

void licence_check_in() {

  Feature feature = FALCON_RT;

  fc_license_checkin(feature);

  // cleanup for licensing. call once
  fc_license_cleanup();
}
#endif

#define print_cmd_col(str1, str2) std::cout \
    << "  " << std::left << std::setw(16) << str1 \
    << std::left << std::setw(54) << str2 \
    << std::endl;

int print_help() {
  std::cout << "Falcon Genome Analysis Toolkit v1.0.3" << std::endl;
  std::cout << "Usage: fcs-genome [command] <options>" << std::endl;
  std::cout << std::endl;
  std::cout << "Commands: " << std::endl;

  print_cmd_col("align", "align pair-end FASTQ files into a sorted,");
  print_cmd_col("          ", "duplicates-marked BAM file");
  print_cmd_col("bqsr", "base recalibration with GATK BaseRecalibrator");
  print_cmd_col("    ", "and GATK PrintReads");
  print_cmd_col("baserecal", "equivalent to GATK BaseRecalibrator");
  print_cmd_col("printreads", "equivalent to GATK PrintReads");
  print_cmd_col("htc", "variant calling with GATK HaplotypeCaller");
  print_cmd_col("gatk", "call GATK routines");

  return 0;	
}

namespace fcsgenome {
  namespace po = boost::program_options;
  int align_main(int argc, char** argv, po::options_description &opt_desc);
  int bqsr_main(int argc, char** argv, po::options_description &opt_desc);
  int baserecal_main(int argc, char** argv, po::options_description &opt_desc);
  int pr_main(int argc, char** argv, po::options_description &opt_desc);
  int concat_main(int argc, char** argv, po::options_description &opt_desc);
  int htc_main(int argc, char** argv, po::options_description &opt_desc);
  int markdup_main(int argc, char** argv, po::options_description &opt_desc);
}

int main(int argc, char** argv) {

  using namespace fcsgenome;

  // Initialize Google Log
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr=1;

  if (argc < 2) {
    print_help();
    return 1;
  }

#ifdef USELICENSE
  // check license
  licence_check_out();
#endif
 
  namespace po = boost::program_options;
  po::options_description opt_desc;

  std::string cmd(argv[1]);
  // transform all cmd to lower-case
  std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

  try {
    // Load configurations
    init(argv[0]);

    if (cmd == "align" | cmd == "al") {
      return align_main(argc-1, &argv[1], opt_desc);
    }
    else if (cmd == "markdup" | cmd == "md") {
      return markdup_main(argc-1, &argv[1], opt_desc);
    }
    else if (cmd == "baserecal") {
      return baserecal_main(argc-1, &argv[1], opt_desc);
    }
    else if (cmd == "printreads" | cmd == "pr") {
      return pr_main(argc-1, &argv[1], opt_desc);
    }
    else if (cmd == "bqsr") {
      return bqsr_main(argc-1, &argv[1], opt_desc);
    }
    else if (cmd == "htc") {
      return htc_main(argc-1, &argv[1], opt_desc);
    }
    else if (cmd == "concat") {
      return concat_main(argc-1, &argv[1], opt_desc);
    }
    else {
      print_help(); 
      return 1;
    }
  }
  catch (helpRequest &e) { 
    std::cerr << "'fcs-genome " << cmd << "' options:" << std::endl;
    std::cerr << opt_desc << std::endl; 
    return 0;
  }
  catch (invalidParam &e) { 
    LOG(ERROR) << "Missing argument '" << e.what() << "'";
    std::cerr << "'fcs-genome " << cmd << "' options:" << std::endl;
    std::cerr << opt_desc << std::endl; 
    return 1;
  }
  catch (boost::program_options::error &e) { 
    LOG(ERROR) << "Failed to parse arguments, " << e.what();
    std::cerr << "'fcs-genome " << cmd << "' options:" << std::endl;
    std::cerr << opt_desc << std::endl; 
    return 1; 
  }
  catch (fileNotFound &e) { 
    LOG(ERROR) << e.what();
    return 1;
  }
  catch (silentExit &e) {
    LOG(INFO) << "Exiting program.";
    return 1; 
  } 
  catch (failedCommand &e) {
    LOG(ERROR) << e.what();
    return 1; 
  }
  catch (std::runtime_error &e) {
    LOG(ERROR) << "Encountered an internal error: " << e.what();
    LOG(ERROR) << "Please contact support@falcon-computing.com for details.";
    return -1;
  }
#ifdef USELICENSE
  // release license
  licence_check_in();
#endif

  return 0;
}
