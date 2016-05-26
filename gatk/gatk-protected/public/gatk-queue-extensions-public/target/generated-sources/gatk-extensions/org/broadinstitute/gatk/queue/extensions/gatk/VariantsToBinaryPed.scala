package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class VariantsToBinaryPed extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "VariantsToBinaryPed"
analysis_type = "VariantsToBinaryPed"
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

/** dbSNP file */
@Input(fullName="dbsnp", shortName="D", doc="dbSNP file", required=false, exclusiveOf="", validation="")
var dbsnp: File = _

/**
 * Short name of dbsnp
 * @return Short name of dbsnp
 */
def D = this.dbsnp

/**
 * Short name of dbsnp
 * @param value Short name of dbsnp
 */
def D_=(value: File) { this.dbsnp = value }

/** Dependencies on the index of dbsnp */
@Input(fullName="dbsnpIndex", shortName="", doc="Dependencies on the index of dbsnp", required=false, exclusiveOf="", validation="")
private var dbsnpIndex: Seq[File] = Nil

/** Sample metadata file */
@Input(fullName="metaData", shortName="m", doc="Sample metadata file", required=true, exclusiveOf="", validation="")
var metaData: File = _

/**
 * Short name of metaData
 * @return Short name of metaData
 */
def m = this.metaData

/**
 * Short name of metaData
 * @param value Short name of metaData
 */
def m_=(value: File) { this.metaData = value }

/** The output file mode (SNP major or individual major) */
@Input(fullName="outputMode", shortName="mode", doc="The output file mode (SNP major or individual major)", required=false, exclusiveOf="", validation="")
var outputMode: File = _

/**
 * Short name of outputMode
 * @return Short name of outputMode
 */
def mode = this.outputMode

/**
 * Short name of outputMode
 * @param value Short name of outputMode
 */
def mode_=(value: File) { this.outputMode = value }

/** output bed file */
@Output(fullName="bed", shortName="bed", doc="output bed file", required=true, exclusiveOf="", validation="")
@Gather(classOf[org.broadinstitute.gatk.queue.function.scattergather.SimpleTextGatherFunction])
var bed: File = _

/** output map file */
@Output(fullName="bim", shortName="bim", doc="output map file", required=true, exclusiveOf="", validation="")
@Gather(classOf[org.broadinstitute.gatk.queue.function.scattergather.SimpleTextGatherFunction])
var bim: File = _

/** output fam file */
@Output(fullName="fam", shortName="fam", doc="output fam file", required=true, exclusiveOf="", validation="")
@Gather(classOf[org.broadinstitute.gatk.queue.function.scattergather.SimpleTextGatherFunction])
var fam: File = _

/** If genotype quality is lower than this value, output NO_CALL */
@Argument(fullName="minGenotypeQuality", shortName="mgq", doc="If genotype quality is lower than this value, output NO_CALL", required=true, exclusiveOf="", validation="")
var minGenotypeQuality: Int = _

/**
 * Short name of minGenotypeQuality
 * @return Short name of minGenotypeQuality
 */
def mgq = this.minGenotypeQuality

/**
 * Short name of minGenotypeQuality
 * @param value Short name of minGenotypeQuality
 */
def mgq_=(value: Int) { this.minGenotypeQuality = value }

/** Sets the major allele to be 'reference' for the bim file, rather than the ref allele */
@Argument(fullName="majorAlleleFirst", shortName="", doc="Sets the major allele to be 'reference' for the bim file, rather than the ref allele", required=false, exclusiveOf="", validation="")
var majorAlleleFirst: Boolean = _

/** Checks that alternate alleles actually appear in samples, erroring out if they do not */
@Argument(fullName="checkAlternateAlleles", shortName="", doc="Checks that alternate alleles actually appear in samples, erroring out if they do not", required=false, exclusiveOf="", validation="")
var checkAlternateAlleles: Boolean = _

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
if (dbsnp != null)
  dbsnpIndex :+= new File(dbsnp.getPath + ".idx")
}

override def commandLine = super.commandLine + required(TaggedFile.formatCommandLineParameter("-V", variant), variant, spaceSeparated=true, escape=true, format="%s") + optional(TaggedFile.formatCommandLineParameter("-D", dbsnp), dbsnp, spaceSeparated=true, escape=true, format="%s") + required("-m", metaData, spaceSeparated=true, escape=true, format="%s") + optional("-mode", outputMode, spaceSeparated=true, escape=true, format="%s") + required("-bed", bed, spaceSeparated=true, escape=true, format="%s") + required("-bim", bim, spaceSeparated=true, escape=true, format="%s") + required("-fam", fam, spaceSeparated=true, escape=true, format="%s") + required("-mgq", minGenotypeQuality, spaceSeparated=true, escape=true, format="%s") + conditional(majorAlleleFirst, "--majorAlleleFirst", escape=true, format="%s") + conditional(checkAlternateAlleles, "--checkAlternateAlleles", escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
