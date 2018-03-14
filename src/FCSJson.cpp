#include "fcs-genome/workers/FCSJson.h"

FCSJson::FCSJson(std::string File) : File_(File) {
	jsonRead();
}

void FCSJson::jsonRead () {
	using namespace Json;
	ifstream in;
	in.open(File_);
	Json::Reader reader;
	Json::Value root;
	bool jsonB = reader.parse(in, root);
	in.close();
	if(!jsonB) {
		throw runtime_error("json File read failure");
		exit(EXIT_FAILURE);
	}

	assert(root.size() == 4);

	for(auto it1 = root.begin(); it1 != root.end(); ++it1){
		//std::string str(it1.name());
		if(it1.name() == "ref"){
			ref_path_ = root["ref"].asString();
		}
		else if(it1.name() == "fastq") {
			int nSample = root["fastq"].size();
			//std::cout << nSample << std::endl;
			for(int i = 0; i < nSample; ++i){
				vector<std::string> tmp(2);
				tmp[0] = root["fastq"][i][0].asString();
				tmp[1] = root["fastq"][i][1].asString();
				fqFiles_.push_back(tmp);
			}
		}
		else if(it1.name() == "jobs") {
			for(int i = 0; i < root["jobs"].size(); ++i)
				jobs_.push_back(root["jobs"][i].asString());
		}
		else if(it1.name() == "samples") {
			for(int i = 0; i < root["samples"].size(); ++i)
				sample_.push_back(root["jobs"][i].asString());
		}
		else {
			std::cerr << "josn file id not  identified!" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}

std::string FCSJson::getFile () { return File_; }
std::string FCSJson::getRef () { return ref_path_; }
std::vector<std::vector<std::string> > FCSJson::getFq () { return fqFiles_; }
std::vector<std::string> FCSJson::getJobs () { return jobs_; }
std::vector<std::string> FCSJson::getSamples () { return samples_; }

std::vector<std::string> jobs_to_pipeline(const std::vector<std::string> jobs) {

	int maxIndex=-1, minIndex=-1;
	for(int i=0;i<jobs.size();i++)	 {
		//int index = pipeline.find(jobs.at(i));
		int index;
		for(int  n = 0; n < pipeline.size(); n++)
			if(pipeline.at(n) == jobs.at(i))
				index = n;
		if(index > maxIndex) maxIndex = index;
		if(index < minIndex) minIndex =index;
	}

	assert(maxIndex !=-1 && minIndex != -1);

	vector<std::string> jobs_selected;
	for(int i=minIndex; i<= maxIndex; ++i)
		jobs_selected.push_back(pipeline.at(i));

	return jobs_selected;
}
