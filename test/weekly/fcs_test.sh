#!/bin/bash

source common.sh
source settings.sh
source fcs_genome.sh

check () {
  if [[ $# -ne 1 && $# -ne 2 ]]; then
    echo "Usage: binary [function]"
    exit 0
  fi
  local bin=$1

  which $bin &> /dev/null
  if [ "$?" -gt 0 ]; then
    echo "Cannot find \'$bin\' in PATH"
    exit -1
  fi

  if [ $bin = "fcs-genome" ]; then
    if [ $# = 2 ]; then
      tool_mode=1
    fi
  elif [ $bin = "XXX" ]; then
    echo "Usage: xxx"
    exit 0
  else
    echo "binnary is not in test list"
    exit 0
  fi
}

db_dir=/curr/wenp/Analysis/fcs-genome_test-env
db_samples=("CDMD1015")
tmp_dir=/curr/wenp/Analysis/fcs-genome_test-env
summary=/curr/wenp/Analysis/fcs-genome_test-env/summary.txt

check $*

if [ -f $summary]; then
  rm $summary
fi

start_ts=$(date +%s)
for sample in ${db_samples[@]}; do
  if [ ! -z $tool_mode ]; then
    if [ $tool_mode = "align" ]; then
      fcs_genome_align ${sample} "${tmp_dir}/${sample}-align.log"
      if [ $? = 0 ]; then
        rm -rf ${tmp_dir}/${sample}
	echo -e "${sample}\talign\tpass" >> $summary
      else
        echo -e "${sample}\talign\tfail" >> $summary
      fi
    elif [ $tool_mode = "markdup" ]; then
      fcs_genome_markdup ${sample} "${tmp_dir}/${sample}-markdup.log"
      if [ $? = 0 ]; then
        rm -rf ${tmp_dir}/${sample}.bam
	echo -e "${sample}\tmarkdup\tpass" >> $summary
      else
        echo -e "${sample}\tmarkdup\tfail" >> $summary
      fi
    elif [ $tool_mode = "bqsr" ]; then
      fcs_genome_bqsr ${sample} "${tmp_dir}/${sample}-bqsr.log"
      if [ $? = 0 ]; then
        rm -rf ${tmp_dir}/${sample}.recal.bam
	echo -e "${sample}\tbqsr\tpass" >> $summary
      else
        echo -e "${sample}\tbqsr\tfail" >> $summary
      fi
    elif [ $tool_mode = "htc" ]; then
      fcs_genome_htc ${sample} "${tmp_dir}/${sample}-htc.log"
      if [ $? = 0 ]; then
        echo -e "${sample}\thtc\tpass" >> $summary
      else
        echo -e "${sample}\thtc\tfail" >> $summary
      fi
    else
      echo "tool name wrong"
      exit -1
    fi
  else
    fcs_genome_align ${sample} "${tmp_dir}/${sample}-align.log"
    if [ $? = 1 ]; then
      echo -e "${sample}\tpipeline\tfail\talign" >> $summary
      continue
    fi
    fcs_genome_markdup ${sample} "${tmp_dir}/${sample}-markdup.log"
    if [ $? = 1 ]; then
      echo -e "${sample}\tpipeline\tfail\tmarkdup" >> $summary
      continue
    fi
    fcs_genome_bqsr ${sample} "${tmp_dir}/${sample}-bqsr.log"
    if [ $? = 1 ]; then
      echo -e "${sample}\tpipeline\tfail\tbqsr" >> $summary
      continue
    fi
    fcs_genome_htc ${sample} "${tmp_dir}/${sample}-htc.log"
    if [ $? = 1 ]; then
      echo -e "${sample}\tpipeline\tfail\thtc" >> $summary
      continue
    fi
    rm -rf ${tmp_dir}/${sample}.bam
    rm -rf ${tmp_dir}/${sample}.recal.bam

    precision=$($vcfdiff ${db_dir}/${sample}.vcf.gz $tmp_dir/${sample}.vcf.gz|sed -n '2,2p'|awk '{print $NF}')
    if [ $(echo "$precision > 0.995" | bc) -eq 1 ]; then
      echo -e "${sample}\tpipeline\tpass" >> $summary
    else
      echo -e "${sample}\tpipeline\tfail\tprecision-${precision}" >> $summary
    fi
  fi
done

cat $summary

end_ts=$(date +%s)
log_info "finish in $((end_ts - start_ts)) seconds"



