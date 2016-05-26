package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Output

class CallableLoci extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "CallableLoci"
analysis_type = "CallableLoci"
scatterClass = classOf[LocusScatterFunction]
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

/** Name of file for output summary */
@Output(fullName="summary", shortName="summary", doc="Name of file for output summary", required=true, exclusiveOf="", validation="")
@Gather(classOf[org.broadinstitute.gatk.queue.function.scattergather.SimpleTextGatherFunction])
var summary: File = _

/** Maximum value for MAPQ to be considered a problematic mapped read. */
@Argument(fullName="maxLowMAPQ", shortName="mlmq", doc="Maximum value for MAPQ to be considered a problematic mapped read.", required=false, exclusiveOf="", validation="")
var maxLowMAPQ: Option[Byte] = None

/**
 * Short name of maxLowMAPQ
 * @return Short name of maxLowMAPQ
 */
def mlmq = this.maxLowMAPQ

/**
 * Short name of maxLowMAPQ
 * @param value Short name of maxLowMAPQ
 */
def mlmq_=(value: Option[Byte]) { this.maxLowMAPQ = value }

/** Minimum mapping quality of reads to count towards depth. */
@Argument(fullName="minMappingQuality", shortName="mmq", doc="Minimum mapping quality of reads to count towards depth.", required=false, exclusiveOf="", validation="")
var minMappingQuality: Option[Byte] = None

/**
 * Short name of minMappingQuality
 * @return Short name of minMappingQuality
 */
def mmq = this.minMappingQuality

/**
 * Short name of minMappingQuality
 * @param value Short name of minMappingQuality
 */
def mmq_=(value: Option[Byte]) { this.minMappingQuality = value }

/** Minimum quality of bases to count towards depth. */
@Argument(fullName="minBaseQuality", shortName="mbq", doc="Minimum quality of bases to count towards depth.", required=false, exclusiveOf="", validation="")
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

/** Minimum QC+ read depth before a locus is considered callable */
@Argument(fullName="minDepth", shortName="minDepth", doc="Minimum QC+ read depth before a locus is considered callable", required=false, exclusiveOf="", validation="")
var minDepth: Option[Int] = None

/** Maximum read depth before a locus is considered poorly mapped */
@Argument(fullName="maxDepth", shortName="maxDepth", doc="Maximum read depth before a locus is considered poorly mapped", required=false, exclusiveOf="", validation="")
var maxDepth: Option[Int] = None

/** Minimum read depth before a locus is considered a potential candidate for poorly mapped */
@Argument(fullName="minDepthForLowMAPQ", shortName="mdflmq", doc="Minimum read depth before a locus is considered a potential candidate for poorly mapped", required=false, exclusiveOf="", validation="")
var minDepthForLowMAPQ: Option[Int] = None

/**
 * Short name of minDepthForLowMAPQ
 * @return Short name of minDepthForLowMAPQ
 */
def mdflmq = this.minDepthForLowMAPQ

/**
 * Short name of minDepthForLowMAPQ
 * @param value Short name of minDepthForLowMAPQ
 */
def mdflmq_=(value: Option[Int]) { this.minDepthForLowMAPQ = value }

/** If the fraction of reads at a base with low mapping quality exceeds this value, the site may be poorly mapped */
@Argument(fullName="maxFractionOfReadsWithLowMAPQ", shortName="frlmq", doc="If the fraction of reads at a base with low mapping quality exceeds this value, the site may be poorly mapped", required=false, exclusiveOf="", validation="")
var maxFractionOfReadsWithLowMAPQ: Option[Double] = None

/**
 * Short name of maxFractionOfReadsWithLowMAPQ
 * @return Short name of maxFractionOfReadsWithLowMAPQ
 */
def frlmq = this.maxFractionOfReadsWithLowMAPQ

/**
 * Short name of maxFractionOfReadsWithLowMAPQ
 * @param value Short name of maxFractionOfReadsWithLowMAPQ
 */
def frlmq_=(value: Option[Double]) { this.maxFractionOfReadsWithLowMAPQ = value }

/** Format string for maxFractionOfReadsWithLowMAPQ */
@Argument(fullName="maxFractionOfReadsWithLowMAPQFormat", shortName="", doc="Format string for maxFractionOfReadsWithLowMAPQ", required=false, exclusiveOf="", validation="")
var maxFractionOfReadsWithLowMAPQFormat: String = "%s"

/** Output format */
@Argument(fullName="format", shortName="format", doc="Output format", required=false, exclusiveOf="", validation="")
var format: org.broadinstitute.gatk.tools.walkers.coverage.CallableLoci.OutputFormat = _

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

override def commandLine = super.commandLine + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + required("-summary", summary, spaceSeparated=true, escape=true, format="%s") + optional("-mlmq", maxLowMAPQ, spaceSeparated=true, escape=true, format="%s") + optional("-mmq", minMappingQuality, spaceSeparated=true, escape=true, format="%s") + optional("-mbq", minBaseQuality, spaceSeparated=true, escape=true, format="%s") + optional("-minDepth", minDepth, spaceSeparated=true, escape=true, format="%s") + optional("-maxDepth", maxDepth, spaceSeparated=true, escape=true, format="%s") + optional("-mdflmq", minDepthForLowMAPQ, spaceSeparated=true, escape=true, format="%s") + optional("-frlmq", maxFractionOfReadsWithLowMAPQ, spaceSeparated=true, escape=true, format=maxFractionOfReadsWithLowMAPQFormat) + optional("-format", format, spaceSeparated=true, escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
