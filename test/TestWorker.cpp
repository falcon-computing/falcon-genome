#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <gtest/gtest.h>

#include "fcs-genome/BackgroundExecutor.h"
#include "fcs-genome/config.h"
#include "fcs-genome/common.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/workers/BQSRWorker.h"

namespace fcs = fcsgenome;
namespace fs = boost::filesystem;

class TestWorker : public ::testing::Test {
  ;
};

void touch(std::string path) {
  DLOG(INFO) << "touching " << path;
  fs::ofstream f(path);
  f.close();
}

TEST_F(TestWorker, TestBQSRWorker_check) {
  std::string temp_dir = "/tmp/fcs-genome-test-" +
    std::to_string((long long)fcs::getTid());
  fcs::create_dir(temp_dir);

  // common input
  std::string ref    = temp_dir + "/" + "ref.fasta";
  std::string intv   = temp_dir + "/" + "intv.list";
  std::string input  = temp_dir + "/" + "input.bam";
  std::string output = temp_dir + "/" + "output.bam";
  std::vector<std::string> known;
  known.push_back(temp_dir + "/" + "known1.vcf");
  bool flag = true;

  std::vector<std::string> empty;
  std::string emptyString;

  fcs::BQSRWorker worker(ref, known, intv, input, output,
      empty, emptyString, 0, flag, false);

#define CHECK_EXCEPTION try { \
    worker.check(); \
    FAIL() << "Expecting exception to be thrown"; \
  } \
  catch (...) { \
    ; \
  }

#define CHECK_NOEXCEPTION try { \
    worker.check(); \
  } catch (...) { \
    FAIL() << "Not expecting exception to be thrown"; \
  }

  // first check will thrown fileNotFound
  CHECK_EXCEPTION;

  // create all files
  touch(ref);
  touch(intv);
  touch(input);
  touch(output);
  touch(known[0]);
  touch(known[0]+".idx");

  // no exception should be thrown
  CHECK_NOEXCEPTION;

  // test index
  boost::this_thread::sleep_for(boost::chrono::seconds(1));

  touch(known[0]); // now known will be older than idx
  CHECK_NOEXCEPTION;
  // no exception will be thrown since the tool will try
  // to update file

  // test no index exception
  known.push_back(temp_dir + "/" + "known2.vcf.gz");

  {
  fcs::BQSRWorker worker(ref, known, intv, input, output,
      empty, empty, 0, flag, false);

  touch(known[1]);
  CHECK_EXCEPTION;
  touch(known[1] + ".idx");
  CHECK_EXCEPTION;
  touch(known[1] + ".tbi");
  CHECK_NOEXCEPTION;
  }

  // test wrong known site exception
  known.push_back(temp_dir + "/" + "known3.vcf1");
  {
  fcs::BQSRWorker worker(ref, known, intv, input, output,
      empty, empty, 0, flag, false);
  CHECK_EXCEPTION;
  }

  fcs::remove_path(temp_dir);
}
