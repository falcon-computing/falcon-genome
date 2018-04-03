#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <vector>

#include <assert.h>

#include "fcs-genome/common.h"
#include "fcs-genome/config.h"
#include "fcs-genome/Executor.h"
#include "fcs-genome/workers.h"

#include "worker-bqsr.h"


namespace fcsgenome {
	void baserecalAddWorkers(Executor &, std::string &, std::vector<std::string> &, std::string &, std::string &, bool);
	void removePartialBQSR(std::string bqsr_path);
	void prAddWorkers(Executor &executor, std::string &ref_path, std::string &input_path, std::string &bqsr_path, std::string &output_path, bool flag_f);

	int json_main(int argc, char** argv, boost::program_options::options_description &opt_desc){
		namespace po = boost::program_options;

		// Define arguments
		po::variables_map cmd_vm;

		opt_desc.add_options()
			arg_decl_string("output,o", "output directory")
			arg_decl_string("json,j", "input file of json format");

		po::store(po::parse_command_line(argc, argv, opt_desc), cmd_vm);

		if(cmd_vm.count("help")){
			throw helpRequest();
		}
		po::notify(cmd_vm);

		std::string json_file_path = get_argument<std::string>(cmd_vm, "json");
		std::string output_path = get_argument<std::string>(cmd_vm, "output");
		FCSJson js(json_file_path);
		assert(js.getFqs().size() == js.getSamples().size());
		std::vector<string> jobs_selected = jobs_to_pipeline(js.getJobs());
		std::string temp_dir = conf_temp_dir; // here should be changed
		create_dir(temp_dir);
		std::string pre_job;
		Executor executor("GATK workflow");
		for(int i = 0; i < jobs_selected.size(); ++i) {
			std::string job = jobs_selected[i];
			if(job == "align") {
				std::string align_temp_dir = temp_dir + "/align";
				create_dir(align_temp_dir);
				for(int j = 0; j < js.getFqs().size(); j++) {
					std::string sample_id = (js.getSamples())[j];
					std::string library_id = sample_id;
					std::string read_group = "test";    // should be adjusted in json file?
					std::string platform_id = "illumina";    // see up
					std::string sample_dir = align_temp_dir + "/" + sample_id;
					create_dir(sample_dir);
					bool flag_f = true;    // see up
					Worker_ptr worker(new BWAWorker(js.getRef(), (js.getFqs())[i][0], (js.getFqs())[i][1], sample_dir, sample_id, read_group, platform_id, library_id, flag_f));
					executor.addTask(worker, 1);
				}
				pre_job = "align";
			}
			else if (job == "markdup") {
				std::string markdup_temp_dir = temp_dir + "/markdup";
				create_dir(markdup_temp_dir);
				if(i == 0) {
					for(int j = 0; j< js.getBams().size(); j++) {
						std::string sample_id = (js.getSamples())[j];
						bool flag_f = true;
						Worker_ptr worker(new MarkdupWorker((js.getBams())[j], markdup_temp_dir + "/" + sample_id + "_markdup.bam", flag_f));
						executor.addTask(worker, j==0);
					}
				}
				else {
					for(int j =0; j < js.getSamples().size();j++) {
						std::string sample_id = (js.getSamples())[j];
						std::string sample_align_dir = temp_dir + "/align/" + sample_id;
						bool flag_f = true;
						Worker_ptr worker( new MarkdupWorker(sample_align_dir, markdup_temp_dir + "/" + sample_id + "_markdup.bam", flag_f) );
						executor.addTask(worker, 1);
					}
				}
				//remove_path(temp_dir + "./" + pre_job);
				pre_job = "markdup";  
			}
			else if (job == "indel") {
				//std::string indel_temp_dir = tmp_dir + "./indel";
				//s

			}
			else if (job == "bqsr") {
				std::string bqsr_temp_dir = temp_dir + "/bqsr";
				create_dir(bqsr_temp_dir);
				if(i == 0) {
					for(int j = 0; j< js.getSamples().size(); j++) {
						//std::string inputBam = (js.getBams())[j];
						std::string inputBam = "/tmp/fcs-genome-wenp-17498/markdup/S1_markdup.bam";
						std::string ref_path = js.getRef();
						std::string bqsr_path = bqsr_temp_dir + "/" + (js.getSamples())[j] + ".grp";
						bool flag_f = true;
						std::string output = bqsr_temp_dir + "/" + (js.getSamples())[j];
						create_dir(output);
						baserecalAddWorkers(executor, ref_path, js.knownSites_, inputBam, bqsr_path, flag_f);
						prAddWorkers(executor, ref_path, inputBam, bqsr_path, output, flag_f);
					}
				}
				else {
					for(int j =0; j< js.getSamples().size(); j++) {
						std::string sample_id = (js.getSamples())[j];
						std::string sample_bqsr_temp_dir = bqsr_temp_dir + "/" + sample_id;
						create_dir(sample_bqsr_temp_dir);
						bool flag_f = true;
						std::string markBam = temp_dir + "/markdup/" + sample_id + "_markdup.bam";
						std::string ref_path = js.getRef();
						std::string bqsr_path = sample_bqsr_temp_dir + "/" + sample_id + ".grp";
						baserecalAddWorkers(executor, ref_path, js.knownSites_, markBam, bqsr_path, flag_f);
						prAddWorkers(executor, ref_path, markBam, bqsr_path, sample_bqsr_temp_dir, flag_f);
					}
				}
				pre_job = "bqsr";
			}
			else if (job == "htc") {
				std::string htc_temp_dir = temp_dir + "/htc";
				create_dir(htc_temp_dir);
				if(i == 0) {
					for(int j = 0; j < js.getSamples().size();j++) {
					}
				}
				else {
					//std::string intv_paths = temp_dir + "/intv_32";
					std::string ref_path = js.getRef();
					std::vector<std::string> intv_paths = init_contig_intv(ref_path);
					std::string bqsr_path = temp_dir + "/bqsr";
					for(int j = 0; j< js.getSamples().size(); j++) {
						std::string sample_id = (js.getSamples())[j];
						std::vector <std::string> output_files;
						std::string out_vcf = htc_temp_dir + "/" + sample_id + ".vcf";
						std::string sample_bqsr_path = bqsr_path + "/" + sample_id;
						for (int contig = 0; contig < get_config<int>("gatk.ncontigs"); contig++) {
							std::string input_file = get_contig_fname(sample_bqsr_path, contig);
							std::string output_file = get_contig_fname(htc_temp_dir, contig, "vcf");
							bool htc_flag = true;
							Worker_ptr worker(new HTCWorker(ref_path, intv_paths[contig], input_file, output_file, contig, htc_flag));
							executor.addTask(worker, contig == 0);
							output_files.push_back(output_file);
						}
						bool concat_flag = true;
						Worker_ptr worker(new VCFConcatWorker(output_files, out_vcf, concat_flag));
						executor.addTask(worker, true);
					}
				}
			}
			else {
				exit(0);
			}
		}
		executor.run();
	}
};
