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
    std::string line;
    while (fin.good()) {
      getline(fin, line); 
      size_t pos;
      pos = line.find("##### ERROR"); // search ERROR for gatk logs
      if (pos != std::string::npos) {
        match = match + line + "\n";
        continue;
      }
      pos = line.find("[E::"); // search fail for bwa logs
      if(pos != std::string::npos) {
        match = match + line + "\n";
        continue;
      }
      //pos = line.find("error"); // search
      //if(pos != std::string::npos) {
      //  match = match + line + "\n";
      //  continue;
      //}
    }
    if (!match.empty()) {
      if (message.empty()) {
        message = match;
      }
      else if (message.compare(match) != 0) {
        return std::string();
      }
    }
  }
  return message;
}
} // namespace fcsgenome
