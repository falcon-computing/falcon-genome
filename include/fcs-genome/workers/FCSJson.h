#ifndef FCSJSON_H
#define FCSJSON_H

#include <stdlib.h>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include "/curr/diwu/tools/jsoncpp-1.7.7/include/json/json.h"

using namespace std;


class FCSJson {
	public:
		FCSJson(std::string File);
		void jsonRead();
		std::string getFile();
		std::string getRef();
		std::vector<std::vector<std::string> > getFqs();
		std::vector<std::string> getJobs();
		std::vector<std::string> getSamples();
		std::vector<std::string> getBams();
	private:
		std::string File_;
		std::string ref_path_;
		std::vector<std::vector<std::string> > fqFiles_;
		std::vector<std::string> jobs_;
		std::vector<std::string> samples_;
		std::vector<std::string> bams_;
	public:
		std::vector<std::string> knownSites_;
};




//const std::vector<std::string> pipeline = {"align", "indel", "baserecal", "bqsr", "printreads", "htc"};
const std::vector<std::string> pipeline = {"align", "markdup", "bqsr", "htc"};

const std::vector<std::string> alignValue = {"bqsr", "baserecal", "printreads", "htc", "indel"};
const std::vector<std::string> bqsrValue = {"printreads", "htc"};
const std::vector<std::string> baserecalValue = {"bqsr", "printreads", "htc", "indel"};
const std::vector<std::string> printreadsValue = {"htc"};
const std::vector<std::string> indelValue = {"bqsr", "baserecal", "printreads", "htc"};
const std::vector<std::string> htcValue;

const std::map <std::string, vector<std::string> > jobsMap  = {
	{"align", alignValue},
	{"bqsr", bqsrValue},
	{"baserecal", baserecalValue},
	{"printreads", printreadsValue},
	{"indel", indelValue},
	{"htc", htcValue}
};


std::vector<std::string> jobs_to_pipeline(const std::vector<string> jobs);

#endif
