package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class FindCoveredIntervals extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "FindCoveredIntervals"
analysis_type = "FindCoveredIntervals"
scatterClass = classOf[ContigScatterFunction]
setupScatterFunction = { case scatter: GATKScatterFunction => scatter.includeUnmapped = false }

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

/** output intervals that fail the coverage threshold instead */
@Argument(fullName="uncovered", shortName="u", doc="output intervals that fail the coverage threshold instead", required=false, exclusiveOf="", validation="")
var uncovered: Boolean = _

/**
 * Short name of uncovered
 * @return Short name of uncovered
 */
def u = this.uncovered

/**
 * Short name of uncovered
 * @param value Short name of uncovered
 */
def u_=(value: Boolean) { this.uncovered = value }

/** The minimum allowable coverage to be considered covered */
@Argument(fullName="coverage_threshold", shortName="cov", doc="The minimum allowable coverage to be considered covered", required=false, exclusiveOf="", validation="")
var coverage_threshold: Option[Int] = None

/**
 * Short name of coverage_threshold
 * @return Short name of coverage_threshold
 */
def cov = this.coverage_threshold

/**
 * Short name of coverage_threshold
 * @param value Short name of coverage_threshold
 */
def cov_=(value: Option[Int]) { this.coverage_threshold = value }

/** The minimum allowable base quality score to be counted for coverage */
@Argument(fullName="minBaseQuality", shortName="minBQ", doc="The minimum allowable base quality score to be counted for coverage", required=false, exclusiveOf="", validation="")
var minBaseQuality: Option[Int] = None

/**
 * Short name of minBaseQuality
 * @return Short name of minBaseQuality
 */
def minBQ = this.minBaseQuality

/**
 * Short name of minBaseQuality
 * @param value Short name of minBaseQuality
 */
def minBQ_=(value: Option[Int]) { this.minBaseQuality = value }

/** The minimum allowable mapping quality score to be counted for coverage */
@Argument(fullName="minMappingQuality", shortName="minMQ", doc="The minimum allowable mapping quality score to be counted for coverage", required=false, exclusiveOf="", validation="")
var minMappingQuality: Option[Int] = None

/**
 * Short name of minMappingQuality
 * @return Short name of minMappingQuality
 */
def minMQ = this.minMappingQuality

/**
 * Short name of minMappingQuality
 * @param value Short name of minMappingQuality
 */
def minMQ_=(value: Option[Int]) { this.minMappingQuality = value }

/** Output the raw activity profile results in IGV format */
@Output(fullName="activityProfileOut", shortName="APO", doc="Output the raw activity profile results in IGV format", required=false, exclusiveOf="", validation="")
@Gather(classOf[org.broadinstitute.gatk.queue.function.scattergather.SimpleTextGatherFunction])
var activityProfileOut: File = _

/**
 * Short name of activityProfileOut
 * @return Short name of activityProfileOut
 */
def APO = this.activityProfileOut

/**
 * Short name of activityProfileOut
 * @param value Short name of activityProfileOut
 */
def APO_=(value: File) { this.activityProfileOut = value }

/** Output the active region to this IGV formatted file */
@Output(fullName="activeRegionOut", shortName="ARO", doc="Output the active region to this IGV formatted file", required=false, exclusiveOf="", validation="")
@Gather(classOf[org.broadinstitute.gatk.queue.function.scattergather.SimpleTextGatherFunction])
var activeRegionOut: File = _

/**
 * Short name of activeRegionOut
 * @return Short name of activeRegionOut
 */
def ARO = this.activeRegionOut

/**
 * Short name of activeRegionOut
 * @param value Short name of activeRegionOut
 */
def ARO_=(value: File) { this.activeRegionOut = value }

/** Use this interval list file as the active regions to process */
@Input(fullName="activeRegionIn", shortName="AR", doc="Use this interval list file as the active regions to process", required=false, exclusiveOf="", validation="")
var activeRegionIn: Seq[File] = Nil

/**
 * Short name of activeRegionIn
 * @return Short name of activeRegionIn
 */
def AR = this.activeRegionIn

/**
 * Short name of activeRegionIn
 * @param value Short name of activeRegionIn
 */
def AR_=(value: Seq[File]) { this.activeRegionIn = value }

/** The active region extension; if not provided defaults to Walker annotated default */
@Argument(fullName="activeRegionExtension", shortName="activeRegionExtension", doc="The active region extension; if not provided defaults to Walker annotated default", required=false, exclusiveOf="", validation="")
var activeRegionExtension: Option[Int] = None

/** If provided, all bases will be tagged as active */
@Argument(fullName="forceActive", shortName="forceActive", doc="If provided, all bases will be tagged as active", required=false, exclusiveOf="", validation="")
var forceActive: Boolean = _

/** The active region maximum size; if not provided defaults to Walker annotated default */
@Argument(fullName="activeRegionMaxSize", shortName="activeRegionMaxSize", doc="The active region maximum size; if not provided defaults to Walker annotated default", required=false, exclusiveOf="", validation="")
var activeRegionMaxSize: Option[Int] = None

/** The sigma of the band pass filter Gaussian kernel; if not provided defaults to Walker annotated default */
@Argument(fullName="bandPassSigma", shortName="bandPassSigma", doc="The sigma of the band pass filter Gaussian kernel; if not provided defaults to Walker annotated default", required=false, exclusiveOf="", validation="")
var bandPassSigma: Option[Double] = None

/** Format string for bandPassSigma */
@Argument(fullName="bandPassSigmaFormat", shortName="", doc="Format string for bandPassSigma", required=false, exclusiveOf="", validation="")
var bandPassSigmaFormat: String = "%s"

/** Region probability propagation distance beyond it's maximum size. */
@Argument(fullName="maxProbPropagationDistance", shortName="maxProbPropDist", doc="Region probability propagation distance beyond it's maximum size.", required=false, exclusiveOf="", validation="")
var maxProbPropagationDistance: Option[Int] = None

/**
 * Short name of maxProbPropagationDistance
 * @return Short name of maxProbPropagationDistance
 */
def maxProbPropDist = this.maxProbPropagationDistance

/**
 * Short name of maxProbPropagationDistance
 * @param value Short name of maxProbPropagationDistance
 */
def maxProbPropDist_=(value: Option[Int]) { this.maxProbPropagationDistance = value }

/** Threshold for the probability of a profile state being active. */
@Argument(fullName="activeProbabilityThreshold", shortName="ActProbThresh", doc="Threshold for the probability of a profile state being active.", required=false, exclusiveOf="", validation="")
var activeProbabilityThreshold: Option[Double] = None

/**
 * Short name of activeProbabilityThreshold
 * @return Short name of activeProbabilityThreshold
 */
def ActProbThresh = this.activeProbabilityThreshold

/**
 * Short name of activeProbabilityThreshold
 * @param value Short name of activeProbabilityThreshold
 */
def ActProbThresh_=(value: Option[Double]) { this.activeProbabilityThreshold = value }

/** Format string for activeProbabilityThreshold */
@Argument(fullName="activeProbabilityThresholdFormat", shortName="", doc="Format string for activeProbabilityThreshold", required=false, exclusiveOf="", validation="")
var activeProbabilityThresholdFormat: String = "%s"

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

override def commandLine = super.commandLine + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + conditional(uncovered, "-u", escape=true, format="%s") + optional("-cov", coverage_threshold, spaceSeparated=true, escape=true, format="%s") + optional("-minBQ", minBaseQuality, spaceSeparated=true, escape=true, format="%s") + optional("-minMQ", minMappingQuality, spaceSeparated=true, escape=true, format="%s") + optional("-APO", activityProfileOut, spaceSeparated=true, escape=true, format="%s") + optional("-ARO", activeRegionOut, spaceSeparated=true, escape=true, format="%s") + repeat("-AR", activeRegionIn, spaceSeparated=true, escape=true, format="%s") + optional("-activeRegionExtension", activeRegionExtension, spaceSeparated=true, escape=true, format="%s") + conditional(forceActive, "-forceActive", escape=true, format="%s") + optional("-activeRegionMaxSize", activeRegionMaxSize, spaceSeparated=true, escape=true, format="%s") + optional("-bandPassSigma", bandPassSigma, spaceSeparated=true, escape=true, format=bandPassSigmaFormat) + optional("-maxProbPropDist", maxProbPropagationDistance, spaceSeparated=true, escape=true, format="%s") + optional("-ActProbThresh", activeProbabilityThreshold, spaceSeparated=true, escape=true, format=activeProbabilityThresholdFormat) + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
