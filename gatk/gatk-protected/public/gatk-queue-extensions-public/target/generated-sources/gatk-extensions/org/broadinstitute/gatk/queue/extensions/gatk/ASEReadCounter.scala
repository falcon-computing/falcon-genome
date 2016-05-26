package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class ASEReadCounter extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "ASEReadCounter"
analysis_type = "ASEReadCounter"
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

/** Undocumented option */
@Input(fullName="sitesVCFFile", shortName="sites", doc="Undocumented option", required=true, exclusiveOf="", validation="")
var sitesVCFFile: File = _

/**
 * Short name of sitesVCFFile
 * @return Short name of sitesVCFFile
 */
def sites = this.sitesVCFFile

/**
 * Short name of sitesVCFFile
 * @param value Short name of sitesVCFFile
 */
def sites_=(value: File) { this.sitesVCFFile = value }

/** Dependencies on the index of sitesVCFFile */
@Input(fullName="sitesVCFFileIndex", shortName="", doc="Dependencies on the index of sitesVCFFile", required=false, exclusiveOf="", validation="")
private var sitesVCFFileIndex: Seq[File] = Nil

/** Minimum number of bases that pass filters */
@Argument(fullName="minDepthOfNonFilteredBase", shortName="minDepth", doc="Minimum number of bases that pass filters", required=false, exclusiveOf="", validation="")
var minDepthOfNonFilteredBase: Option[Int] = None

/**
 * Short name of minDepthOfNonFilteredBase
 * @return Short name of minDepthOfNonFilteredBase
 */
def minDepth = this.minDepthOfNonFilteredBase

/**
 * Short name of minDepthOfNonFilteredBase
 * @param value Short name of minDepthOfNonFilteredBase
 */
def minDepth_=(value: Option[Int]) { this.minDepthOfNonFilteredBase = value }

/** Minimum read mapping quality */
@Argument(fullName="minMappingQuality", shortName="mmq", doc="Minimum read mapping quality", required=false, exclusiveOf="", validation="")
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

/** Minimum base quality */
@Argument(fullName="minBaseQuality", shortName="mbq", doc="Minimum base quality", required=false, exclusiveOf="", validation="")
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

/** Handling of overlapping reads from the same fragment */
@Argument(fullName="countOverlapReadsType", shortName="overlap", doc="Handling of overlapping reads from the same fragment", required=false, exclusiveOf="", validation="")
var countOverlapReadsType: org.broadinstitute.gatk.tools.walkers.coverage.CoverageUtils.CountPileupType = _

/**
 * Short name of countOverlapReadsType
 * @return Short name of countOverlapReadsType
 */
def overlap = this.countOverlapReadsType

/**
 * Short name of countOverlapReadsType
 * @param value Short name of countOverlapReadsType
 */
def overlap_=(value: org.broadinstitute.gatk.tools.walkers.coverage.CoverageUtils.CountPileupType) { this.countOverlapReadsType = value }

/** Format of the output file, can be CSV, TABLE, RTABLE */
@Argument(fullName="outputFormat", shortName="", doc="Format of the output file, can be CSV, TABLE, RTABLE", required=false, exclusiveOf="", validation="")
var outputFormat: org.broadinstitute.gatk.tools.walkers.rnaseq.ASEReadCounter.OUTPUT_FORMAT = _

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
if (sitesVCFFile != null)
  sitesVCFFileIndex :+= new File(sitesVCFFile.getPath + ".idx")
}

override def commandLine = super.commandLine + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + required(TaggedFile.formatCommandLineParameter("-sites", sitesVCFFile), sitesVCFFile, spaceSeparated=true, escape=true, format="%s") + optional("-minDepth", minDepthOfNonFilteredBase, spaceSeparated=true, escape=true, format="%s") + optional("-mmq", minMappingQuality, spaceSeparated=true, escape=true, format="%s") + optional("-mbq", minBaseQuality, spaceSeparated=true, escape=true, format="%s") + optional("-overlap", countOverlapReadsType, spaceSeparated=true, escape=true, format="%s") + optional("--outputFormat", outputFormat, spaceSeparated=true, escape=true, format="%s") + conditional(includeDeletions, "-dels", escape=true, format="%s") + conditional(ignoreDeletionSites, "--ignoreDeletionSites", escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
