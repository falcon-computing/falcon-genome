#!/usr/bin/env bats

source common.sh
source settings.sh
source fcs_genome.sh
source fcs_test_mode.sh

@test "fcs-genome installed, PATH" {
  which fcs-genome &> /dev/null
  (($? == 0))
}

db_dir=/curr/wenp/Analysis/fcs-genome_test-env
db_samples=("CDMD1015")
tmp_dir=/curr/wenp/Analysis/fcs-genome_test-env
log_file="${tmp_dir}/fcs_test.log"

start_ts=$(date +%s)
for sample in ${db_samples[@]}; do

  @test "${sample} align" {
    if [[ (! -z $tool_mode) && ($tool_mode != "align") ]]; then
      skip
    fi
    fcs_genome_align ${sample} "${tmp_dir}/${sample}-align.log"
  }

  @test "${sample} markdup" {
    if [[ (! -z $tool_mode) && ($tool_mode != "markdup") ]]; then
      skip
    fi
    fcs_genome_markdup ${sample} "${tmp_dir}/${sample}-markdup.log"
  }

  @test "${sample} bqsr" {
    if [[ (! -z $tool_mode) && ($tool_mode != "bqsr") ]]; then
      skip
    fi
    fcs_genome_bqsr ${sample} "${tmp_dir}/${sample}-bqsr.log"
  }

  @test "${sample} htc" {
    if [[ (! -z $tool_mode) && ($tool_mode != "htc") ]]; then
      skip
    fi
    fcs_genome_htc ${sample} "${tmp_dir}/${sample}-htc.log"
  }

  @test "${sample} precision test" {
    if [ ! -z $tool_mode ]; then
      skip
    fi
    run $vcfdiff ${db_dir}/${sample}.vcf.gz ${tmp_dir}/${sample}.vcf.gz
    [ $status == 0 ]
    precision=$(echo "${lines[1]}"| awk '{print $NF}')
    ptest=$(echo "${precision} > 0.995" | bc)
    [ "$ptest" -eq 1 ]
  }
  @test "file clean" {
    rm -rf ${tmp_dir}/${sample}
    rm -rf ${tmp_dir}/${sample}.bam
    rm -rf ${tmp_dir}/${sample}.recal.bam
  }
done

end_ts=$(date +%s)
log_info "finish in $((end_ts - start_ts)) seconds"



