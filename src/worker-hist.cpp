#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <string>

#include <htslib/sam.h>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/workers.h"

#define num_partition 32

namespace fcsgenome {

struct bin_pos {
  int chr;
  int pos;
};

std::string chr_name[26] = {"1", "2", "3", "4", "5", "6",
                   "7", "8", "9", "10", "11", "12",
                   "13", "14", "15", "16", "17", "18",
                   "19", "20", "21", "22", "X", "Y", "MT","OTHER"};

void get_interval (int index,
    int start_bin, int end_bin, 
    bin_pos* bin_pos_batch,
    const int chr_length[])
{
  printf("Generate interval for partition %d\n", index);
  printf ("Partition %d contains bin %d - %d\n", index, start_bin, end_bin);
  std::ofstream fout;
  fout.open("intv" + std::to_string(index) + ".list");
  
  int start_chr = bin_pos_batch[start_bin].chr;
  int start_pos;
  // The first interval
  if(start_bin ==0 && index ==0) {
    start_pos = 1;
  }
  // in case the start pos is out of curr chr's bound
  else if (bin_pos_batch[start_bin].pos == chr_length[start_chr]){
    start_pos = 1;
    start_chr = start_chr + 1;
  }
  else {
    start_pos = bin_pos_batch[start_bin].pos + 1;
  }
  int end_chr = bin_pos_batch[end_bin].chr;
  int end_pos = bin_pos_batch[end_bin].pos;

  if (start_chr == end_chr) {
    printf ("%s:%d-%d\n", chr_name[start_chr].c_str(), start_pos, end_pos);
    fout << chr_name[start_chr] << ":" << start_pos << "-" << end_pos <<std::endl;
  }
  else {
    for (int curr_chr=start_chr; curr_chr<end_chr; curr_chr++) {
      printf ("%s:%d-%d\n", chr_name[curr_chr].c_str(), start_pos, chr_length[curr_chr]);
      fout << chr_name[curr_chr] << ":" << start_pos << "-" << chr_length[curr_chr] <<std::endl;
      start_pos = 1;
    }
    printf ("%s:%d-%d\n", chr_name[end_chr].c_str(), start_pos, end_pos);
    fout << chr_name[end_chr] << ":" << start_pos << "-" << end_pos <<std::endl;
  }
  fout.close();
}

void next_bin(const int bin_len, 
    const int num_chr, 
    const int length[],
    int &bin, int &chr, int &pos,
    bin_pos* bin_pos_batch) 
{
  int next_pos = pos + bin_len;
  while (chr < num_chr - 1 && next_pos > length[chr]) {
    next_pos -= length[chr];
    chr ++;
  }
  pos = next_pos;
  bin ++;

  bin_pos_batch[bin].chr = chr;
  bin_pos_batch[bin].pos = pos;
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

  bin_pos* bin_pos_batch = new bin_pos[num_bins];

  int curr_chr = 0;
  int curr_pos = bin_len;
  int curr_bin = 0;
  bin_pos_batch[0].chr = curr_chr;
  bin_pos_batch[0].pos = curr_pos;

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
      next_bin(bin_len, num_chr, length, curr_bin, curr_chr, curr_pos, bin_pos_batch);
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
  
  uint64_t partition_read_num = hist_reads / num_partition;
  uint64_t curr_read_num = hist[0];
  uint64_t old_read_num = 0;
  int j =0;
  curr_bin = 0;
  for (int i =0; i<num_partition; i++) {
    while(curr_read_num < partition_read_num * (i+1)
        && j < num_bins -1 ) {
        curr_read_num += hist[++j];
    }
    // in case there are uncontained reads
    if(i == num_partition -1 && j < num_bins - 1) {
      j = num_bins -1;
    }
    get_interval(i, curr_bin, j, bin_pos_batch, length);
    printf("Partition %d contains %d reads\n", i, curr_read_num - old_read_num);
    old_read_num = curr_read_num;
    curr_bin = j;
  }

  printf("%d/%d\n", hist_reads, total_reads);

  bam_destroy1(aln);
  sam_close(fin);

  delete [] hist;
  delete [] length;
  delete [] bin_pos_batch;
  return 0;
}

} // namespace fcsgenome
