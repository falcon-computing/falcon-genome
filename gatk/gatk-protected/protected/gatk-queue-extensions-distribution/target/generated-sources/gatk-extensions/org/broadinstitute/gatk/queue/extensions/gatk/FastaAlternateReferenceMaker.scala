package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class FastaAlternateReferenceMaker extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "FastaAlternateReferenceMaker"
analysis_type = "FastaAlternateReferenceMaker"
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

/** SNP mask VCF file */
@Input(fullName="snpmask", shortName="snpmask", doc="SNP mask VCF file", required=false, exclusiveOf="", validation="")
var snpmask: File = _

/** Dependencies on the index of snpmask */
@Input(fullName="snpmaskIndex", shortName="", doc="Dependencies on the index of snpmask", required=false, exclusiveOf="", validation="")
private var snpmaskIndex: Seq[File] = Nil

/** SNP mask priority */
@Argument(fullName="snpmaskPriority", shortName="snpmaskPriority", doc="SNP mask priority", required=false, exclusiveOf="", validation="")
var snpmaskPriority: Boolean = _

/** If specified, heterozygous SNP sites will be output using IUPAC ambiguity codes given the genotypes for this sample */
@Argument(fullName="use_IUPAC_sample", shortName="IUPAC", doc="If specified, heterozygous SNP sites will be output using IUPAC ambiguity codes given the genotypes for this sample", required=false, exclusiveOf="", validation="")
var use_IUPAC_sample: String = _

/**
 * Short name of use_IUPAC_sample
 * @return Short name of use_IUPAC_sample
 */
def IUPAC = this.use_IUPAC_sample

/**
 * Short name of use_IUPAC_sample
 * @param value Short name of use_IUPAC_sample
 */
def IUPAC_=(value: String) { this.use_IUPAC_sample = value }

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

/** Maximum length of sequence to write per line */
@Argument(fullName="lineWidth", shortName="lw", doc="Maximum length of sequence to write per line", required=false, exclusiveOf="", validation="")
var lineWidth: Option[Int] = None

/**
 * Short name of lineWidth
 * @return Short name of lineWidth
 */
def lw = this.lineWidth

/**
 * Short name of lineWidth
 * @param value Short name of lineWidth
 */
def lw_=(value: Option[Int]) { this.lineWidth = value }

/** Print sequences with no FASTA header lines, one line per interval (i.e. lineWidth = infinity) */
@Argument(fullName="rawOnelineSeq", shortName="raw", doc="Print sequences with no FASTA header lines, one line per interval (i.e. lineWidth = infinity)", required=false, exclusiveOf="", validation="")
var rawOnelineSeq: Boolean = _

/**
 * Short name of rawOnelineSeq
 * @return Short name of rawOnelineSeq
 */
def raw = this.rawOnelineSeq

/**
 * Short name of rawOnelineSeq
 * @param value Short name of rawOnelineSeq
 */
def raw_=(value: Boolean) { this.rawOnelineSeq = value }

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
if (snpmask != null)
  snpmaskIndex :+= new File(snpmask.getPath + ".idx")
}

override def commandLine = super.commandLine + required(TaggedFile.formatCommandLineParameter("-V", variant), variant, spaceSeparated=true, escape=true, format="%s") + optional(TaggedFile.formatCommandLineParameter("-snpmask", snpmask), snpmask, spaceSeparated=true, escape=true, format="%s") + conditional(snpmaskPriority, "-snpmaskPriority", escape=true, format="%s") + optional("-IUPAC", use_IUPAC_sample, spaceSeparated=true, escape=true, format="%s") + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + optional("-lw", lineWidth, spaceSeparated=true, escape=true, format="%s") + conditional(rawOnelineSeq, "-raw", escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
