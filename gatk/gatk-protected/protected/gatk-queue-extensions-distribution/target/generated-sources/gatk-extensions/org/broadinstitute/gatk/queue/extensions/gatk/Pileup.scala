package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class Pileup extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "Pileup"
analysis_type = "Pileup"
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

/** Add an extra verbose section to the pileup output */
@Argument(fullName="showVerbose", shortName="verbose", doc="Add an extra verbose section to the pileup output", required=false, exclusiveOf="", validation="")
var showVerbose: Boolean = _

/**
 * Short name of showVerbose
 * @return Short name of showVerbose
 */
def verbose = this.showVerbose

/**
 * Short name of showVerbose
 * @param value Short name of showVerbose
 */
def verbose_=(value: Boolean) { this.showVerbose = value }

/** ROD file containing metadata */
@Input(fullName="metadata", shortName="metadata", doc="ROD file containing metadata", required=false, exclusiveOf="", validation="")
var metadata: Seq[File] = Nil

/** Dependencies on any indexes of metadata */
@Input(fullName="metadataIndexes", shortName="", doc="Dependencies on any indexes of metadata", required=false, exclusiveOf="", validation="")
private var metadataIndexes: Seq[File] = Nil

/** Output insert length */
@Argument(fullName="outputInsertLength", shortName="outputInsertLength", doc="Output insert length", required=false, exclusiveOf="", validation="")
var outputInsertLength: Boolean = _

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
metadataIndexes ++= metadata.filter(orig => orig != null && (!orig.getName.endsWith(".list"))).map(orig => new File(orig.getPath + ".idx"))
}

override def commandLine = super.commandLine + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + conditional(showVerbose, "-verbose", escape=true, format="%s") + repeat("-metadata", metadata, formatPrefix=TaggedFile.formatCommandLineParameter, spaceSeparated=true, escape=true, format="%s") + conditional(outputInsertLength, "-outputInsertLength", escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
