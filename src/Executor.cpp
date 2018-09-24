#include <algorithm>
#include <boost/smart_ptr.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/wait.h>

#include "fcs-genome/common.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/LogUtils.h"

namespace fcsgenome {

boost::mutex mutex_sig;

// put signal handler here because it will be the worker thread 
// that catches the signal
void sigint_handler(int s) {
  boost::lock_guard<boost::mutex> guard(mutex_sig);
  //DLOG(INFO) << "Caught interrupt in worker";
  LOG(INFO) << "Caught interrupt, cleaning up...";
  if (g_executor) {
    //kill(getppid(), SIGINT); 
    DLOG(INFO) << "Deleting the executor";
    delete g_executor;
  }

  // delete temp folder
#ifndef NDEBUG
  remove_path(conf_temp_dir);
#endif
  exit(1);
}

Stage::Stage(Executor* executor): executor_(executor) {;}

void Stage::add(Worker_ptr worker, std::string job_label) {
  std::string tag = job_label;
  tag = job_label;
  LOG(INFO) << "Stage::add " << executor_->get_log_name(tag, logs_.size());
  DLOG(INFO) << "Stage::add " << executor_->get_log_name(tag, logs_.size());
  logs_.push_back(executor_->get_log_name(tag, logs_.size()));
  tasks_.push_back(worker);  
}

void Stage::run(std::string job_name, std::string sample_id) {
  uint64_t start_ts = getTs();

  typedef boost::packaged_task<void> task_t;
  std::vector<boost::unique_future<void> > pending_tasks_;

  std::string job_logname=logs_[0];
  boost::replace_all(job_logname, ".log.0", ".log");
  std::ofstream outlog(job_logname, std::ios::out|std::ios::app);

  if (!sample_id.empty()) {
    outlog << "Start doing " << job_name << " for " << sample_id << std::endl;
    LOG(INFO) << "Start doing " << job_name << " for " << sample_id;
  }
  else{
    outlog << "Start doing " << job_name  << std::endl;
    LOG(INFO) << "Start doing " << job_name;
  }

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

  outlog << job_name << " finishes in "  << getTs() - start_ts << " seconds" << std::endl;
  if (sample_id.empty()) {
    log_time(job_name , start_ts);
  }
  else{
    log_time(job_name + " " + sample_id , start_ts);
  }

  outlog.close();

  // concat all logs
  std::string output_logname=executor_->log();
  
  std::ofstream fout(output_logname, std::ios::out|std::ios::app); 
  for (int i = 0; i < logs_.size(); i++) {
    LOG(INFO) <<  "logs_ " << logs_[i] ; 
    std::ifstream fin(logs_[i], std::ios::in);   
    if (fin) {
      fout << fin.rdbuf();
    }
    fin.close();
  }
  fout.flush();
  fout.close();

  //check if any error message in log files
  if (!status_.empty()) {
    fcsgenome::LogUtils logUtils;
    std::string match = logUtils.findError(logs_);
    LOG(ERROR) << executor_->job_name() << " failed, please check log: " 
               << executor_->log()  << " for details.";
    if (!match.empty()) {
      LOG(INFO) << "Potential errors:";
      std::cout << match;
    }
    throw failedCommand("");
  }

  for (int i = 0; i < logs_.size(); i++) {
    // remove task log
    remove_path(logs_[i]);
  }
  DLOG(INFO) << "Stage finishes in " << getTs() - start_ts << " seconds";
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

Executor::Executor(std::string job_name, std::vector<std::string> stage_levels,  std::string sample_id,
    int num_executors): 
  job_name_(job_name), 
  stage_levels_(stage_levels),
  sample_id_(sample_id),
  num_executors_(num_executors),
  job_id_(0)
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

  DLOG(INFO) << "Executor thread id is " << getpid();

  // create temp dir for the executor
  temp_dir_ = conf_temp_dir + "/executor";
  create_dir(temp_dir_);

  // create log folder :  
  log_dir_ = get_config<std::string>("log_dir");
  try {
    create_dir(log_dir_);
  } catch (silentExit & e) {
    std::string username("");
    username = std::getenv("USER");
    log_dir_ = "/tmp/log" + username;
    create_dir(log_dir_);
  }
  if (!sample_id_.empty()){
    log_fname_ = get_log_name(job_name_ + " " + sample_id_);
  }
  else{
    log_fname_ = get_log_name(job_name_);
  }

}

Executor::~Executor() {

  DLOG(INFO) << "Killing executor";

  // kill all forked processes
  for (int i = 0; i < job_id_.load(); i++) {
    std::string script_file = temp_dir_ + "/job-" +
      std::to_string((long long)i) +
      ".sh";
    std::string pid_file = script_file + ".pid";

    std::ifstream fin;
    fin.open(pid_file);
    if (fin && !fin.eof()) {
      int pid;
      fin >> pid;

      DLOG(INFO) << "Killing pid = " << pid;
      kill(pid, SIGHUP);
    }
    remove_path(pid_file);
  }

  // wait for worker threads to finish
  ios_work_.reset();
  executors_.join_all();
}

void Executor::addTask(Worker_ptr worker, std::string job_label, bool wait_for_prev) {
  if (job_stages_.empty() || wait_for_prev) {
    Stage_ptr stage(new Stage(this));
    job_stages_.push(stage);
  }
  job_stages_.back()->add(worker, job_label);
}

void Executor::run() {
  uint64_t start_ts = getTs();

  int count=0;
  while (!job_stages_.empty()) {
    job_stages_.front()->run(stage_levels_[count], sample_id_);
    job_stages_.pop();
    count++;
  }
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

  int job_id = job_id_.fetch_add(1);

  // setup worker
  try {
    worker->setup();

    std::string cmd;

    // launch system calls through ssh if using latency mode
    if (get_config<bool>("latency_mode") && 
        worker->num_process_ == 1 &&
        conf_host_list.size() > 1) 
    {
      // construct wrapper script for cmd to record pid
      std::string script_file = temp_dir_ + "/job-" +
        std::to_string((long long)job_id) +
        ".sh";
      std::string pid_file = script_file + ".pid";

      // generate a script the record the process id
      std::stringstream cmd_sh;
      cmd_sh << worker->getCommand()
        << " 2> " << log
        << " &" << std::endl;
      cmd_sh << "pid=$!" << std::endl;
      cmd_sh << "echo $pid > " << pid_file << std::endl;
      cmd_sh << "wait \"$pid\"" << std::endl;
      cmd_sh << "ret=$?" << std::endl;
      cmd_sh << "rm -f " << pid_file << std::endl;
      cmd_sh << "exit $ret" << std::endl;

      DLOG(INFO) << worker->getCommand();

      // write the script to file
      std::ofstream fout;
      fout.open(script_file);
      fout << cmd_sh.rdbuf();
      fout.close();

      // obtain host name from host_list
      int host_id = job_id % conf_host_list.size();
      std::string host = conf_host_list[host_id];

      cmd = "ssh -q " + host + " '/bin/bash -s' < " +
        script_file;
    }
    else {
      cmd = worker->getCommand() + " 2> " + log;
      DLOG(INFO) << cmd;
    }

    signal(SIGINT, sigint_handler);

    // fork command
    return system(cmd.c_str());
  } 
  catch (std::runtime_error &e) {
    DLOG(ERROR) << "Failed to setup job execution, because: " << e.what();
    return 1;
  }
}

std::string Executor:: get_log_name(std::string job_name, int idx) {
  time_t timestamp = ::time(0);
  struct ::tm tm_time;
  localtime_r(&timestamp, &tm_time);

  // manipulate stage_name to replace all spaces and to lower case
  std::string log_name = job_name;
  std::transform(log_name.begin(), log_name.end(), log_name.begin(),
		   [](char c) {
		     return c == ' ' ? '-' : ::tolower(c);
		   });

  std::stringstream ss;
  ss << log_dir_ << "/"
     << log_name << "-"
     << 1900 + tm_time.tm_year
     << std::setw(2) << std::setfill('0') << 1+tm_time.tm_mon
     << std::setw(2) << std::setfill('0') << tm_time.tm_mday
     << '-'
     << std::setw(2) << std::setfill('0') << tm_time.tm_hour
     << std::setw(2) << std::setfill('0') << tm_time.tm_min
     << std::setw(2) << std::setfill('0') << tm_time.tm_sec
     << ".log";
  if (idx >= 0) {
    ss << "." << idx;
  }
  DLOG(INFO) << "Log File : "  << ss.str();
  return  ss.str();
}

} // namespace fcsgenome
