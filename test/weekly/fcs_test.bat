#!/usr/bin/env bats

source common.sh
source settings.sh
source fcs_genome.sh


@test "args check" {
  (($# == 0)) || (($# == 1))
}

@test "fcs-genome installed, PATH" {
  which fcs-genome &> /dev/null
  (($? == 0))
}

db_dir=/curr/wenp/Analysis/fcs-genome_test-env
db_samples=("CDMD1015")
tmp_dir=/curr/wenp/Analysis/fcs-genome_test-env

@test "set tool_mode" {
  if [[ ! $# == 1 ]]; then
  skip
  fi
  tool_mode=$1
}

start_ts=$(date +%s)
for sample in ${db_samples[@]}; do

  @test "${sample} align" {
    if [[ (! -z $tool_mode) && ($tool_mode -ne "align") ]]; then
      skip
    fi
    fcs_genome_align ${sample} "${tmp_dir}/${sample}-align.log"
  }

  @test "${sample} markdup" {
    if [[ (! -z $tool_mode) && ($tool_mode -ne "markdup") ]]; then
      skip
    fi
    fcs_genome_markdup ${sample} "${tmp_dir}/${sample}-markdup.log"
  }

  @test "${sample} bqsr" {
    if [[ (! -z $tool_mode) && ($tool_mode -ne "bqsr") ]]; then
      skip
    fi
    fcs_genome_bqsr ${sample} "${tmp_dir}/${sample}-bqsr.log"
  }

  @test "${sample} htc" {
    if [[ (! -z $tool_mode) && ($tool_mode -ne "htc") ]]; then
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
    [ eval 'res=(${lines[0])}' && $(echo "${res[5]} > 0.995" | bc) -eq 1 ]
  }
  
  #rm -rf ${tmp_dir}/${sample}
  #rm -rf ${tmp_dir}/${sample}.bam
  #rm -rf ${tmp_dir}/${sample}.recal.bam
done

end_ts=$(date +%s)
log_info "finish in $((end_ts - start_ts)) seconds"



