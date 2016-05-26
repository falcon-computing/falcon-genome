package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class CalculateGenotypePosteriors extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "CalculateGenotypePosteriors"
analysis_type = "CalculateGenotypePosteriors"
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

/** Other callsets to use in generating genotype posteriors */
@Input(fullName="supporting", shortName="supporting", doc="Other callsets to use in generating genotype posteriors", required=false, exclusiveOf="", validation="")
var supporting: Seq[File] = Nil

/** Dependencies on any indexes of supporting */
@Input(fullName="supportingIndexes", shortName="", doc="Dependencies on any indexes of supporting", required=false, exclusiveOf="", validation="")
private var supportingIndexes: Seq[File] = Nil

/** The global Dirichlet prior parameters for the allele frequency */
@Argument(fullName="globalPrior", shortName="G", doc="The global Dirichlet prior parameters for the allele frequency", required=false, exclusiveOf="", validation="")
var globalPrior: Option[Double] = None

/**
 * Short name of globalPrior
 * @return Short name of globalPrior
 */
def G = this.globalPrior

/**
 * Short name of globalPrior
 * @param value Short name of globalPrior
 */
def G_=(value: Option[Double]) { this.globalPrior = value }

/** Format string for globalPrior */
@Argument(fullName="globalPriorFormat", shortName="", doc="Format string for globalPrior", required=false, exclusiveOf="", validation="")
var globalPriorFormat: String = "%s"

/** The de novo mutation prior */
@Argument(fullName="deNovoPrior", shortName="DNP", doc="The de novo mutation prior", required=false, exclusiveOf="", validation="")
var deNovoPrior: Option[Double] = None

/**
 * Short name of deNovoPrior
 * @return Short name of deNovoPrior
 */
def DNP = this.deNovoPrior

/**
 * Short name of deNovoPrior
 * @param value Short name of deNovoPrior
 */
def DNP_=(value: Option[Double]) { this.deNovoPrior = value }

/** Format string for deNovoPrior */
@Argument(fullName="deNovoPriorFormat", shortName="", doc="Format string for deNovoPrior", required=false, exclusiveOf="", validation="")
var deNovoPriorFormat: String = "%s"

/** The number of homozygous reference to infer were seen at a position where an \"other callset\" contains no site or genotype information */
@Argument(fullName="numRefSamplesIfNoCall", shortName="nrs", doc="The number of homozygous reference to infer were seen at a position where an \"other callset\" contains no site or genotype information", required=false, exclusiveOf="", validation="")
var numRefSamplesIfNoCall: Option[Int] = None

/**
 * Short name of numRefSamplesIfNoCall
 * @return Short name of numRefSamplesIfNoCall
 */
def nrs = this.numRefSamplesIfNoCall

/**
 * Short name of numRefSamplesIfNoCall
 * @param value Short name of numRefSamplesIfNoCall
 */
def nrs_=(value: Option[Int]) { this.numRefSamplesIfNoCall = value }

/** Use the AC field as opposed to MLEAC. Does nothing if VCF lacks MLEAC field */
@Argument(fullName="defaultToAC", shortName="useAC", doc="Use the AC field as opposed to MLEAC. Does nothing if VCF lacks MLEAC field", required=false, exclusiveOf="", validation="")
var defaultToAC: Boolean = _

/**
 * Short name of defaultToAC
 * @return Short name of defaultToAC
 */
def useAC = this.defaultToAC

/**
 * Short name of defaultToAC
 * @param value Short name of defaultToAC
 */
def useAC_=(value: Boolean) { this.defaultToAC = value }

/** Use external information only; do not inform genotype priors by the discovered allele frequency in the callset whose posteriors are being calculated. Useful for callsets containing related individuals. */
@Argument(fullName="ignoreInputSamples", shortName="ext", doc="Use external information only; do not inform genotype priors by the discovered allele frequency in the callset whose posteriors are being calculated. Useful for callsets containing related individuals.", required=false, exclusiveOf="", validation="")
var ignoreInputSamples: Boolean = _

/**
 * Short name of ignoreInputSamples
 * @return Short name of ignoreInputSamples
 */
def ext = this.ignoreInputSamples

/**
 * Short name of ignoreInputSamples
 * @param value Short name of ignoreInputSamples
 */
def ext_=(value: Boolean) { this.ignoreInputSamples = value }

/** Do not use discovered allele count in the input callset for variants that do not appear in the external callset.  */
@Argument(fullName="discoveredACpriorsOff", shortName="useACoff", doc="Do not use discovered allele count in the input callset for variants that do not appear in the external callset. ", required=false, exclusiveOf="", validation="")
var discoveredACpriorsOff: Boolean = _

/**
 * Short name of discoveredACpriorsOff
 * @return Short name of discoveredACpriorsOff
 */
def useACoff = this.discoveredACpriorsOff

/**
 * Short name of discoveredACpriorsOff
 * @param value Short name of discoveredACpriorsOff
 */
def useACoff_=(value: Boolean) { this.discoveredACpriorsOff = value }

/** Skip application of population-based priors */
@Argument(fullName="skipPopulationPriors", shortName="skipPop", doc="Skip application of population-based priors", required=false, exclusiveOf="", validation="")
var skipPopulationPriors: Boolean = _

/**
 * Short name of skipPopulationPriors
 * @return Short name of skipPopulationPriors
 */
def skipPop = this.skipPopulationPriors

/**
 * Short name of skipPopulationPriors
 * @param value Short name of skipPopulationPriors
 */
def skipPop_=(value: Boolean) { this.skipPopulationPriors = value }

/** Skip application of family-based priors */
@Argument(fullName="skipFamilyPriors", shortName="skipFam", doc="Skip application of family-based priors", required=false, exclusiveOf="", validation="")
var skipFamilyPriors: Boolean = _

/**
 * Short name of skipFamilyPriors
 * @return Short name of skipFamilyPriors
 */
def skipFam = this.skipFamilyPriors

/**
 * Short name of skipFamilyPriors
 * @param value Short name of skipFamilyPriors
 */
def skipFam_=(value: Boolean) { this.skipFamilyPriors = value }

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
supportingIndexes ++= supporting.filter(orig => orig != null && (!orig.getName.endsWith(".list"))).map(orig => new File(orig.getPath + ".idx"))
if (out != null && !org.broadinstitute.gatk.utils.io.IOUtils.isSpecialFile(out))
  if (!org.broadinstitute.gatk.utils.commandline.ArgumentTypeDescriptor.isCompressed(out.getPath))
    outIndex = new File(out.getPath + ".idx")
}

override def commandLine = super.commandLine + required(TaggedFile.formatCommandLineParameter("-V", variant), variant, spaceSeparated=true, escape=true, format="%s") + repeat("-supporting", supporting, formatPrefix=TaggedFile.formatCommandLineParameter, spaceSeparated=true, escape=true, format="%s") + optional("-G", globalPrior, spaceSeparated=true, escape=true, format=globalPriorFormat) + optional("-DNP", deNovoPrior, spaceSeparated=true, escape=true, format=deNovoPriorFormat) + optional("-nrs", numRefSamplesIfNoCall, spaceSeparated=true, escape=true, format="%s") + conditional(defaultToAC, "-useAC", escape=true, format="%s") + conditional(ignoreInputSamples, "-ext", escape=true, format="%s") + conditional(discoveredACpriorsOff, "-useACoff", escape=true, format="%s") + conditional(skipPopulationPriors, "-skipPop", escape=true, format="%s") + conditional(skipFamilyPriors, "-skipFam", escape=true, format="%s") + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
