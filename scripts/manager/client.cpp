#include <boost/filesystem.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/thread.hpp>
#include <fstream>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <queue>
#include <stdio.h>
#include <string>
#include <unordered_map>

std::string queue_name = 
    "fcs-req_queue-" + std::to_string((long long)getuid());

DEFINE_string(q, "default", "Queue name for the manager");

int main(int argc, char** argv) {

  // Initialize Google Log
  google::InitGoogleLogging(argv[0]);

  // Initialize Google Flags
  gflags::SetUsageMessage(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  queue_name = queue_name + "-" + FLAGS_q;

  int pid = 0;
  if (argc < 2) {
    // Make request for a slot
    pid = getpid();

    VLOG(1) << "Request a slot for queue: " << FLAGS_q;
  }
  else {
    // Free a slot
    pid = atoi(argv[1]);

    VLOG(1) << "Freeing a slot for queue: " << FLAGS_q;
  }

  try {
    // Open message queue
    boost::interprocess::message_queue msg_q(
        boost::interprocess::open_only,
        queue_name.c_str());

    // Send own pid to message queue
    msg_q.send(&pid, sizeof(pid), 0);

    if (argc < 2) {
      // Wait incase client_q is not created yet
      boost::this_thread::sleep_for(boost::chrono::microseconds(100));

      // Start waiting for allocated slot
      bool wait_for_setup = true;
      while (wait_for_setup) {
        try {
          char host[1000];
          unsigned int priority = 0;
          boost::interprocess::message_queue::size_type recv_size;

          DLOG(INFO) << "Opening queue " << std::to_string((long long) pid).c_str();

          bool wait_for_msg = true;
          while (wait_for_msg) {
            boost::interprocess::message_queue client_q(
                boost::interprocess::open_only,
                std::to_string((long long)pid).c_str());

            // At this point exception means there are something wrong
            wait_for_setup = false;
            DLOG_IF(INFO, wait_for_setup) << "Opened queue " 
              << std::to_string((long long) pid).c_str();

            // Try receiving a message
            wait_for_msg = !client_q.try_receive(host, 1000, recv_size, priority);

            if (wait_for_msg) {
              boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
            }
          }

          // Just to make sure char* is ended properly
          host[recv_size] = '\0';

          // Slot is allocated
          printf("%s %d\n", host, pid);
          break;
        }
        catch (boost::interprocess::interprocess_exception &e) {
          if (wait_for_setup) {
            // Catch exception when client_q is not created yet
            DLOG(INFO) << "Queue is not established yet, wait for a while: " 
              << e.what();
            boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
          }
          else {
            DLOG(INFO) << "Failure in receiving from manager: " << e.what();
            return 1;
          }
        }
      }
    }
    else {
      VLOG(1) << "Freed slot for " << pid;
    }
  }
  catch (boost::interprocess::interprocess_exception &e) {
    LOG(ERROR) << e.what();
    return -1;
  }

  return 0;
}

