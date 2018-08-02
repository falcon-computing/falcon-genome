#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "fcs-genome/LogUtils.h"

namespace fcsgenome {

std::string LogUtils::findError(std::vector<std::string> logs_) {
  std::string message;
  for (int i = 0; i < logs_.size(); i++) {
    std::ifstream fin(logs_[i], std::ios::in);
    std::string match;
    std::string last_line;
    while (fin.good()) {
      std::string line;
      getline(fin, line); 
      if (line.find("##### ERROR") != std::string::npos) {
        match = match + line + "\n";
      }
      else if (line.find("[E::") != std::string::npos) {
        match = match + line + "\n";
      }
      else if (!line.empty()) {
        last_line = line + '\n';
      }
    }
    if (message.empty()) {
      if (!match.empty()) message = match;
      else message = last_line; // get the last line and treat it as error
    }
    else if (message.compare(match) != 0) {
      if (!match.empty()) return match;
      else return std::string();
    }
  }
  // message will be the shared message accross all logs
  return message;
}
} // namespace fcsgenome
