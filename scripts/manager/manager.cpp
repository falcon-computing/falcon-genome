#include <boost/filesystem.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <fstream>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <queue>
#include <stdio.h>
#include <string>
#include <unordered_map>

int main(int argc, char** argv) {

  // Initialize Google Flags
  gflags::SetUsageMessage(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  // Initialize Google Log
  google::InitGoogleLogging(argv[0]);

  if (argc < 2) {
    printf("Usage: %s host_file\n", argv[0]);
    return 1;
  }

  // Build table for hosts and their slots
  std::unordered_map<std::string, int>  host_slots_table;
  std::unordered_map<std::string, int>  host_usage_table;
  std::unordered_map<int, std::string>  client_table;
  std::queue<std::string>               available_hosts;
  std::queue<std::string>               pending_requests;

  // Parse host file
  std::ifstream fin(argv[1]);
  
  if (!fin.is_open()) {
    LOG(ERROR) << "Cannot open host file at " << argv[1];
    return 1;
  }

  std::string file_line;
  while (std::getline(fin, file_line)) {
    std::stringstream ss(file_line);
    std::string host;
    int slots;

    ss >> host;
    ss >> slots;

    host_slots_table[host] = slots;
    host_usage_table[host] = 0;
    available_hosts.push(host);

    VLOG(1) << "Added host " << host << " with " << slots << " slots";
  }

  try {
    // Erase previous message queue
    boost::interprocess::message_queue::remove("fcs-req-queue");

    // Create a message_queue.
    boost::interprocess::message_queue msg_q(
        boost::interprocess::create_only,
        "fcs-req-queue",
        64,
        sizeof(int)); 

    // Start receiving loop
    while (true) {
      int pid = 0;
      unsigned int priority = 0;
      boost::interprocess::message_queue::size_type recv_size;
      msg_q.receive(&pid, sizeof(pid), recv_size, priority);

      if (recv_size != sizeof(pid) || pid < 0) {
        LOG(ERROR) << "Unrecognized message";
        break;
      }

      if (client_table.count(pid)) {
        // client is finished, remove it from host_usage_table
        std::string host = client_table[pid];
        int host_slots = host_usage_table[host];

        bool freed = false;
        if (!pending_requests.empty()) {

          std::string pid_s = pending_requests.front();
        
          DLOG(INFO) << "Freed one slot in " << host
            << ", re-allocate it for " << pid_s;

          boost::interprocess::message_queue client_q(
              boost::interprocess::open_only,
              pid_s.c_str());

          client_q.send(host.c_str(), host.size(), 0);

          pending_requests.pop();

          // Add pid to client_table
          client_table[std::stoi(pid_s)] = host;
        }
        else {
          host_usage_table[host]--;
          freed = true;

          DLOG(INFO) << "Freed one slot in " << host
            << ", currently there are " << host_slots-1 << " occupied";
        }
        // Remove the process queue
        boost::interprocess::message_queue::remove(
            std::to_string((long long) pid).c_str());

        VLOG(1) << "Process " << pid << " is finished";

        // Remove the finished client
        client_table.erase(pid);

        if (host_slots == host_slots_table[host] && freed) {
          // This means the host previously is fully occupied.
          // After freeing one slot the host can be added back
          // to available_host
          available_hosts.push(host);

          VLOG(1) << "Added " << host << " back to available hosts";
        }
      }
      else {
        VLOG(1) << "Received request from " << pid;

        // Create a new queue to send host information
        boost::interprocess::message_queue client_q(
            boost::interprocess::open_or_create,
            std::to_string((long long) pid).c_str(),
            1,
            1000*sizeof(char)); 

        if (available_hosts.empty()) {
          VLOG(1) << "No more slots available";
          pending_requests.push(std::to_string((long long) pid));
        }
        else {
          // Allocate host for current request
          std::string host = available_hosts.front();

          VLOG(1) << "Allocate a slot on " << host
            << " for " << pid;

          client_q.send(host.c_str(), host.size(), 0);

          // Add pid to client_table, and add one slot from table
          client_table[pid] = host;
          host_usage_table[host]++;

          if (host_usage_table[host] == host_slots_table[host]) {
            // Slots for this host are full, remove from available host
            available_hosts.pop();
            DLOG(INFO) << "Slots for " << host << " is full, remove from "
              << "available hosts";
          }
        }
      }
    }
  }
  catch (boost::interprocess::interprocess_exception &e) {
    LOG(ERROR) << e.what();
    return 1;
  }

  return 0;
}
