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



namespace fcsgenome {

	int json_main(int argc, char** argv, boost::program_options::options_description &opt_desc){
		namespace po = boost::program_options;

		// Define arguments
		po::variables_map cmd_vm;

		opt_desc.add_options()
			arg_decl_string("output,o", "output directory")
			arg_decl_string("josn,j", "input file of json format");

		po::store(po::parse_command_line(argc, argv, opt_desc), cmd_vm);

		if(cmd_vm.count("help")){
			throw helpRequest();
		}
		po::notify(cmd_vm);

		std::string json_file_path = get_argument<std::string>(cmd_vm, "json");
		std::string output_path = get_argument<std::string>(cmd_vm, "output");
		FCSJson js(json_file_path);
		assert(js.getFq().size() == js.getSamples().size());
		vector<string> jobs_selected = jobs_to_pipeline(js.getJobs());
		std::string tmp_dir = conf_temp_dir + "./out"; // this one should be changed
		create_dir(tmp_dir);
		std::string pre_job;
		for(int i = 0;i < jobs_selected.size(); ++i) {
			std::string job = jobs_selected.at(i);
			if(job == "align") {
				std::string align_temp_dir = tmp_dir + "./align";
				create_dir(align_temp_dir);
				Executor executor("bwa mem");
				for(int j = 0; j < js.getFq().size(); j++) {
					std::string sample_id = (js.getSample())[j];
					std::string library_id = "sample" + std::to_string(j);
					std::string read_group = "test";
					std::string platform_id = "illumina";
					bool flag_f = false;
					//std::string parts_dir;
					//if(boost::filesystem::exists(output_path) && !boost::filesystem::is_directory(output_path)){
					//	throw fileNotFound("Output path" + output_path + " is not a directory");
					//	parts_dir = output_path + "/" + sample_id + "/" + read_group;
					//	create_dir(output_path + "/" + sample_id);
					//}
					//Executor executor("bwa mem");
					Worker_ptr worker(new BWAWorker(js.getRef(), (js.getFq())[i][0], (js.getFq())[i][1], align_temp_dir, sample_id, read_group, platform_id, library_id, flag_f));
					executor.addTask(worker);
				}
				executor.run();
				pre_job = "align";
			}
			else if (job == "markdup") {
				std::string markdup_temp_dir = tmp_dir + "./markdup";
				create_dir(markdup_temp_dir);
				if(i == 0) {
					Executor executor("Mark Duplicates");
					for(int j = 0; j< js.getBam().size(); j++) {
						std::string sample_id = "sample" + std::to_string(j);
						Worker_ptr worker(new MarkDupWorker((js.getBam())[j], markdup_temp_dir + "./" + sample_id + "_markdup.bam", flag_f));
						executor.addTask(worker);
					}
					executor.run();
				}
				else {
					Executor executor("Mark Duplicates");
					//std::string input_dir = tmp_dir + "./" + pre_job;
					//get_input_list(input_dir, input_files, "*.bam", true);
					for(int j =0; j < js.getSample().size();j++) {
						std::string sample_id = (js.getSample())[j];
						std::string sample_dir = tmp_dir + "./align/" + sample_id;
						std::vector<std::string> input_files;
						get_input_list(sample_dir, input_files, "*.bam", true);
						Worker_ptr worker( new MarkDupWorker(input_files[0], markdup_temp_dir + "./" + sample_id + "_markdup.bam", flag_f) );
						executor.addTask(worker);
					}
					executor.run();
				}
				remove_path(tmp_dir + "./" + pre_job);
				pre_job = "markdup";
			}
			else if (job == "indel") {
				//std::string indel_temp_dir = tmp_dir + "./indel";
				//s

			}
			else if (job == "bqsr") {
				std::string bqsr_temp_dir = tmp_dir + "./bqsr";
				create_dir(bqsr_temp_dir);
				Executor executor("bqsr");
				if(i == 0) {
					for(int j = 0; j< js.getBam().size(); j++) {
						baserecalAddWorkers(executor, ref_path, known_sites, input_path, bqsr_path, flag_f);
						prAddWorkers(executor, ref_path, input_path, bqsr_path, output_path, flag_f);
					}
				}
				else {
					std::vector<std::string> input_files;
					get_input_list(tmp_dir + "./" + pre_job, input_files, "*.bam", true);
					for(int j =0; j< input_files.size(); j++) {
						baserecalAddWorkers(executor, ref_path, known_sites, input_files[j], bqsr_temp_dir, flag_f);
						prAddWorkers(executor, ref_path, input_files[j], bqsr_temp_dir, bqsr_temp_dir, flag_f);
					}
				}
				executor.run();
				pre_job = "bqsr";
			}
			else if (job == "htc") {
				std::string htc_temp_dir = tmp_dir + "./htc";
				create_dir(htc_temp_dir);
				Executor executor("htc");
				if(i == 0) {
					for(int j = 0; j < js.getBam().size();j++) {
						kkkkkkkk
					}
				}
			}
			else {
				exit(0);
			}
		}
	}
};
