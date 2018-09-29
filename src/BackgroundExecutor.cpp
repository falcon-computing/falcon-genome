#include <csignal>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>

#include "fcs-genome/common.h"
#include "fcs-genome/BackgroundExecutor.h"

namespace fcsgenome {

BackgroundExecutor::BackgroundExecutor( std::string job_name, Worker_ptr worker): Executor(job_name, 1)
{
  // start worker in the background in constructor
  worker->check();

  worker->setup();

  create_dir(get_config<std::string>("log_dir"));

  std::string tag = worker->getTaskName();
  //LOG(INFO) << "Background Executor tag " << tag;

  std::string log = get_log_name(tag);
  std::string cmd = worker->getCommand() + " &> " + log;
  LOG(INFO) << "BackgroundExecutor cmd : " << cmd;

  // fork and execute cmd using system call
  int pid = fork();
  if (pid) {
    // parent task
    child_pid_ = pid;
    DLOG(INFO) << "Started proc " << pid << " in the background";
  }
  else {
    // child task
    DLOG(INFO) << cmd;

    // create a wrapper script to run command
    std::string temp_dir = conf_temp_dir + "/background_executor";
    create_dir(temp_dir);

    std::string script_file = temp_dir + "/job-" + job_name + ".sh";

    std::stringstream cmd_sh;
    cmd_sh << "trap 'kill $(jobs -p) &> /dev/null' EXIT" << std::endl;
    cmd_sh << cmd << std::endl;

    // write the script to file
    std::ofstream fout;
    fout.open(script_file);
    if (!fout) {
      DLOG(ERROR) << "cannot write to " << script_file;    
      throw internalError("cannot start nam");
    }
    fout << cmd_sh.rdbuf();
    fout.close();

    job_script_ = script_file;

    // add x flag to script
    if (chmod(script_file.c_str(), S_IRWXU)) {
      DLOG(ERROR) << "cannot chmod +x on " << script_file;    
      throw internalError("cannot start nam");
    }

    //setsid();
    execl("/bin/sh", "sh", "-c", script_file.c_str(), (char*)0);

    remove_path(script_file);
    //int ret = system(cmd.c_str());
    //if (ret && !interrupted_) {
    //  LOG(WARNING) << "Background task has none zero ret";
    //}
  }
}

BackgroundExecutor::~BackgroundExecutor() {
  // send a signal and kill process
  DLOG(INFO) << "Send a signal to child process " << child_pid_ << " to terminate";
  kill(child_pid_, SIGALRM);
  if (!job_script_.empty()) {
    remove_path(job_script_);
    DLOG(INFO) << "deleted " << job_script_;
  }
}

} // namespace fcsgenome
