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
#include "falcon-lic/license.h"
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
  print_cmd_col("mutect2", "(Experimental) somatic variant calling with GATK Mutect2");
  print_cmd_col("indel", "indel realignment with GATK IndelRealigner");
  print_cmd_col("joint", "joint variant calling with GATK GenotypeGVCFs");
  print_cmd_col("ug", "variant calling with GATK UnifiedGenotyper");
  print_cmd_col("gatk", "call GATK routines");

  return 0;	
}

void sigint_handler(int s){

  boost::lock_guard<fcsgenome::Executor> guard(*fcsgenome::g_executor);
  LOG(INFO) << "Caught interrupt, cleaning up...";
  if (fcsgenome::g_executor) {
    delete fcsgenome::g_executor;
  }
#ifndef NDEBUG
  fcsgenome::remove_path(fcsgenome::conf_temp_dir);
#endif
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
  int mutect2_main(int argc, char** argv, po::options_description &opt_desc);
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
  namespace fc   = falconlic;
#ifdef DEPLOY_aws
  fc::enable_aws();
#endif
#ifdef DEPLOY_hwc
  fc::enable_hwc();
#endif
  fc::enable_flexlm();

  namespace fclm = falconlic::flexlm;
  fclm::add_feature(fclm::FALCON_DNA);
  int licret = fc::license_verify();
  if (licret != fc::SUCCESS) {
    LOG(ERROR) << "Cannot authorize software usage: " << licret;
    LOG(ERROR) << "Please contact support@falcon-computing.com for details.";
    return licret;
  }
#endif

  signal(SIGINT, sigint_handler);

  int ret = 0;
  try {
    // load configurations
    init(argv, argc);

    std::stringstream cmd_log;
    for (int i = 0; i < argc; i++) {
      cmd_log << argv[i] << " ";
    }
    LOG(INFO) << "Arguments: " << cmd_log.str();   
  
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
    else if (cmd == "concat") {
      concat_main(argc-1, &argv[1], opt_desc);
    }
    else if (cmd == "gatk") {
      gatk_main(argc-1, &argv[1], opt_desc);
    }
    else if (cmd == "mutect2") {
      mutect2_main(argc-1, &argv[1], opt_desc);
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
    std::cerr << "'fcs-genome " << cmd;
    std::cerr << "' options:" << std::endl;
    std::cerr << opt_desc << std::endl; 

    // delete temp dir
    remove_path(conf_temp_dir);

    ret = 0;
  }
  catch (invalidParam &e) { 
    LOG(ERROR) << "Failed to parse arguments: " 
               << "invalid option " << e.what();
    std::cerr << "'fcs-genome " << cmd;
    std::cerr << "' options:" << std::endl;
    std::cerr << opt_desc << std::endl; 

    ret = 1;
  }
  catch (pathEmpty &e) {
    LOG(ERROR) << "Failed to parse arguments: " 
               << "option " << e.what() << " cannot be empty";
    std::cerr << "'fcs-genome " << cmd;
    std::cerr << "' options:" << std::endl;
    std::cerr << opt_desc << std::endl; 

    ret = 1;
  }
  catch (boost::program_options::error &e) { 
    LOG(ERROR) << "Failed to parse arguments: " 
               << e.what();
    std::cerr << "'fcs-genome " << cmd;
    std::cerr << "' options:" << std::endl;
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
  fc::license_clean();
#endif
  return ret;
}
