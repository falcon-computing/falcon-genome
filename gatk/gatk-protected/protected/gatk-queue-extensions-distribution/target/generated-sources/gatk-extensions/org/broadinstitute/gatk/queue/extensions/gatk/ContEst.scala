package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class ContEst extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "ContEst"
analysis_type = "ContEst"
scatterClass = classOf[LocusScatterFunction]
setupScatterFunction = { case scatter: GATKScatterFunction => scatter.includeUnmapped = false }

/** the genotype information for our sample */
@Input(fullName="genotypes", shortName="genotypes", doc="the genotype information for our sample", required=false, exclusiveOf="", validation="")
var genotypes: File = _

/** Dependencies on the index of genotypes */
@Input(fullName="genotypesIndex", shortName="", doc="Dependencies on the index of genotypes", required=false, exclusiveOf="", validation="")
private var genotypesIndex: Seq[File] = Nil

/** the variant file containing information about the population allele frequencies */
@Input(fullName="popfile", shortName="pf", doc="the variant file containing information about the population allele frequencies", required=true, exclusiveOf="", validation="")
var popfile: File = _

/**
 * Short name of popfile
 * @return Short name of popfile
 */
def pf = this.popfile

/**
 * Short name of popfile
 * @param value Short name of popfile
 */
def pf_=(value: File) { this.popfile = value }

/** Dependencies on the index of popfile */
@Input(fullName="popfileIndex", shortName="", doc="Dependencies on the index of popfile", required=false, exclusiveOf="", validation="")
private var popfileIndex: Seq[File] = Nil

/** An output file created by the walker.  Will overwrite contents if file exists */
@Output(fullName="out", shortName="o", doc="An output file created by the walker.  Will overwrite contents if file exists", required=false, exclusiveOf="", validation="")
@Gather(classOf[org.broadinstitute.gatk.queue.function.scattergather.SimpleTextGatherFunction])
var out: File = _

/**
 * Short name of out
 * @return Short name of out
 */
def o = this.out

/**
 * Short name of out
 * @param value Short name of out
 */
def o_=(value: File) { this.out = value }

/** threshold for minimum base quality score */
@Argument(fullName="min_qscore", shortName="", doc="threshold for minimum base quality score", required=false, exclusiveOf="", validation="")
var min_qscore: Option[Int] = None

/** threshold for minimum mapping quality score */
@Argument(fullName="min_mapq", shortName="", doc="threshold for minimum mapping quality score", required=false, exclusiveOf="", validation="")
var min_mapq: Option[Int] = None

/** at most, what fraction of sites should be trimmed based on BETA_THRESHOLD */
@Argument(fullName="trim_fraction", shortName="", doc="at most, what fraction of sites should be trimmed based on BETA_THRESHOLD", required=false, exclusiveOf="", validation="")
var trim_fraction: Option[Double] = None

/** Format string for trim_fraction */
@Argument(fullName="trim_fractionFormat", shortName="", doc="Format string for trim_fraction", required=false, exclusiveOf="", validation="")
var trim_fractionFormat: String = "%s"

/** threshold for p(f>=0.5) to trim */
@Argument(fullName="beta_threshold", shortName="", doc="threshold for p(f>=0.5) to trim", required=false, exclusiveOf="", validation="")
var beta_threshold: Option[Double] = None

/** Format string for beta_threshold */
@Argument(fullName="beta_thresholdFormat", shortName="", doc="Format string for beta_threshold", required=false, exclusiveOf="", validation="")
var beta_thresholdFormat: String = "%s"

/** set to META (default), SAMPLE or READGROUP to produce per-bam, per-sample or per-lane estimates */
@Argument(fullName="lane_level_contamination", shortName="llc", doc="set to META (default), SAMPLE or READGROUP to produce per-bam, per-sample or per-lane estimates", required=false, exclusiveOf="", validation="")
var lane_level_contamination: Seq[org.broadinstitute.gatk.tools.walkers.cancer.contamination.ContEst.ContaminationRunType] = Nil

/**
 * Short name of lane_level_contamination
 * @return Short name of lane_level_contamination
 */
def llc = this.lane_level_contamination

/**
 * Short name of lane_level_contamination
 * @param value Short name of lane_level_contamination
 */
def llc_=(value: Seq[org.broadinstitute.gatk.tools.walkers.cancer.contamination.ContEst.ContaminationRunType]) { this.lane_level_contamination = value }

/** The sample name; used to extract the correct genotypes from mutli-sample truth vcfs */
@Argument(fullName="sample_name", shortName="sn", doc="The sample name; used to extract the correct genotypes from mutli-sample truth vcfs", required=false, exclusiveOf="", validation="")
var sample_name: String = _

/**
 * Short name of sample_name
 * @return Short name of sample_name
 */
def sn = this.sample_name

/**
 * Short name of sample_name
 * @param value Short name of sample_name
 */
def sn_=(value: String) { this.sample_name = value }

/** the degree of precision to which the contamination tool should estimate (e.g. the bin size) */
@Argument(fullName="precision", shortName="pc", doc="the degree of precision to which the contamination tool should estimate (e.g. the bin size)", required=false, exclusiveOf="", validation="")
var precision: Option[Double] = None

/**
 * Short name of precision
 * @return Short name of precision
 */
def pc = this.precision

/**
 * Short name of precision
 * @param value Short name of precision
 */
def pc_=(value: Option[Double]) { this.precision = value }

/** Format string for precision */
@Argument(fullName="precisionFormat", shortName="", doc="Format string for precision", required=false, exclusiveOf="", validation="")
var precisionFormat: String = "%s"

/** Where to write a full report about the loci we processed */
@Argument(fullName="base_report", shortName="br", doc="Where to write a full report about the loci we processed", required=false, exclusiveOf="", validation="")
var base_report: File = _

/**
 * Short name of base_report
 * @return Short name of base_report
 */
def br = this.base_report

/**
 * Short name of base_report
 * @param value Short name of base_report
 */
def br_=(value: File) { this.base_report = value }

/** write the likelihood values to the specified location */
@Argument(fullName="likelihood_file", shortName="lf", doc="write the likelihood values to the specified location", required=false, exclusiveOf="", validation="")
var likelihood_file: File = _

/**
 * Short name of likelihood_file
 * @return Short name of likelihood_file
 */
def lf = this.likelihood_file

/**
 * Short name of likelihood_file
 * @param value Short name of likelihood_file
 */
def lf_=(value: File) { this.likelihood_file = value }

/** should we verify that the sample name is in the genotypes file? */
@Argument(fullName="verify_sample", shortName="vs", doc="should we verify that the sample name is in the genotypes file?", required=false, exclusiveOf="", validation="")
var verify_sample: Boolean = _

/**
 * Short name of verify_sample
 * @return Short name of verify_sample
 */
def vs = this.verify_sample

/**
 * Short name of verify_sample
 * @param value Short name of verify_sample
 */
def vs_=(value: Boolean) { this.verify_sample = value }

/** what minimum number of bases do we need to see to call contamination in a lane / sample? */
@Argument(fullName="minimum_base_count", shortName="mbc", doc="what minimum number of bases do we need to see to call contamination in a lane / sample?", required=false, exclusiveOf="", validation="")
var minimum_base_count: Option[Int] = None

/**
 * Short name of minimum_base_count
 * @return Short name of minimum_base_count
 */
def mbc = this.minimum_base_count

/**
 * Short name of minimum_base_count
 * @param value Short name of minimum_base_count
 */
def mbc_=(value: Option[Int]) { this.minimum_base_count = value }

/** evaluate contamination for just a single contamination population */
@Argument(fullName="population", shortName="population", doc="evaluate contamination for just a single contamination population", required=false, exclusiveOf="", validation="")
var population: String = _

/** which approach should we take to getting the genotypes (only in array-free mode) */
@Argument(fullName="genotype_mode", shortName="gm", doc="which approach should we take to getting the genotypes (only in array-free mode)", required=false, exclusiveOf="", validation="")
var genotype_mode: org.broadinstitute.gatk.tools.walkers.cancer.contamination.ContEst.SeqGenotypeMode = _

/**
 * Short name of genotype_mode
 * @return Short name of genotype_mode
 */
def gm = this.genotype_mode

/**
 * Short name of genotype_mode
 * @param value Short name of genotype_mode
 */
def gm_=(value: org.broadinstitute.gatk.tools.walkers.cancer.contamination.ContEst.SeqGenotypeMode) { this.genotype_mode = value }

/** progressively trim from 0 to TRIM_FRACTION by this interval */
@Argument(fullName="trim_interval", shortName="", doc="progressively trim from 0 to TRIM_FRACTION by this interval", required=false, exclusiveOf="", validation="")
var trim_interval: Option[Double] = None

/** Format string for trim_interval */
@Argument(fullName="trim_intervalFormat", shortName="", doc="Format string for trim_interval", required=false, exclusiveOf="", validation="")
var trim_intervalFormat: String = "%s"

/** minimum depth at a site to consider in calculation */
@Argument(fullName="min_site_depth", shortName="", doc="minimum depth at a site to consider in calculation", required=false, exclusiveOf="", validation="")
var min_site_depth: Option[Int] = None

/** use a constant epsilon (phred scale) for calculation */
@Argument(fullName="fixed_epsilon_qscore", shortName="", doc="use a constant epsilon (phred scale) for calculation", required=false, exclusiveOf="", validation="")
var fixed_epsilon_qscore: Option[Byte] = None

/** what minimum depth is required to call a site in seq genotype mode */
@Argument(fullName="min_genotype_depth", shortName="", doc="what minimum depth is required to call a site in seq genotype mode", required=false, exclusiveOf="", validation="")
var min_genotype_depth: Option[Int] = None

/** the ratio of alt to other bases to call a site a hom non-ref variant */
@Argument(fullName="min_genotype_ratio", shortName="", doc="the ratio of alt to other bases to call a site a hom non-ref variant", required=false, exclusiveOf="", validation="")
var min_genotype_ratio: Option[Double] = None

/** Format string for min_genotype_ratio */
@Argument(fullName="min_genotype_ratioFormat", shortName="", doc="Format string for min_genotype_ratio", required=false, exclusiveOf="", validation="")
var min_genotype_ratioFormat: String = "%s"

/** the min log likelihood for UG to call a genotype */
@Argument(fullName="min_genotype_llh", shortName="", doc="the min log likelihood for UG to call a genotype", required=false, exclusiveOf="", validation="")
var min_genotype_llh: Option[Double] = None

/** Format string for min_genotype_llh */
@Argument(fullName="min_genotype_llhFormat", shortName="", doc="Format string for min_genotype_llh", required=false, exclusiveOf="", validation="")
var min_genotype_llhFormat: String = "%s"

/** Filter out reads with CIGAR containing the N operator, instead of failing with an error */
@Argument(fullName="filter_reads_with_N_cigar", shortName="filterRNC", doc="Filter out reads with CIGAR containing the N operator, instead of failing with an error", required=false, exclusiveOf="", validation="")
var filter_reads_with_N_cigar: Boolean = _

/**
 * Short name of filter_reads_with_N_cigar
 * @return Short name of filter_reads_with_N_cigar
 */
def filterRNC = this.filter_reads_with_N_cigar

/**
 * Short name of filter_reads_with_N_cigar
 * @param value Short name of filter_reads_with_N_cigar
 */
def filterRNC_=(value: Boolean) { this.filter_reads_with_N_cigar = value }

/** Filter out reads with mismatching numbers of bases and base qualities, instead of failing with an error */
@Argument(fullName="filter_mismatching_base_and_quals", shortName="filterMBQ", doc="Filter out reads with mismatching numbers of bases and base qualities, instead of failing with an error", required=false, exclusiveOf="", validation="")
var filter_mismatching_base_and_quals: Boolean = _

/**
 * Short name of filter_mismatching_base_and_quals
 * @return Short name of filter_mismatching_base_and_quals
 */
def filterMBQ = this.filter_mismatching_base_and_quals

/**
 * Short name of filter_mismatching_base_and_quals
 * @param value Short name of filter_mismatching_base_and_quals
 */
def filterMBQ_=(value: Boolean) { this.filter_mismatching_base_and_quals = value }

/** Filter out reads with no stored bases (i.e. '*' where the sequence should be), instead of failing with an error */
@Argument(fullName="filter_bases_not_stored", shortName="filterNoBases", doc="Filter out reads with no stored bases (i.e. '*' where the sequence should be), instead of failing with an error", required=false, exclusiveOf="", validation="")
var filter_bases_not_stored: Boolean = _

/**
 * Short name of filter_bases_not_stored
 * @return Short name of filter_bases_not_stored
 */
def filterNoBases = this.filter_bases_not_stored

/**
 * Short name of filter_bases_not_stored
 * @param value Short name of filter_bases_not_stored
 */
def filterNoBases_=(value: Boolean) { this.filter_bases_not_stored = value }

override def freezeFieldValues() {
super.freezeFieldValues()
if (genotypes != null)
  genotypesIndex :+= new File(genotypes.getPath + ".idx")
if (popfile != null)
  popfileIndex :+= new File(popfile.getPath + ".idx")
}

override def commandLine = super.commandLine + optional(TaggedFile.formatCommandLineParameter("-genotypes", genotypes), genotypes, spaceSeparated=true, escape=true, format="%s") + required(TaggedFile.formatCommandLineParameter("-pf", popfile), popfile, spaceSeparated=true, escape=true, format="%s") + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + optional("--min_qscore", min_qscore, spaceSeparated=true, escape=true, format="%s") + optional("--min_mapq", min_mapq, spaceSeparated=true, escape=true, format="%s") + optional("--trim_fraction", trim_fraction, spaceSeparated=true, escape=true, format=trim_fractionFormat) + optional("--beta_threshold", beta_threshold, spaceSeparated=true, escape=true, format=beta_thresholdFormat) + repeat("-llc", lane_level_contamination, spaceSeparated=true, escape=true, format="%s") + optional("-sn", sample_name, spaceSeparated=true, escape=true, format="%s") + optional("-pc", precision, spaceSeparated=true, escape=true, format=precisionFormat) + optional("-br", base_report, spaceSeparated=true, escape=true, format="%s") + optional("-lf", likelihood_file, spaceSeparated=true, escape=true, format="%s") + conditional(verify_sample, "-vs", escape=true, format="%s") + optional("-mbc", minimum_base_count, spaceSeparated=true, escape=true, format="%s") + optional("-population", population, spaceSeparated=true, escape=true, format="%s") + optional("-gm", genotype_mode, spaceSeparated=true, escape=true, format="%s") + optional("--trim_interval", trim_interval, spaceSeparated=true, escape=true, format=trim_intervalFormat) + optional("--min_site_depth", min_site_depth, spaceSeparated=true, escape=true, format="%s") + optional("--fixed_epsilon_qscore", fixed_epsilon_qscore, spaceSeparated=true, escape=true, format="%s") + optional("--min_genotype_depth", min_genotype_depth, spaceSeparated=true, escape=true, format="%s") + optional("--min_genotype_ratio", min_genotype_ratio, spaceSeparated=true, escape=true, format=min_genotype_ratioFormat) + optional("--min_genotype_llh", min_genotype_llh, spaceSeparated=true, escape=true, format=min_genotype_llhFormat) + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
