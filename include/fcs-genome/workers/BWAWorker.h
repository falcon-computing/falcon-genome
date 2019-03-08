#ifndef FCSGENOME_WORKERS_BWAWORKER_H
#define FCSGENOME_WORKERS_BWAWORKER_H

#include <string>
#include "fcs-genome/Worker.h"

namespace fcsgenome {

class BWAWorker : public Worker {
 public:
  BWAWorker(std::string ref_path,
      std::string fq1_path,
      std::string fq2_path,
      std::string partdir_path,
      std::string output_path,
      std::vector<std::string> extra_opts,
      std::string sample_id,
      std::string read_group,
      std::string platform_id,
      std::string library_id,
      bool flag_merge_bams,
      bool flag_align_only,
      bool &flag_f);

  void check();
  void setup();

 private:
  std::string ref_path_;
  std::string fq1_path_;
  std::string fq2_path_;
  std::string partdir_path_;
  std::string output_path_;

  std::string sample_id_;
  std::string read_group_;
  std::string platform_id_;
  std::string library_id_;

  bool flag_align_only_;
  bool flag_merge_bams_;
  bool flag_f_;
};

} // namespace fcsgenome
#endif
