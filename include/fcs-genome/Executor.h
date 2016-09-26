#ifndef FCSGENOME_EXECUTOR_H
#define FCSGENOME_EXECUTOR_H

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <string>
#include <queue>

#include "fcs-genome/config.h"
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class Executor;
class Stage;

typedef boost::shared_ptr<Stage> Stage_ptr;

class Stage
: public boost::basic_lockable_adapter<boost::mutex>
{
 public:
  Stage(Executor* executor);

  void add(Worker_ptr worker);
  void run();

 private:
  void runTask(int idx);

  Executor*                executor_;
  std::vector<Worker_ptr>  tasks_;
  std::vector<std::string> logs_;
  std::map<int, int>       status_;
};

class Executor {
 public:
  Executor(std::string job_name,
      int num_executors = 1); 

  virtual int execute(Worker_ptr worker, std::string log);
  virtual void run();

  template <typename CompletionHandler>
  void post(CompletionHandler handler) {
    ios_->post(handler);
  }
  std::string log() { return log_fname_; }
  std::string job_name() { return job_name_; }

  void addTask(Worker_ptr worker, bool wait_for_prev = false);

 private:
  int                   num_executors_;
  std::string           job_name_;
  std::queue<Stage_ptr> job_stages_; 
  std::string           log_fname_;

  boost::shared_ptr<boost::asio::io_service> ios_;
  boost::shared_ptr<boost::asio::io_service::work> ios_work_;
  boost::thread_group executors_;
};
} // namespace fcsgenome
#endif
