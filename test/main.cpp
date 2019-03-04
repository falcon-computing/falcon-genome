#include <glog/logging.h>
#include <gtest/gtest.h>

#include "fcs-genome/config.h"

class TestConfig;

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = true;
  ::testing::InitGoogleTest(&argc, argv);

  // initialize configurations
  fcsgenome::init(argv, argc); 

  // run all tests
  return RUN_ALL_TESTS();
}
