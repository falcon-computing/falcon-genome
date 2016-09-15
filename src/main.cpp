#include <glog/logging.h>
#include <string>
#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/workers.h"

using namespace fcsgenome;

int print_help() {
  return 0;	
}

int main(int argc, char** argv) {

  // Initialize Google Log
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr=1;

  if (argc < 2) {
    print_help();
    return 1;
  }

  try {
    // Load configurations
    init(argv[0]);

    std::string cmd(argv[1]);

    if (cmd == "align" | cmd == "al") {
      return align_main(argc-1, &argv[1]);
    }
    else if (cmd == "markdup" | cmd == "markDup" | cmd == "md") {
      return markdup_main(argc-1, &argv[1]);
    }
    else {
      print_help(); 
      return 1;
    }
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
}
