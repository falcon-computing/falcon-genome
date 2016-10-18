#include <algorithm>
#include <boost/smart_ptr.hpp>
#include <boost/thread/future.hpp>
#include <iostream>
#include <fstream>
#include <string>


#include "fcs-genome/common.h"
#include "fcs-genome/Executor.h"

namespace fcsgenome {

Stage::Stage(Executor* executor): executor_(executor) {;}

void Stage::add(Worker_ptr worker) {
  logs_.push_back(get_log_name(executor_->job_name(), logs_.size()));
  tasks_.push_back(worker);
}

void Stage::run() {
  typedef boost::packaged_task<void> task_t;
  std::vector<boost::unique_future<void> > pending_tasks_;

  // post all tasks
  for (int i = 0; i < tasks_.size(); i++) {
    tasks_[i]->check();

    boost::shared_ptr<task_t> task = boost::make_shared<task_t>(
        boost::bind(&Stage::runTask, this, i));

    boost::unique_future<void> fut = task->get_future();
    pending_tasks_.push_back(std::move(fut));

    executor_->post(boost::bind(&task_t::operator(), task)); 
  }
  // wait for tasks to finish
  boost::wait_for_all(pending_tasks_.begin(), pending_tasks_.end()); 

  std::ofstream fout(executor_->log(), std::ios::out|std::ios::app);
  for (int i = 0; i < logs_.size(); i++) {
    std::ifstream fin(logs_[i], std::ios::in);   
    if (fin) {
      fout << fin.rdbuf();
    }
    fin.close();

    // remove task log
    remove_path(logs_[i]);
  }
  fout.flush();
  fout.close();

  // check results
  if (!status_.empty()) {
    throw failedCommand(executor_->job_name() +
        " failed, please check log: " + executor_->log() +
        " for details");
  }
}

void Stage::runTask(int idx) {
  int ret = executor_->execute(tasks_[idx], logs_[idx]);
  if (ret) {
    DLOG(ERROR) << "Task " << idx << " in stage"
                << " failed with error code " << ret;
    boost::lock_guard<Stage> guard(*this);
    status_[idx] = ret;
  }
}

Executor::Executor(std::string job_name,
    int num_executors): job_name_(job_name), num_executors_(num_executors)
{
  // create thread group
  boost::shared_ptr<boost::asio::io_service> ios(new boost::asio::io_service);
  ios_ = ios;

  // Start io service processing loop
  boost::shared_ptr<boost::asio::io_service::work> work(
      new boost::asio::io_service::work(*ios));
  ios_work_ = work;

  for (int t = 0; t < num_executors; t++) {
    executors_.create_thread(
        boost::bind(&boost::asio::io_service::run, ios.get()));
  }
  DLOG(INFO) << "Started " << executors_.size() << " workers.";
}

Executor::~Executor() {
  ios_work_.reset();
  executors_.join_all();
}

void Executor::addTask(Worker_ptr worker, bool wait_for_prev) {
  if (job_stages_.empty() || wait_for_prev) {
    Stage_ptr stage(new Stage(this));
    job_stages_.push(stage);
  }
  job_stages_.back()->add(worker);
}

void Executor::run() {
  uint64_t start_ts = getTs();

  create_dir(get_config<std::string>("log_dir"));
  log_fname_ = get_log_name(job_name_);

  LOG(INFO) << "Start doing " << job_name_;

  while (!job_stages_.empty()) {
    job_stages_.front()->run();
    job_stages_.pop();
  }

  log_time(job_name_, start_ts);
}

void Executor::stop() {
  // finish existing jobs
  ios_work_.reset();
  executors_.join_all();
}

void Executor::interrupt() {
  executors_.interrupt_all();
  executors_.join_all();
}

int Executor::execute(Worker_ptr worker, std::string log) {
  worker->setup();

  std::string cmd = worker->getCommand() + " 2> " + log;
  int ret = system(cmd.c_str());

  worker->teardown();

  return ret;
}
} // namespace fcsgenome
