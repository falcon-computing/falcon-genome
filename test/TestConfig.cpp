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

  fcs::init(NULL, 0);
  ASSERT_TRUE(fcs::config_vtable.count("temp_dir"));
  ASSERT_TRUE(fcs::config_vtable.count("log_dir"));
  ASSERT_TRUE(fcs::config_vtable.count("gatk.nprocs"));
}

TEST_F(TestConfig, GATKNprocs) {

  fcs::init(NULL, 0); 

  int ncontigs = 32;
  int nprocs = fcs::get_config<int>("gatk.nprocs");
  int nct    = fcs::get_config<int>("gatk.nct");
  int memory = fcs::get_config<int>("gatk.memory");

  // nprocs <= ncontigs
  ASSERT_LE(nprocs, ncontigs);

  ASSERT_EQ(nprocs, fcs::get_config<int>("gatk.bqsr.nprocs"));
  ASSERT_EQ(nprocs, fcs::get_config<int>("gatk.pr.nprocs"));
  ASSERT_EQ(nprocs, fcs::get_config<int>("gatk.htc.nprocs"));
  ASSERT_EQ(nprocs, fcs::get_config<int>("gatk.indel.nprocs"));
  ASSERT_EQ(nprocs, fcs::get_config<int>("gatk.ug.nprocs"));

  ASSERT_EQ(nct, fcs::get_config<int>("gatk.bqsr.nct"));
  ASSERT_EQ(nct, fcs::get_config<int>("gatk.pr.nct"));
  ASSERT_EQ(nct, fcs::get_config<int>("gatk.htc.nct"));
  ASSERT_EQ(nct, fcs::get_config<int>("gatk.ug.nt"));

  ASSERT_EQ(memory, fcs::get_config<int>("gatk.bqsr.memory"));
  ASSERT_EQ(memory, fcs::get_config<int>("gatk.pr.memory"));
  ASSERT_EQ(memory, fcs::get_config<int>("gatk.htc.memory"));
  ASSERT_EQ(memory, fcs::get_config<int>("gatk.indel.memory"));
  ASSERT_EQ(memory, fcs::get_config<int>("gatk.ug.memory"));
}

TEST_F(TestConfig, AutoTuneNprocsMemory) {
  // TEST 1: 4, 16
  int nprocs = 32;
  int memory = 4;

  fcs::calc_gatk_default_config(nprocs, memory, 32, 128);
  ASSERT_EQ(32, nprocs);
  ASSERT_EQ(4, memory);

  fcs::calc_gatk_default_config(nprocs, memory, 4, 32);
  ASSERT_EQ(4, nprocs);
  ASSERT_EQ(8, memory);

  fcs::calc_gatk_default_config(nprocs, memory, 16, 120);
  ASSERT_EQ(16, nprocs);
  ASSERT_EQ(6, memory);

  fcs::calc_gatk_default_config(nprocs, memory, 28, 112);
  ASSERT_EQ(16, nprocs);
  ASSERT_EQ(6, memory);

  fcs::calc_gatk_default_config(nprocs, memory, 32, 224);
  ASSERT_EQ(32, nprocs);
  ASSERT_EQ(6, memory);

  fcs::calc_gatk_default_config(nprocs, memory, 32, 192);
  ASSERT_EQ(32, nprocs);
  ASSERT_EQ(6, memory);

  fcs::calc_gatk_default_config(nprocs, memory, 48, 252);
  ASSERT_EQ(32, nprocs);
  ASSERT_EQ(8, memory);
}

TEST_F(TestConfig, CheckNprocsAndMemory) {

  // set values through env
  FILE* fout = fopen("fcs-genome.conf", "w+");
  ASSERT_EQ(NULL, !fout) << "Cannot write to fcs-genome.conf";

  fprintf(fout, "gatk.bqsr.nprocs = 16\n");
  fprintf(fout, "gatk.bqsr.memory = 2\n");
  fprintf(fout, "gatk.pr.nprocs = 16\n");
  fprintf(fout, "gatk.pr.memory = 4\n");
  fprintf(fout, "gatk.htc.nprocs = 16\n");
  fprintf(fout, "gatk.htc.memory = 8\n");
  fclose(fout);

  fcs::init(NULL, 0); 

  std::remove("fcs-genome.conf");

  ASSERT_EQ(0, fcs::check_nprocs_config("bqsr", 16));
  ASSERT_EQ(1, fcs::check_nprocs_config("bqsr", 8));
  ASSERT_EQ(1, fcs::check_memory_config("bqsr", 32));

  ASSERT_EQ(0, fcs::check_memory_config("pr", 64));
  ASSERT_EQ(1, fcs::check_memory_config("pr", 60));

  ASSERT_EQ(0, fcs::check_memory_config("htc", 128));
  ASSERT_EQ(0, fcs::check_memory_config("htc", 124));
  ASSERT_EQ(1, fcs::check_memory_config("htc", 120));
}
