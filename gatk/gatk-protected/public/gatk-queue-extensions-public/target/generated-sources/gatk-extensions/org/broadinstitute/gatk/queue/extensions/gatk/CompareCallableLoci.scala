package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class CompareCallableLoci extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "CompareCallableLoci"
analysis_type = "CompareCallableLoci"
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

/** First comparison track name */
@Input(fullName="comp1", shortName="comp1", doc="First comparison track name", required=true, exclusiveOf="", validation="")
var comp1: File = _

/** Dependencies on the index of comp1 */
@Input(fullName="comp1Index", shortName="", doc="Dependencies on the index of comp1", required=false, exclusiveOf="", validation="")
private var comp1Index: Seq[File] = Nil

/** Second comparison track name */
@Input(fullName="comp2", shortName="comp2", doc="Second comparison track name", required=true, exclusiveOf="", validation="")
var comp2: File = _

/** Dependencies on the index of comp2 */
@Input(fullName="comp2Index", shortName="", doc="Dependencies on the index of comp2", required=false, exclusiveOf="", validation="")
private var comp2Index: Seq[File] = Nil

/** If provided, prints sites satisfying this state pair */
@Argument(fullName="printstate", shortName="printState", doc="If provided, prints sites satisfying this state pair", required=false, exclusiveOf="", validation="")
var printstate: String = _

/**
 * Short name of printstate
 * @return Short name of printstate
 */
def printState = this.printstate

/**
 * Short name of printstate
 * @param value Short name of printstate
 */
def printState_=(value: String) { this.printstate = value }

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
if (comp1 != null)
  comp1Index :+= new File(comp1.getPath + ".idx")
if (comp2 != null)
  comp2Index :+= new File(comp2.getPath + ".idx")
}

override def commandLine = super.commandLine + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + required(TaggedFile.formatCommandLineParameter("-comp1", comp1), comp1, spaceSeparated=true, escape=true, format="%s") + required(TaggedFile.formatCommandLineParameter("-comp2", comp2), comp2, spaceSeparated=true, escape=true, format="%s") + optional("-printState", printstate, spaceSeparated=true, escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
