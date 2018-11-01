#ifndef FCSGENOME_WORKERS_COMBINEGVCFSWORKER_H
#define FCSGENOME_WORKERS_COMBINEGVCFSWORKER_H

#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class CombineGVCFsWorker : public Worker {
 public:
  CombineGVCFsWorker(std::string ref_path,
      std::string input_path,
      std::string output_path,
      std::string database_name,
      //std::string intervals,
      bool &flag_f, 
      bool flag_gatk);

  void check();
  void setup();

 private:
  void genVid();
  void genCallSet();
  void genLoader();
  void genHostlist();

  std::string ref_path_;
  std::string input_path_;
  std::vector<std::string> input_files_;
  std::string output_path_;
  std::string database_name_;
  //std::string intervals_;
  bool flag_gatk_;

  std::string temp_dir_;
  std::string vid_json_;
  std::string callset_json_;
  std::string loader_json_;
  std::string host_list_;
};
} // namespace fcsgenome
#endif
