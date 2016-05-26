package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Output

class DiagnoseTargets extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "DiagnoseTargets"
analysis_type = "DiagnoseTargets"
scatterClass = classOf[IntervalScatterFunction]
setupScatterFunction = { case scatter: GATKScatterFunction => scatter.includeUnmapped = false }

/** File to which interval statistics should be written */
@Output(fullName="out", shortName="o", doc="File to which interval statistics should be written", required=false, exclusiveOf="", validation="")
@Gather(classOf[CatVariantsGatherer])
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

/** Automatically generated index for out */
@Output(fullName="outIndex", shortName="", doc="Automatically generated index for out", required=false, exclusiveOf="", validation="")
@Gather(enabled=false)
private var outIndex: File = _

/** The minimum Base Quality that is considered for calls */
@Argument(fullName="minimum_base_quality", shortName="BQ", doc="The minimum Base Quality that is considered for calls", required=false, exclusiveOf="", validation="")
var minimum_base_quality: Option[Int] = None

/**
 * Short name of minimum_base_quality
 * @return Short name of minimum_base_quality
 */
def BQ = this.minimum_base_quality

/**
 * Short name of minimum_base_quality
 * @param value Short name of minimum_base_quality
 */
def BQ_=(value: Option[Int]) { this.minimum_base_quality = value }

/** The minimum read mapping quality considered for calls */
@Argument(fullName="minimum_mapping_quality", shortName="MQ", doc="The minimum read mapping quality considered for calls", required=false, exclusiveOf="", validation="")
var minimum_mapping_quality: Option[Int] = None

/**
 * Short name of minimum_mapping_quality
 * @return Short name of minimum_mapping_quality
 */
def MQ = this.minimum_mapping_quality

/**
 * Short name of minimum_mapping_quality
 * @param value Short name of minimum_mapping_quality
 */
def MQ_=(value: Option[Int]) { this.minimum_mapping_quality = value }

/** The minimum allowable coverage, used for calling LOW_COVERAGE */
@Argument(fullName="minimum_coverage", shortName="min", doc="The minimum allowable coverage, used for calling LOW_COVERAGE", required=false, exclusiveOf="", validation="")
var minimum_coverage: Option[Int] = None

/**
 * Short name of minimum_coverage
 * @return Short name of minimum_coverage
 */
def min = this.minimum_coverage

/**
 * Short name of minimum_coverage
 * @param value Short name of minimum_coverage
 */
def min_=(value: Option[Int]) { this.minimum_coverage = value }

/** The maximum allowable coverage, used for calling EXCESSIVE_COVERAGE */
@Argument(fullName="maximum_coverage", shortName="max", doc="The maximum allowable coverage, used for calling EXCESSIVE_COVERAGE", required=false, exclusiveOf="", validation="")
var maximum_coverage: Option[Int] = None

/**
 * Short name of maximum_coverage
 * @return Short name of maximum_coverage
 */
def max = this.maximum_coverage

/**
 * Short name of maximum_coverage
 * @param value Short name of maximum_coverage
 */
def max_=(value: Option[Int]) { this.maximum_coverage = value }

/** The maximum allowed distance between a read and its mate */
@Argument(fullName="maximum_insert_size", shortName="ins", doc="The maximum allowed distance between a read and its mate", required=false, exclusiveOf="", validation="")
var maximum_insert_size: Option[Int] = None

/**
 * Short name of maximum_insert_size
 * @return Short name of maximum_insert_size
 */
def ins = this.maximum_insert_size

/**
 * Short name of maximum_insert_size
 * @param value Short name of maximum_insert_size
 */
def ins_=(value: Option[Int]) { this.maximum_insert_size = value }

/** The needed proportion of samples containing a call for the interval to adopt the call  */
@Argument(fullName="voting_status_threshold", shortName="stV", doc="The needed proportion of samples containing a call for the interval to adopt the call ", required=false, exclusiveOf="", validation="")
var voting_status_threshold: Option[Double] = None

/**
 * Short name of voting_status_threshold
 * @return Short name of voting_status_threshold
 */
def stV = this.voting_status_threshold

/**
 * Short name of voting_status_threshold
 * @param value Short name of voting_status_threshold
 */
def stV_=(value: Option[Double]) { this.voting_status_threshold = value }

/** Format string for voting_status_threshold */
@Argument(fullName="voting_status_thresholdFormat", shortName="", doc="Format string for voting_status_threshold", required=false, exclusiveOf="", validation="")
var voting_status_thresholdFormat: String = "%s"

/** The proportion of the loci needed for calling BAD_MATE */
@Argument(fullName="bad_mate_status_threshold", shortName="stBM", doc="The proportion of the loci needed for calling BAD_MATE", required=false, exclusiveOf="", validation="")
var bad_mate_status_threshold: Option[Double] = None

/**
 * Short name of bad_mate_status_threshold
 * @return Short name of bad_mate_status_threshold
 */
def stBM = this.bad_mate_status_threshold

/**
 * Short name of bad_mate_status_threshold
 * @param value Short name of bad_mate_status_threshold
 */
def stBM_=(value: Option[Double]) { this.bad_mate_status_threshold = value }

/** Format string for bad_mate_status_threshold */
@Argument(fullName="bad_mate_status_thresholdFormat", shortName="", doc="Format string for bad_mate_status_threshold", required=false, exclusiveOf="", validation="")
var bad_mate_status_thresholdFormat: String = "%s"

/** The proportion of the loci needed for calling LOW_COVERAGE and COVERAGE_GAPS */
@Argument(fullName="coverage_status_threshold", shortName="stC", doc="The proportion of the loci needed for calling LOW_COVERAGE and COVERAGE_GAPS", required=false, exclusiveOf="", validation="")
var coverage_status_threshold: Option[Double] = None

/**
 * Short name of coverage_status_threshold
 * @return Short name of coverage_status_threshold
 */
def stC = this.coverage_status_threshold

/**
 * Short name of coverage_status_threshold
 * @param value Short name of coverage_status_threshold
 */
def stC_=(value: Option[Double]) { this.coverage_status_threshold = value }

/** Format string for coverage_status_threshold */
@Argument(fullName="coverage_status_thresholdFormat", shortName="", doc="Format string for coverage_status_threshold", required=false, exclusiveOf="", validation="")
var coverage_status_thresholdFormat: String = "%s"

/** The proportion of the loci needed for calling EXCESSIVE_COVERAGE */
@Argument(fullName="excessive_coverage_status_threshold", shortName="stXC", doc="The proportion of the loci needed for calling EXCESSIVE_COVERAGE", required=false, exclusiveOf="", validation="")
var excessive_coverage_status_threshold: Option[Double] = None

/**
 * Short name of excessive_coverage_status_threshold
 * @return Short name of excessive_coverage_status_threshold
 */
def stXC = this.excessive_coverage_status_threshold

/**
 * Short name of excessive_coverage_status_threshold
 * @param value Short name of excessive_coverage_status_threshold
 */
def stXC_=(value: Option[Double]) { this.excessive_coverage_status_threshold = value }

/** Format string for excessive_coverage_status_threshold */
@Argument(fullName="excessive_coverage_status_thresholdFormat", shortName="", doc="Format string for excessive_coverage_status_threshold", required=false, exclusiveOf="", validation="")
var excessive_coverage_status_thresholdFormat: String = "%s"

/** The proportion of the loci needed for calling POOR_QUALITY */
@Argument(fullName="quality_status_threshold", shortName="stQ", doc="The proportion of the loci needed for calling POOR_QUALITY", required=false, exclusiveOf="", validation="")
var quality_status_threshold: Option[Double] = None

/**
 * Short name of quality_status_threshold
 * @return Short name of quality_status_threshold
 */
def stQ = this.quality_status_threshold

/**
 * Short name of quality_status_threshold
 * @param value Short name of quality_status_threshold
 */
def stQ_=(value: Option[Double]) { this.quality_status_threshold = value }

/** Format string for quality_status_threshold */
@Argument(fullName="quality_status_thresholdFormat", shortName="", doc="Format string for quality_status_threshold", required=false, exclusiveOf="", validation="")
var quality_status_thresholdFormat: String = "%s"

/** Produces a file with the intervals that don't pass filters */
@Output(fullName="missing_intervals", shortName="missing", doc="Produces a file with the intervals that don't pass filters", required=false, exclusiveOf="", validation="")
@Gather(classOf[org.broadinstitute.gatk.queue.function.scattergather.SimpleTextGatherFunction])
var missing_intervals: File = _

/**
 * Short name of missing_intervals
 * @return Short name of missing_intervals
 */
def missing = this.missing_intervals

/**
 * Short name of missing_intervals
 * @param value Short name of missing_intervals
 */
def missing_=(value: File) { this.missing_intervals = value }

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
if (out != null && !org.broadinstitute.gatk.utils.io.IOUtils.isSpecialFile(out))
  if (!org.broadinstitute.gatk.utils.commandline.ArgumentTypeDescriptor.isCompressed(out.getPath))
    outIndex = new File(out.getPath + ".idx")
}

override def commandLine = super.commandLine + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + optional("-BQ", minimum_base_quality, spaceSeparated=true, escape=true, format="%s") + optional("-MQ", minimum_mapping_quality, spaceSeparated=true, escape=true, format="%s") + optional("-min", minimum_coverage, spaceSeparated=true, escape=true, format="%s") + optional("-max", maximum_coverage, spaceSeparated=true, escape=true, format="%s") + optional("-ins", maximum_insert_size, spaceSeparated=true, escape=true, format="%s") + optional("-stV", voting_status_threshold, spaceSeparated=true, escape=true, format=voting_status_thresholdFormat) + optional("-stBM", bad_mate_status_threshold, spaceSeparated=true, escape=true, format=bad_mate_status_thresholdFormat) + optional("-stC", coverage_status_threshold, spaceSeparated=true, escape=true, format=coverage_status_thresholdFormat) + optional("-stXC", excessive_coverage_status_threshold, spaceSeparated=true, escape=true, format=excessive_coverage_status_thresholdFormat) + optional("-stQ", quality_status_threshold, spaceSeparated=true, escape=true, format=quality_status_thresholdFormat) + optional("-missing", missing_intervals, spaceSeparated=true, escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
