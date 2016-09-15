namespace fcsgenome {

int bwa_command(std::string ref_path,
    std::string fq1_path,
    std::string fq2_path,
    std::string output_path,
    std::string sample_id,
    std::string read_group,
    std::string platform_id,
    std::string library_id);

int worker_align(int argc, char** argv);
int worker_markdup(int argc, char** argv);

} // namespace fcsgenome
