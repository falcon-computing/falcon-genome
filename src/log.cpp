#include <iostream>
#include <string>
#include <fstream>
#include <vector>

namespace fcsgenome {

bool findError(std::vector<std::string> logs_, std::string & match){
  bool if_match = false;
  for (int i = 0; i < logs_.size(); i++){
    std::ifstream fin(logs_[i], std::ios::in);
    if(if_match == true){
      return if_match;
    }
    else{
      int search_range = 1000;
      fin.seekg(0, std::ios_base::end);
      int length = fin.tellg();
      if(length > search_range){
        fin.seekg(length - search_range, std::ios_base::beg);}
      else{
        fin.seekg(0, std::ios_base::beg);}
      std::string line;
      if(fin.good()){getline(fin, line);}
      while(fin.good()){
        getline(fin, line); 
        size_t pos;
        pos=line.find("ERROR"); // search ERROR for gatk logs
        if(pos!=std::string::npos) // string::npos is returned if string is not found
        {
          match = match + line + "\n";
          if_match = true;
          continue;
        }
        pos=line.find("fail"); // search fail for bwa logs
        if(pos!=std::string::npos) // string::npos is returned if string is not found
        {
          match = match + line + "\n";
          if_match = true;
          continue;
        }
        pos=line.find("error"); // search
        if(pos!=std::string::npos) // string::npos is returned if string is not found
        {
          match = match + line + "\n";
          if_match = true;
          continue;
        }
     }
     if(if_match == true){
       return if_match;
     }
  }
}
return if_match;
}
}
