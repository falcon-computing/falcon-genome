#ifndef FCSGENOME_WORKER_H
#define FCSGENOME_WORKER_H

#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <iostream>
#include <map>
#include <regex>
#include <string>

#include "config.h"
#include "common.h"
#include "Executor.h"

// for testing purpose
#ifndef TEST_FRIENDS_LIST
#define TEST_FRIENDS_LIST
#endif

namespace fcsgenome {

// Base class of a task worker
class Worker {
  friend class Executor;
  TEST_FRIENDS_LIST
 public:
  Worker(int num_proc = 1,
         int num_t = 1,
         std::vector<std::string> extra_opts = std::vector<std::string>()):
    num_process_(num_proc),
    num_thread_(num_t),
    extra_opts_()
  {  
     for (int i =0; i < extra_opts.size(); i++) {
       std::string type;
       std::string value;    
       //std::regex r("(\\s+");
       //std::vector<std::string> result{
       //  std::sregex_token_iterator iter(extra_opts[i].begin(), extra_opts[i].end(), r ), end
       //};
       std::vector<std::string> type_and_value;
       boost::split(type_and_value, extra_opts[i], boost::is_any_of(" "));
       for (int i=0; i < type_and_value.size(); i++) {
         if (boost::starts_with(type_and_value[i], "--") || boost::starts_with(type_and_value[i], "-")) {
           type=type_and_value[i];
           if (boost::starts_with(type_and_value[i+1], "--") || boost::starts_with(type_and_value[i+1], "-")) {
             value=" ";
           }
           else {
             value=type_and_value[i+1];
             i++;
           }
         }
         extra_opts_.insert(std::map<std::string, std::string>::value_type(type,value));
       //for (int i=0; i < type_and_value.size(); i++) {
         LOG(INFO) << "Parsing one extra option: Key=" << type << ", Value=" << value;
	 //std::cout << type_and_value[i] << std::endl;
       //} 
     }
     // initialize extra_opts_ table from extra_opts
    
  }
}
  virtual void check() {}
  virtual void setup() {}
  virtual void teardown() {}

  std::string getCommand() { return cmd_; }

 protected:
  std::string cmd_;
  std::string log_fname_;
  std::map<std::string, std::string> extra_opts_;

 private:
  int num_process_;   // num_processes per task
  int num_thread_;    // num_thread per task process
};

typedef boost::shared_ptr<Worker> Worker_ptr;
} // namespace fcsgenome
#endif
