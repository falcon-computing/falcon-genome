#include <algorithm>
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>
#include <string>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"

// use flexlm
#ifdef USELICENSE
#include "license.h"
#endif

#ifdef USELICENSE
void licence_check_out() {
  // initialize for licensing. call once
  fc_license_init();

  // get a feature
  int status = 0;
  while (-4 == (status = fc_license_checkout(FALCON_DNA, 0))) {
    LOG(INFO) << "Reached maximum allowed instances on this machine, "
      << "wait for 30 seconds. Please press CTRL+C to exit.";
    boost::this_thread::sleep_for(boost::chrono::seconds(30));
  }
  if (status) {
    throw fcsgenome::internalError(std::to_string((long long)status));
  }
}

void licence_check_in() {
  fc_license_checkin(FALCON_DNA);

  // cleanup for licensing. call once
  fc_license_cleanup();
}
#endif

#define print_cmd_col(str1, str2) std::cout \
    << "  " << std::left << std::setw(16) << str1 \
    << std::left << std::setw(54) << str2 \
    << std::endl;

int print_help() {
  std::cout << "Falcon Genome Analysis Toolkit " << VERSION << std::endl;
  std::cout << "Usage: fcs-genome [command] <options>" << std::endl;
  std::cout << std::endl;
  std::cout << "Commands: " << std::endl;

  print_cmd_col("align", "align pair-end FASTQ files into a sorted,");
  print_cmd_col("     ", "duplicates-marked BAM file");
  print_cmd_col("markdup", "mark duplicates in an aligned BAM file");
  print_cmd_col("bqsr", "base recalibration with GATK BaseRecalibrator");
  print_cmd_col("    ", "and GATK PrintReads");
  print_cmd_col("baserecal", "equivalent to GATK BaseRecalibrator");
  print_cmd_col("printreads", "equivalent to GATK PrintReads");
  print_cmd_col("htc", "variant calling with GATK HaplotypeCaller");
  print_cmd_col("indel", "indel realignment with GATK IndelRealigner");
  print_cmd_col("joint", "joint variant calling with GATK GenotypeGVCFs");
  print_cmd_col("ug", "variant calling with GATK UnifiedGenotyper");
  print_cmd_col("gatk", "call GATK routines");

  print_cmd_col("doc", "depthOfCoverage"); //added by pingwen
  print_cmd_col("json", "jobs workflow written in file of json format"); // added by pingwen

  return 0;	
}

void sigint_handler(int s){

  boost::lock_guard<fcsgenome::Executor> guard(*fcsgenome::g_executor);
  LOG(INFO) << "Caught interrupt, cleaning up...";
  if (fcsgenome::g_executor) {
    delete fcsgenome::g_executor;
  }
  
  exit(0); 
}

namespace fcsgenome {
  namespace po = boost::program_options;
  int align_main(int argc, char** argv, po::options_description &opt_desc);
  int bqsr_main(int argc, char** argv, po::options_description &opt_desc);
  int baserecal_main(int argc, char** argv, po::options_description &opt_desc);
  int concat_main(int argc, char** argv, po::options_description &opt_desc);
  int htc_main(int argc, char** argv, po::options_description &opt_desc);
  int ir_main(int argc, char** argv, po::options_description &opt_desc);
  int joint_main(int argc, char** argv, po::options_description &opt_desc);
  int markdup_main(int argc, char** argv, po::options_description &opt_desc);
  int pr_main(int argc, char** argv, po::options_description &opt_desc);
  int ug_main(int argc, char** argv, po::options_description &opt_desc);
  int gatk_main(int argc, char** argv, po::options_description &opt_desc);
  int hist_main(int argc, char** argv, po::options_description &opt_desc);

  int doc_main(int argc, char** argv, po::options_description &opt_desc);
  int json_main(int argc, char** argv, po::options_description &opt_desc);
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

  namespace po = boost::program_options;
  po::options_description opt_desc;

  opt_desc.add_options() 
    ("help,h", "print help messages")
    ("force,f", "overwrite output files if they exist")
    ("extra-options,O", po::value<std::vector<std::string> >(),
     "extra options for the command");
    //("checkpoint", "save the output of the command");
    //("schedule,a", "schedule the command rather than executing it");

  std::string cmd(argv[1]);
  // transform all cmd to lower-case
  std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

#ifdef USELICENSE
  try {
    // check license
    licence_check_out();
  }
  catch (std::runtime_error &e) {
    LOG(ERROR) << "Cannot connect to the license server: " << e.what();
    LOG(ERROR) << "Please contact support@falcon-computing.com for details.";
    return -1;
  }
#endif

  signal(SIGINT, sigint_handler);

  int ret = 0;
  try {
    // load configurations
    init(argv, argc);

    // run command
    if (cmd == "align" | cmd == "al") {
      align_main(argc-1, &argv[1], opt_desc);
    }
    else if (cmd == "markdup" | cmd == "md") {
      markdup_main(argc-1, &argv[1], opt_desc);
    }
    else if (cmd == "baserecal") {
      baserecal_main(argc-1, &argv[1], opt_desc);
    }
    else if (cmd == "printreads" | cmd == "pr") {
      pr_main(argc-1, &argv[1], opt_desc);
    }
    else if (cmd == "bqsr") {
      bqsr_main(argc-1, &argv[1], opt_desc);
    }
    else if (cmd == "indel" | cmd == "ir") {
      ir_main(argc-1, &argv[1], opt_desc);
    }
    else if (cmd == "joint") {
      joint_main(argc-1, &argv[1], opt_desc);
    }
    else if (cmd == "unifiedgeno" | cmd == "ug") {
      ug_main(argc-1, &argv[1], opt_desc);
    }
    else if (cmd == "htc") {
      htc_main(argc-1, &argv[1], opt_desc);
    }

    else if (cmd == "doc") {   // inserted by pingwen
      doc_main(argc-1, &argv[1], opt_desc);
    }

    else if (cmd == "json") {
    	json_main(argc-1, &argv[1], opt_desc);
    }

    else if (cmd == "concat") {
      concat_main(argc-1, &argv[1], opt_desc);
    }
    else if (cmd == "gatk") {
      gatk_main(argc-1, &argv[1], opt_desc);
    }
    else {
      print_help(); 
      throw silentExit();
    }

#ifdef NDEBUG
    // delete temp dir
    remove_path(conf_temp_dir);
#endif
  }
  catch (helpRequest &e) { 
    std::cerr << "'fcs-genome " << cmd << "' options:" << std::endl;
    std::cerr << opt_desc << std::endl; 

    // delete temp dir
    remove_path(conf_temp_dir);

    ret = 0;
  }
  catch (invalidParam &e) { 
    LOG(ERROR) << "Missing argument '--" << e.what() << "'";
    std::cerr << "'fcs-genome " << cmd << "' options:" << std::endl;
    std::cerr << opt_desc << std::endl; 

    ret = 1;
  }
  catch (boost::program_options::error &e) { 
    LOG(ERROR) << "Failed to parse arguments, " << e.what();
    std::cerr << "'fcs-genome " << cmd << "' options:" << std::endl;
    std::cerr << opt_desc << std::endl; 

    // delete temp dir
    remove_path(conf_temp_dir);

    ret = 2;
  }
  catch (fileNotFound &e) { 
    LOG(ERROR) << e.what();
    ret = 3;
  }
  catch (silentExit &e) {
    LOG(INFO) << "Exiting program";

    // delete temp dir
    remove_path(conf_temp_dir);

    ret = 1;
  } 
  catch (failedCommand &e) {
    LOG(ERROR) << e.what();
    ret = 4;
  }
  catch (std::runtime_error &e) {
    LOG(ERROR) << "Encountered an error: " << e.what();
    LOG(ERROR) << "Please contact support@falcon-computing.com for details.";
    ret = -1;
  }

#ifdef USELICENSE
  // release license
  licence_check_in();
#endif

  return ret;
}
