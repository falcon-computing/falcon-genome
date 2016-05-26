package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class VariantsToTable extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "VariantsToTable"
analysis_type = "VariantsToTable"
scatterClass = classOf[LocusScatterFunction]
setupScatterFunction = { case scatter: GATKScatterFunction => scatter.includeUnmapped = false }

/** Input VCF file */
@Input(fullName="variant", shortName="V", doc="Input VCF file", required=true, exclusiveOf="", validation="")
var variant: Seq[File] = Nil

/**
 * Short name of variant
 * @return Short name of variant
 */
def V = this.variant

/**
 * Short name of variant
 * @param value Short name of variant
 */
def V_=(value: Seq[File]) { this.variant = value }

/** Dependencies on any indexes of variant */
@Input(fullName="variantIndexes", shortName="", doc="Dependencies on any indexes of variant", required=false, exclusiveOf="", validation="")
private var variantIndexes: Seq[File] = Nil

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

/** The name of each field to capture for output in the table */
@Argument(fullName="fields", shortName="F", doc="The name of each field to capture for output in the table", required=false, exclusiveOf="", validation="")
var fields: Seq[String] = Nil

/**
 * Short name of fields
 * @return Short name of fields
 */
def F = this.fields

/**
 * Short name of fields
 * @param value Short name of fields
 */
def F_=(value: Seq[String]) { this.fields = value }

/** The name of each genotype field to capture for output in the table */
@Argument(fullName="genotypeFields", shortName="GF", doc="The name of each genotype field to capture for output in the table", required=false, exclusiveOf="", validation="")
var genotypeFields: Seq[String] = Nil

/**
 * Short name of genotypeFields
 * @return Short name of genotypeFields
 */
def GF = this.genotypeFields

/**
 * Short name of genotypeFields
 * @param value Short name of genotypeFields
 */
def GF_=(value: Seq[String]) { this.genotypeFields = value }

/** If provided, field values from filtered records will be included in the output */
@Argument(fullName="showFiltered", shortName="raw", doc="If provided, field values from filtered records will be included in the output", required=false, exclusiveOf="", validation="")
var showFiltered: Boolean = _

/**
 * Short name of showFiltered
 * @return Short name of showFiltered
 */
def raw = this.showFiltered

/**
 * Short name of showFiltered
 * @param value Short name of showFiltered
 */
def raw_=(value: Boolean) { this.showFiltered = value }

/** If provided, we will emit at most maxRecord records to the table */
@Argument(fullName="maxRecords", shortName="M", doc="If provided, we will emit at most maxRecord records to the table", required=false, exclusiveOf="", validation="")
var maxRecords: Option[Int] = None

/**
 * Short name of maxRecords
 * @return Short name of maxRecords
 */
def M = this.maxRecords

/**
 * Short name of maxRecords
 * @param value Short name of maxRecords
 */
def M_=(value: Option[Int]) { this.maxRecords = value }

/** If provided, we will split multi-allelic records into multiple lines of output */
@Argument(fullName="splitMultiAllelic", shortName="SMA", doc="If provided, we will split multi-allelic records into multiple lines of output", required=false, exclusiveOf="", validation="")
var splitMultiAllelic: Boolean = _

/**
 * Short name of splitMultiAllelic
 * @return Short name of splitMultiAllelic
 */
def SMA = this.splitMultiAllelic

/**
 * Short name of splitMultiAllelic
 * @param value Short name of splitMultiAllelic
 */
def SMA_=(value: Boolean) { this.splitMultiAllelic = value }

/** If provided, we will produce molten output */
@Argument(fullName="moltenize", shortName="moltenize", doc="If provided, we will produce molten output", required=false, exclusiveOf="", validation="")
var moltenize: Boolean = _

/** If provided, we will not require every record to contain every field */
@Argument(fullName="allowMissingData", shortName="AMD", doc="If provided, we will not require every record to contain every field", required=false, exclusiveOf="", validation="")
var allowMissingData: Boolean = _

/**
 * Short name of allowMissingData
 * @return Short name of allowMissingData
 */
def AMD = this.allowMissingData

/**
 * Short name of allowMissingData
 * @param value Short name of allowMissingData
 */
def AMD_=(value: Boolean) { this.allowMissingData = value }

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
variantIndexes ++= variant.filter(orig => orig != null && (!orig.getName.endsWith(".list"))).map(orig => new File(orig.getPath + ".idx"))
}

override def commandLine = super.commandLine + repeat("-V", variant, formatPrefix=TaggedFile.formatCommandLineParameter, spaceSeparated=true, escape=true, format="%s") + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + repeat("-F", fields, spaceSeparated=true, escape=true, format="%s") + repeat("-GF", genotypeFields, spaceSeparated=true, escape=true, format="%s") + conditional(showFiltered, "-raw", escape=true, format="%s") + optional("-M", maxRecords, spaceSeparated=true, escape=true, format="%s") + conditional(splitMultiAllelic, "-SMA", escape=true, format="%s") + conditional(moltenize, "-moltenize", escape=true, format="%s") + conditional(allowMissingData, "-AMD", escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
