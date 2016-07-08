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
    "fcs-req_queue" + std::to_string((long long)getuid());

int main(int argc, char** argv) {

  // Initialize Google Flags
  gflags::SetUsageMessage(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  // Initialize Google Log
  google::InitGoogleLogging(argv[0]);

  int pid = 0;
  if (argc < 2) {
    // Make request for a slot
    pid = getpid();
  }
  else {
    // Free a slot
    pid = atoi(argv[1]);
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
      while (true) {
        try {
          char host[1000];
          unsigned int priority = 0;
          boost::interprocess::message_queue::size_type recv_size;

          DLOG(INFO) << "Opening queue " << std::to_string((long long) pid).c_str();

          boost::interprocess::message_queue client_q(
              boost::interprocess::open_only,
              std::to_string((long long)pid).c_str());

          DLOG(INFO) << "Opened queue " << std::to_string((long long) pid).c_str();

          client_q.receive(host, 1000, recv_size, priority);

          // Just to make sure char* is ended properly
          host[recv_size] = '\0';

          // Slot is allocated
          printf("%s %d\n", host, pid);
          break;
        }
        catch (boost::interprocess::interprocess_exception &e) {
          DLOG(INFO) << "Queue is not established yet, wait for a while: " 
            << e.what();
          // Catch exception when client_q is not created yet
          boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
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

