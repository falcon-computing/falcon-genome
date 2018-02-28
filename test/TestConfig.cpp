#include <iostream>
#include <fstream>
#include <gtest/gtest.h>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"

namespace fcs = fcsgenome;
class TestConfig : public ::testing::Test {
 protected:
  virtual void SetUp() {
    // clear all config options 
    namespace po = boost::program_options;
    fcs::config_vtable.clear();
    po::notify(fcs::config_vtable);
  }
};

TEST_F(TestConfig, DefaultValues) {

  std::string g_conf_fname = fcs::conf_root_dir + "/fcs-genome.conf";
  std::string l_conf_fname = "fcs-genome.conf";

  // ensure the configuration files does not exist
  std::ifstream l_conf_file{l_conf_fname.c_str()};
  std::ifstream g_conf_file{g_conf_fname.c_str()};
  if (l_conf_file || g_conf_file) {
    DLOG(WARNING) << "The following tests will be skipped because " 
                  << "configuration files are loaded";
    return;
  }

  fcs::init_config();
  ASSERT_TRUE(fcs::config_vtable.count("temp_dir"));
  ASSERT_TRUE(fcs::config_vtable.count("log_dir"));
  ASSERT_TRUE(fcs::config_vtable.count("gatk.nprocs"));
}
