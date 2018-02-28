#include <glog/logging.h>
#include <gtest/gtest.h>

class TestConfig;

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);

  // run all tests
  return RUN_ALL_TESTS();
}
