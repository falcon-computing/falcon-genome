package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Output

class DepthOfCoverage extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK {
analysisName = "DepthOfCoverage"
analysis_type = "DepthOfCoverage"

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

/** Minimum mapping quality of reads to count towards depth */
@Argument(fullName="minMappingQuality", shortName="mmq", doc="Minimum mapping quality of reads to count towards depth", required=false, exclusiveOf="", validation="")
var minMappingQuality: Option[Int] = None

/**
 * Short name of minMappingQuality
 * @return Short name of minMappingQuality
 */
def mmq = this.minMappingQuality

/**
 * Short name of minMappingQuality
 * @param value Short name of minMappingQuality
 */
def mmq_=(value: Option[Int]) { this.minMappingQuality = value }

/** Maximum mapping quality of reads to count towards depth */
@Argument(fullName="maxMappingQuality", shortName="", doc="Maximum mapping quality of reads to count towards depth", required=false, exclusiveOf="", validation="")
var maxMappingQuality: Option[Int] = None

/** Minimum quality of bases to count towards depth */
@Argument(fullName="minBaseQuality", shortName="mbq", doc="Minimum quality of bases to count towards depth", required=false, exclusiveOf="", validation="")
var minBaseQuality: Option[Byte] = None

/**
 * Short name of minBaseQuality
 * @return Short name of minBaseQuality
 */
def mbq = this.minBaseQuality

/**
 * Short name of minBaseQuality
 * @param value Short name of minBaseQuality
 */
def mbq_=(value: Option[Byte]) { this.minBaseQuality = value }

/** Maximum quality of bases to count towards depth */
@Argument(fullName="maxBaseQuality", shortName="", doc="Maximum quality of bases to count towards depth", required=false, exclusiveOf="", validation="")
var maxBaseQuality: Option[Byte] = None

/** How should overlapping reads from the same fragment be handled? */
@Argument(fullName="countType", shortName="", doc="How should overlapping reads from the same fragment be handled?", required=false, exclusiveOf="", validation="")
var countType: org.broadinstitute.gatk.tools.walkers.coverage.CoverageUtils.CountPileupType = _

/** Add base counts to per-locus output */
@Argument(fullName="printBaseCounts", shortName="baseCounts", doc="Add base counts to per-locus output", required=false, exclusiveOf="", validation="")
var printBaseCounts: Boolean = _

/**
 * Short name of printBaseCounts
 * @return Short name of printBaseCounts
 */
def baseCounts = this.printBaseCounts

/**
 * Short name of printBaseCounts
 * @param value Short name of printBaseCounts
 */
def baseCounts_=(value: Boolean) { this.printBaseCounts = value }

/** Do not calculate per-sample per-depth counts of loci */
@Argument(fullName="omitLocusTable", shortName="omitLocusTable", doc="Do not calculate per-sample per-depth counts of loci", required=false, exclusiveOf="", validation="")
var omitLocusTable: Boolean = _

/** Do not calculate per-interval statistics */
@Argument(fullName="omitIntervalStatistics", shortName="omitIntervals", doc="Do not calculate per-interval statistics", required=false, exclusiveOf="", validation="")
var omitIntervalStatistics: Boolean = _

/**
 * Short name of omitIntervalStatistics
 * @return Short name of omitIntervalStatistics
 */
def omitIntervals = this.omitIntervalStatistics

/**
 * Short name of omitIntervalStatistics
 * @param value Short name of omitIntervalStatistics
 */
def omitIntervals_=(value: Boolean) { this.omitIntervalStatistics = value }

/** Do not output depth of coverage at each base */
@Argument(fullName="omitDepthOutputAtEachBase", shortName="omitBaseOutput", doc="Do not output depth of coverage at each base", required=false, exclusiveOf="", validation="")
var omitDepthOutputAtEachBase: Boolean = _

/**
 * Short name of omitDepthOutputAtEachBase
 * @return Short name of omitDepthOutputAtEachBase
 */
def omitBaseOutput = this.omitDepthOutputAtEachBase

/**
 * Short name of omitDepthOutputAtEachBase
 * @param value Short name of omitDepthOutputAtEachBase
 */
def omitBaseOutput_=(value: Boolean) { this.omitDepthOutputAtEachBase = value }

/** Calculate coverage statistics over this list of genes */
@Argument(fullName="calculateCoverageOverGenes", shortName="geneList", doc="Calculate coverage statistics over this list of genes", required=false, exclusiveOf="", validation="")
var calculateCoverageOverGenes: File = _

/**
 * Short name of calculateCoverageOverGenes
 * @return Short name of calculateCoverageOverGenes
 */
def geneList = this.calculateCoverageOverGenes

/**
 * Short name of calculateCoverageOverGenes
 * @param value Short name of calculateCoverageOverGenes
 */
def geneList_=(value: File) { this.calculateCoverageOverGenes = value }

/** The format of the output file */
@Argument(fullName="outputFormat", shortName="", doc="The format of the output file", required=false, exclusiveOf="", validation="")
var outputFormat: String = _

/** Include sites where the reference is N */
@Argument(fullName="includeRefNSites", shortName="", doc="Include sites where the reference is N", required=false, exclusiveOf="", validation="")
var includeRefNSites: Boolean = _

/** Print the bin values and exit immediately */
@Argument(fullName="printBinEndpointsAndExit", shortName="", doc="Print the bin values and exit immediately", required=false, exclusiveOf="", validation="")
var printBinEndpointsAndExit: Boolean = _

/** Starting (left endpoint) for granular binning */
@Argument(fullName="start", shortName="", doc="Starting (left endpoint) for granular binning", required=false, exclusiveOf="", validation="")
var start: Option[Int] = None

/** Ending (right endpoint) for granular binning */
@Argument(fullName="stop", shortName="", doc="Ending (right endpoint) for granular binning", required=false, exclusiveOf="", validation="")
var stop: Option[Int] = None

/** Number of bins to use for granular binning */
@Argument(fullName="nBins", shortName="", doc="Number of bins to use for granular binning", required=false, exclusiveOf="", validation="")
var nBins: Option[Int] = None

/** Do not output the summary files per-sample */
@Argument(fullName="omitPerSampleStats", shortName="omitSampleSummary", doc="Do not output the summary files per-sample", required=false, exclusiveOf="", validation="")
var omitPerSampleStats: Boolean = _

/**
 * Short name of omitPerSampleStats
 * @return Short name of omitPerSampleStats
 */
def omitSampleSummary = this.omitPerSampleStats

/**
 * Short name of omitPerSampleStats
 * @param value Short name of omitPerSampleStats
 */
def omitSampleSummary_=(value: Boolean) { this.omitPerSampleStats = value }

/** Partition type for depth of coverage */
@Argument(fullName="partitionType", shortName="pt", doc="Partition type for depth of coverage", required=false, exclusiveOf="", validation="")
var partitionType: Seq[org.broadinstitute.gatk.tools.walkers.coverage.DoCOutputType.Partition] = Nil

/**
 * Short name of partitionType
 * @return Short name of partitionType
 */
def pt = this.partitionType

/**
 * Short name of partitionType
 * @param value Short name of partitionType
 */
def pt_=(value: Seq[org.broadinstitute.gatk.tools.walkers.coverage.DoCOutputType.Partition]) { this.partitionType = value }

/** Include information on deletions */
@Argument(fullName="includeDeletions", shortName="dels", doc="Include information on deletions", required=false, exclusiveOf="", validation="")
var includeDeletions: Boolean = _

/**
 * Short name of includeDeletions
 * @return Short name of includeDeletions
 */
def dels = this.includeDeletions

/**
 * Short name of includeDeletions
 * @param value Short name of includeDeletions
 */
def dels_=(value: Boolean) { this.includeDeletions = value }

/** Ignore sites consisting only of deletions */
@Argument(fullName="ignoreDeletionSites", shortName="", doc="Ignore sites consisting only of deletions", required=false, exclusiveOf="", validation="")
var ignoreDeletionSites: Boolean = _

/** Coverage threshold (in percent) for summarizing statistics */
@Argument(fullName="summaryCoverageThreshold", shortName="ct", doc="Coverage threshold (in percent) for summarizing statistics", required=false, exclusiveOf="", validation="")
var summaryCoverageThreshold: Seq[String] = Nil

/**
 * Short name of summaryCoverageThreshold
 * @return Short name of summaryCoverageThreshold
 */
def ct = this.summaryCoverageThreshold

/**
 * Short name of summaryCoverageThreshold
 * @param value Short name of summaryCoverageThreshold
 */
def ct_=(value: Seq[String]) { this.summaryCoverageThreshold = value }

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

override def commandLine = super.commandLine + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + optional("-mmq", minMappingQuality, spaceSeparated=true, escape=true, format="%s") + optional("--maxMappingQuality", maxMappingQuality, spaceSeparated=true, escape=true, format="%s") + optional("-mbq", minBaseQuality, spaceSeparated=true, escape=true, format="%s") + optional("--maxBaseQuality", maxBaseQuality, spaceSeparated=true, escape=true, format="%s") + optional("--countType", countType, spaceSeparated=true, escape=true, format="%s") + conditional(printBaseCounts, "-baseCounts", escape=true, format="%s") + conditional(omitLocusTable, "-omitLocusTable", escape=true, format="%s") + conditional(omitIntervalStatistics, "-omitIntervals", escape=true, format="%s") + conditional(omitDepthOutputAtEachBase, "-omitBaseOutput", escape=true, format="%s") + optional("-geneList", calculateCoverageOverGenes, spaceSeparated=true, escape=true, format="%s") + optional("--outputFormat", outputFormat, spaceSeparated=true, escape=true, format="%s") + conditional(includeRefNSites, "--includeRefNSites", escape=true, format="%s") + conditional(printBinEndpointsAndExit, "--printBinEndpointsAndExit", escape=true, format="%s") + optional("--start", start, spaceSeparated=true, escape=true, format="%s") + optional("--stop", stop, spaceSeparated=true, escape=true, format="%s") + optional("--nBins", nBins, spaceSeparated=true, escape=true, format="%s") + conditional(omitPerSampleStats, "-omitSampleSummary", escape=true, format="%s") + repeat("-pt", partitionType, spaceSeparated=true, escape=true, format="%s") + conditional(includeDeletions, "-dels", escape=true, format="%s") + conditional(ignoreDeletionSites, "--ignoreDeletionSites", escape=true, format="%s") + repeat("-ct", summaryCoverageThreshold, spaceSeparated=true, escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
