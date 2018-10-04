#include <boost/filesystem.hpp>

#include<bits/stdc++.h> 
#include <iostream>
#include <fstream>
#include <string>
#include <gtest/gtest.h>

#include "fcs-genome/BackgroundExecutor.h"
#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Worker.h"
#include "fcs-genome/workers/BlazeWorker.h"

namespace fcs = fcsgenome;

class TestExecutor : public ::testing::Test {
  ; 
};

TEST_F(TestExecutor, TestBackgroundExecutor) {

  bool check_done = false;
  bool setup_done = false;

  // write a file to temp folder
  std::stringstream fname;
  fname << "/tmp/TestExecutor." << fcs::getTid() << ".txt";
  std::fstream fs(fname.str().c_str(), std::ios::out);
  fs.close();

  ASSERT_TRUE(boost::filesystem::exists(fname.str()));

  class RemoveWorker : public fcs::Worker {
    public:
      RemoveWorker(std::string fname, 
          bool* _check, bool* _setup): 
            Worker(1, 1), 
            check_done_(_check),
            setup_done_(_setup)
      {
        cmd_ = "rm " + fname;
      }
      void check() { *check_done_ = true; }
      void setup() { *setup_done_ = true; }

    private:
      bool* check_done_;
      bool* setup_done_;
  };

  class SleepWorker : public fcs::Worker {
    public:
      SleepWorker(std::string fname): Worker(1, 1) {
        cmd_ = "sleep 0.1; touch " + fname;  
      }
  };
  
  { // check if command is executed
    fcs::Worker_ptr worker(new RemoveWorker(
        fname.str(), &check_done, &setup_done));

    fcs::BackgroundExecutor e("remove", worker);

    ASSERT_TRUE(check_done);
    ASSERT_TRUE(setup_done);

    // check if file is removed after a while
    boost::this_thread::sleep_for(boost::chrono::milliseconds(10)); 

    ASSERT_FALSE(boost::filesystem::exists(fname.str()));
  }

  fcs::Worker_ptr worker(new SleepWorker(
        fname.str()));

  fcs::BackgroundExecutor* e = new fcs::BackgroundExecutor("sleep",  worker);

  // check if file is after a while
  boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
  ASSERT_FALSE(boost::filesystem::exists(fname.str()));

  delete e;
  boost::this_thread::sleep_for(boost::chrono::milliseconds(100)); 

  // shouldn't create this file since the executor is killed
  ASSERT_FALSE(boost::filesystem::exists(fname.str()));
}
