#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <string>

#include <htslib/sam.h>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/workers.h"

namespace fcsgenome {

void next_bin(const int bin_len, 
    const int num_chr, 
    const int length[],
    int &bin, int &chr, int &pos) 
{
  int next_pos = pos + bin_len;
  while (chr < num_chr && next_pos > length[chr]) {
    next_pos -= length[chr];
    chr ++;
  }
  pos = next_pos;
  bin ++;

  DLOG(INFO) << "Move to next bin " << bin << ":(" 
             << chr << "," << pos << ")";
}

int hist_main(int argc, char** argv,
    boost::program_options::options_description &opt_desc) 
{
  namespace po = boost::program_options;

  // Define arguments
  po::variables_map cmd_vm;
  int opt_int;

  opt_desc.add_options() 
    arg_decl_string("input,i", "input BAM file")
    arg_decl_string("output,o", "output histogram file in text format")
    arg_decl_int_w_def("num_bins,n", 4096, "number of bins")
    arg_decl_int_w_def("num_chr,c", 25, "number of chromosomes")
  ;

  // Parse arguments
  po::store(po::parse_command_line(argc, argv, opt_desc),
      cmd_vm);

  if (cmd_vm.count("help")) { 
    throw helpRequest();
  } 

  std::string input_path = get_argument<std::string>(cmd_vm, "input");
  int num_bins = get_argument<int>(cmd_vm, "num_bins");
  int num_chr  = get_argument<int>(cmd_vm, "num_chr");

  input_path = check_input(input_path);

  // open bam file
  samFile*   fin   = hts_open(input_path.c_str() ,"r");
  bam_hdr_t* b_hdr = sam_hdr_read(fin);
  bam1_t*    aln   = bam_init1();

  // build histogram bin bounds
  if (num_chr <= 0 || num_chr > b_hdr->n_targets) {
    num_chr = b_hdr->n_targets;
  }
  DLOG(INFO) << "num_chr = " << num_chr;
  DLOG(INFO) << "num_bin = " << num_bins;

  uint64_t total_len = 0;
  int* length = new int[b_hdr->n_targets];
  for (int i = 0; i < b_hdr->n_targets; i++) {
    length[i] = b_hdr->target_len[i];
    if (i < num_chr) {
      total_len += length[i];
    }
  }
  uint64_t bin_len = (total_len + num_bins - 1) / num_bins;

  DLOG(INFO) << "total_len = " << total_len
             << ", bin_len = " << bin_len;

  uint64_t* hist = new uint64_t[num_bins];
  for (int i = 0; i < num_bins; i++) {
    hist[i] = 0;
  }

  int curr_chr = 0;
  int curr_pos = bin_len;
  int curr_bin = 0;

  uint64_t total_reads = 0;
  while (sam_read1(fin, b_hdr, aln) > 0) {
    int chr = aln->core.tid;
    int pos = aln->core.pos;

    if (chr >= num_chr) {
      break;
    }
    total_reads += 1;

    while ( curr_chr < chr ||
           (curr_pos < pos && chr == curr_chr)) 
    {
      next_bin(bin_len, num_chr, length, curr_bin, curr_chr, curr_pos);
    }

    // filtering 
    if (aln->core.qual > 20 &&
       (aln->core.flag & 0x400) == 0) {
      hist[curr_bin] ++;
    }
  }

  uint64_t hist_reads = 0;
  for (int i = 0; i < num_bins; i++) {
    printf("%d\n", hist[i]);
    hist_reads += hist[i];
  }

  printf("%d/%d\n", hist_reads, total_reads);

  bam_destroy1(aln);
  sam_close(fin);

  delete [] hist;
  delete [] length;

  return 0;
}
} // namespace fcsgenome
