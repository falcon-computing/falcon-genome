#ifndef BQSR_H
#define BQSR_H

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <cmath>
#include <iomanip>
#include <string>
#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/workers.h"

namespace fcsgenome {
void baserecalAddWorkers(Executor &, std::string &, std::vector<std::string> &, std::string &, std::string &, bool);
void removePartialBQSR(std::string bqsr_path);
void prAddWorkers(Executor &executor, std::string &ref_path, std::string &input_path, std::string &bqsr_path, std::string &output_path, bool flag_f);
int baserecal_main(int argc, char** argv, boost::program_options::options_description &opt_desc);
int pr_main(int argc, char** argv, boost::program_options::options_description &opt_desc);
int bqsr_main(int argc, char** argv, boost::program_options::options_description &opt_desc);
};

#endif
