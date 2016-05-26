package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class DiffObjects extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "DiffObjects"
analysis_type = "DiffObjects"
scatterClass = classOf[LocusScatterFunction]
setupScatterFunction = { case scatter: GATKScatterFunction => scatter.includeUnmapped = false }

/** File to which results should be written */
@Output(fullName="out", shortName="o", doc="File to which results should be written", required=false, exclusiveOf="", validation="")
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

/** Master file: expected results */
@Input(fullName="master", shortName="m", doc="Master file: expected results", required=true, exclusiveOf="", validation="")
var master: File = _

/**
 * Short name of master
 * @return Short name of master
 */
def m = this.master

/**
 * Short name of master
 * @param value Short name of master
 */
def m_=(value: File) { this.master = value }

/** Test file: new results to compare to the master file */
@Input(fullName="test", shortName="t", doc="Test file: new results to compare to the master file", required=true, exclusiveOf="", validation="")
var test: File = _

/**
 * Short name of test
 * @return Short name of test
 */
def t = this.test

/**
 * Short name of test
 * @param value Short name of test
 */
def t_=(value: File) { this.test = value }

/** Max. number of objects to read from the files.  -1 [default] means unlimited */
@Argument(fullName="maxObjectsToRead", shortName="motr", doc="Max. number of objects to read from the files.  -1 [default] means unlimited", required=false, exclusiveOf="", validation="")
var maxObjectsToRead: Option[Int] = None

/**
 * Short name of maxObjectsToRead
 * @return Short name of maxObjectsToRead
 */
def motr = this.maxObjectsToRead

/**
 * Short name of maxObjectsToRead
 * @param value Short name of maxObjectsToRead
 */
def motr_=(value: Option[Int]) { this.maxObjectsToRead = value }

/** Max. number of differences to include in the summary.  -1 [default] means unlimited */
@Argument(fullName="maxRawDiffsToSummarize", shortName="maxRawDiffsToSummarize", doc="Max. number of differences to include in the summary.  -1 [default] means unlimited", required=false, exclusiveOf="", validation="")
var maxRawDiffsToSummarize: Option[Int] = None

/** If provided, we will compute the minimum pairwise differences to summary, which can be extremely expensive */
@Argument(fullName="doPairwise", shortName="doPairwise", doc="If provided, we will compute the minimum pairwise differences to summary, which can be extremely expensive", required=false, exclusiveOf="", validation="")
var doPairwise: Boolean = _

/** Max. number of diffs to process */
@Argument(fullName="maxDiffs", shortName="M", doc="Max. number of diffs to process", required=false, exclusiveOf="", validation="")
var maxDiffs: Option[Int] = None

/**
 * Short name of maxDiffs
 * @return Short name of maxDiffs
 */
def M = this.maxDiffs

/**
 * Short name of maxDiffs
 * @param value Short name of maxDiffs
 */
def M_=(value: Option[Int]) { this.maxDiffs = value }

/** Max. number of diffs occuring exactly once in the file to process */
@Argument(fullName="maxCount1Diffs", shortName="M1", doc="Max. number of diffs occuring exactly once in the file to process", required=false, exclusiveOf="", validation="")
var maxCount1Diffs: Option[Int] = None

/**
 * Short name of maxCount1Diffs
 * @return Short name of maxCount1Diffs
 */
def M1 = this.maxCount1Diffs

/**
 * Short name of maxCount1Diffs
 * @param value Short name of maxCount1Diffs
 */
def M1_=(value: Option[Int]) { this.maxCount1Diffs = value }

/** Min number of observations for a records to display */
@Argument(fullName="minCountForDiff", shortName="MCFD", doc="Min number of observations for a records to display", required=false, exclusiveOf="", validation="")
var minCountForDiff: Option[Int] = None

/**
 * Short name of minCountForDiff
 * @return Short name of minCountForDiff
 */
def MCFD = this.minCountForDiff

/**
 * Short name of minCountForDiff
 * @param value Short name of minCountForDiff
 */
def MCFD_=(value: Option[Int]) { this.minCountForDiff = value }

/** Should we enumerate all differences between the files? */
@Argument(fullName="showItemizedDifferences", shortName="SID", doc="Should we enumerate all differences between the files?", required=false, exclusiveOf="", validation="")
var showItemizedDifferences: Boolean = _

/**
 * Short name of showItemizedDifferences
 * @return Short name of showItemizedDifferences
 */
def SID = this.showItemizedDifferences

/**
 * Short name of showItemizedDifferences
 * @param value Short name of showItemizedDifferences
 */
def SID_=(value: Boolean) { this.showItemizedDifferences = value }

/** Number of iterations to perform, should be 1 unless you are doing memory testing */
@Argument(fullName="iterations", shortName="", doc="Number of iterations to perform, should be 1 unless you are doing memory testing", required=false, exclusiveOf="", validation="")
var iterations: Option[Int] = None

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

override def commandLine = super.commandLine + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + required("-m", master, spaceSeparated=true, escape=true, format="%s") + required("-t", test, spaceSeparated=true, escape=true, format="%s") + optional("-motr", maxObjectsToRead, spaceSeparated=true, escape=true, format="%s") + optional("-maxRawDiffsToSummarize", maxRawDiffsToSummarize, spaceSeparated=true, escape=true, format="%s") + conditional(doPairwise, "-doPairwise", escape=true, format="%s") + optional("-M", maxDiffs, spaceSeparated=true, escape=true, format="%s") + optional("-M1", maxCount1Diffs, spaceSeparated=true, escape=true, format="%s") + optional("-MCFD", minCountForDiff, spaceSeparated=true, escape=true, format="%s") + conditional(showItemizedDifferences, "-SID", escape=true, format="%s") + optional("--iterations", iterations, spaceSeparated=true, escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
