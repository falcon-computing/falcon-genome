package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Output

class QualifyMissingIntervals extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "QualifyMissingIntervals"
analysis_type = "QualifyMissingIntervals"
scatterClass = classOf[IntervalScatterFunction]
setupScatterFunction = { case scatter: GATKScatterFunction => scatter.includeUnmapped = false }

/** An output file created by the walker.  Will overwrite contents if file exists */
@Output(fullName="out", shortName="o", doc="An output file created by the walker.  Will overwrite contents if file exists", required=false, exclusiveOf="", validation="")
@Gather(classOf[org.broadinstitute.gatk.utils.report.GATKReportGatherer])
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
@Argument(fullName="targetsfile", shortName="targets", doc="Undocumented option", required=true, exclusiveOf="", validation="")
var targetsfile: String = _

/**
 * Short name of targetsfile
 * @return Short name of targetsfile
 */
def targets = this.targetsfile

/**
 * Short name of targetsfile
 * @param value Short name of targetsfile
 */
def targets_=(value: String) { this.targetsfile = value }

/** Undocumented option */
@Argument(fullName="baitsfile", shortName="baits", doc="Undocumented option", required=false, exclusiveOf="", validation="")
var baitsfile: String = _

/**
 * Short name of baitsfile
 * @return Short name of baitsfile
 */
def baits = this.baitsfile

/**
 * Short name of baitsfile
 * @param value Short name of baitsfile
 */
def baits_=(value: String) { this.baitsfile = value }

/** upper and lower bound for an interval to be considered high/low GC content */
@Argument(fullName="gcthreshold", shortName="gc", doc="upper and lower bound for an interval to be considered high/low GC content", required=false, exclusiveOf="", validation="")
var gcthreshold: Option[Double] = None

/**
 * Short name of gcthreshold
 * @return Short name of gcthreshold
 */
def gc = this.gcthreshold

/**
 * Short name of gcthreshold
 * @param value Short name of gcthreshold
 */
def gc_=(value: Option[Double]) { this.gcthreshold = value }

/** Format string for gcthreshold */
@Argument(fullName="gcthresholdFormat", shortName="", doc="Format string for gcthreshold", required=false, exclusiveOf="", validation="")
var gcthresholdFormat: String = "%s"

/** minimum coverage to be considered sequenceable */
@Argument(fullName="coveragethreshold", shortName="cov", doc="minimum coverage to be considered sequenceable", required=false, exclusiveOf="", validation="")
var coveragethreshold: Option[Int] = None

/**
 * Short name of coveragethreshold
 * @return Short name of coveragethreshold
 */
def cov = this.coveragethreshold

/**
 * Short name of coveragethreshold
 * @param value Short name of coveragethreshold
 */
def cov_=(value: Option[Int]) { this.coveragethreshold = value }

/** minimum mapping quality for it to be considered usable */
@Argument(fullName="mappingthreshold", shortName="mmq", doc="minimum mapping quality for it to be considered usable", required=false, exclusiveOf="", validation="")
var mappingthreshold: Option[Byte] = None

/**
 * Short name of mappingthreshold
 * @return Short name of mappingthreshold
 */
def mmq = this.mappingthreshold

/**
 * Short name of mappingthreshold
 * @param value Short name of mappingthreshold
 */
def mmq_=(value: Option[Byte]) { this.mappingthreshold = value }

/** minimum base quality for it to be considered usable */
@Argument(fullName="qualthreshold", shortName="mbq", doc="minimum base quality for it to be considered usable", required=false, exclusiveOf="", validation="")
var qualthreshold: Option[Byte] = None

/**
 * Short name of qualthreshold
 * @return Short name of qualthreshold
 */
def mbq = this.qualthreshold

/**
 * Short name of qualthreshold
 * @param value Short name of qualthreshold
 */
def mbq_=(value: Option[Byte]) { this.qualthreshold = value }

/** minimum interval length to be considered */
@Argument(fullName="intervalsizethreshold", shortName="size", doc="minimum interval length to be considered", required=false, exclusiveOf="", validation="")
var intervalsizethreshold: Option[Byte] = None

/**
 * Short name of intervalsizethreshold
 * @return Short name of intervalsizethreshold
 */
def size = this.intervalsizethreshold

/**
 * Short name of intervalsizethreshold
 * @param value Short name of intervalsizethreshold
 */
def size_=(value: Option[Byte]) { this.intervalsizethreshold = value }

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

override def commandLine = super.commandLine + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + required("-targets", targetsfile, spaceSeparated=true, escape=true, format="%s") + optional("-baits", baitsfile, spaceSeparated=true, escape=true, format="%s") + optional("-gc", gcthreshold, spaceSeparated=true, escape=true, format=gcthresholdFormat) + optional("-cov", coveragethreshold, spaceSeparated=true, escape=true, format="%s") + optional("-mmq", mappingthreshold, spaceSeparated=true, escape=true, format="%s") + optional("-mbq", qualthreshold, spaceSeparated=true, escape=true, format="%s") + optional("-size", intervalsizethreshold, spaceSeparated=true, escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
