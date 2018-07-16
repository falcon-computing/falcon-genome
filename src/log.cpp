#include <iostream>
#include <string>
#include <fstream>

namespace fcsgenome {

Bool findError(std::vector<std::string> logs_, std::string match){
  bool if_match = false;
  for (int i = 0; i < logs_.size(); i++){
    std::ifstream fin(logs_[i], std::ios::in);
    if(if_match == true){
      return if_match;
    }
    else{
      int search_range = 1000;
      fin.seekg(0, ios_base::end);
      int length = fin.tellg();
      if(length > search_range){
        fin.seekg(length - search_range, ios_base::beg);}
      else{
        fin.seekg(0, ios_base::beg);}
      string line;
      if(fin.good()){getline(fin, line);}
      while(fin.good()){
        getline(fin, line); // get line from file
        size_t pos;
        pos=line.find("ERROR"); // search
        if(pos!=string::npos) // string::npos is returned if string is not found
        {
          match = line;
          if_match = true;
          break;
        }
        pos=line.find("fail"); // search
        if(pos!=string::npos) // string::npos is returned if string is not found
        {
          match = line;
          if_match = true;
          break;
        }
        pos=line.find("error"); // search
        if(pos!=string::npos) // string::npos is returned if string is not found
        {
          match = line;
          if_match = true;
          break;
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
