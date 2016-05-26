package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class SelectHeaders extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "SelectHeaders"
analysis_type = "SelectHeaders"
scatterClass = classOf[LocusScatterFunction]
setupScatterFunction = { case scatter: GATKScatterFunction => scatter.includeUnmapped = false }

/** Input VCF file */
@Input(fullName="variant", shortName="V", doc="Input VCF file", required=true, exclusiveOf="", validation="")
var variant: File = _

/**
 * Short name of variant
 * @return Short name of variant
 */
def V = this.variant

/**
 * Short name of variant
 * @param value Short name of variant
 */
def V_=(value: File) { this.variant = value }

/** Dependencies on the index of variant */
@Input(fullName="variantIndex", shortName="", doc="Dependencies on the index of variant", required=false, exclusiveOf="", validation="")
private var variantIndex: Seq[File] = Nil

/** File to which variants should be written */
@Output(fullName="out", shortName="o", doc="File to which variants should be written", required=false, exclusiveOf="", validation="")
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

/** Include header. Can be specified multiple times */
@Argument(fullName="header_name", shortName="hn", doc="Include header. Can be specified multiple times", required=false, exclusiveOf="", validation="")
var header_name: Seq[String] = Nil

/**
 * Short name of header_name
 * @return Short name of header_name
 */
def hn = this.header_name

/**
 * Short name of header_name
 * @param value Short name of header_name
 */
def hn_=(value: Seq[String]) { this.header_name = value }

/** Regular expression to select many headers from the tracks provided. Can be specified multiple times */
@Argument(fullName="header_expression", shortName="he", doc="Regular expression to select many headers from the tracks provided. Can be specified multiple times", required=false, exclusiveOf="", validation="")
var header_expression: Seq[String] = Nil

/**
 * Short name of header_expression
 * @return Short name of header_expression
 */
def he = this.header_expression

/**
 * Short name of header_expression
 * @param value Short name of header_expression
 */
def he_=(value: Seq[String]) { this.header_expression = value }

/** Exclude header. Can be specified multiple times */
@Argument(fullName="exclude_header_name", shortName="xl_hn", doc="Exclude header. Can be specified multiple times", required=false, exclusiveOf="", validation="")
var exclude_header_name: Seq[String] = Nil

/**
 * Short name of exclude_header_name
 * @return Short name of exclude_header_name
 */
def xl_hn = this.exclude_header_name

/**
 * Short name of exclude_header_name
 * @param value Short name of exclude_header_name
 */
def xl_hn_=(value: Seq[String]) { this.exclude_header_name = value }

/** If set the interval file name minus the file extension, or the command line intervals, will be added to the headers */
@Argument(fullName="include_interval_names", shortName="iln", doc="If set the interval file name minus the file extension, or the command line intervals, will be added to the headers", required=false, exclusiveOf="", validation="")
var include_interval_names: Boolean = _

/**
 * Short name of include_interval_names
 * @return Short name of include_interval_names
 */
def iln = this.include_interval_names

/**
 * Short name of include_interval_names
 * @param value Short name of include_interval_names
 */
def iln_=(value: Boolean) { this.include_interval_names = value }

/** If set the headers normally output by the engine will be added to the headers */
@Argument(fullName="include_engine_headers", shortName="ieh", doc="If set the headers normally output by the engine will be added to the headers", required=false, exclusiveOf="", validation="")
var include_engine_headers: Boolean = _

/**
 * Short name of include_engine_headers
 * @return Short name of include_engine_headers
 */
def ieh = this.include_engine_headers

/**
 * Short name of include_engine_headers
 * @param value Short name of include_engine_headers
 */
def ieh_=(value: Boolean) { this.include_engine_headers = value }

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
if (variant != null)
  variantIndex :+= new File(variant.getPath + ".idx")
if (out != null && !org.broadinstitute.gatk.utils.io.IOUtils.isSpecialFile(out))
  if (!org.broadinstitute.gatk.utils.commandline.ArgumentTypeDescriptor.isCompressed(out.getPath))
    outIndex = new File(out.getPath + ".idx")
}

override def commandLine = super.commandLine + required(TaggedFile.formatCommandLineParameter("-V", variant), variant, spaceSeparated=true, escape=true, format="%s") + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + repeat("-hn", header_name, spaceSeparated=true, escape=true, format="%s") + repeat("-he", header_expression, spaceSeparated=true, escape=true, format="%s") + repeat("-xl_hn", exclude_header_name, spaceSeparated=true, escape=true, format="%s") + conditional(include_interval_names, "-iln", escape=true, format="%s") + conditional(include_engine_headers, "-ieh", escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
