package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class ValidationSiteSelector extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "ValidationSiteSelector"
analysis_type = "ValidationSiteSelector"
scatterClass = classOf[LocusScatterFunction]
setupScatterFunction = { case scatter: GATKScatterFunction => scatter.includeUnmapped = false }

/** Input VCF file, can be specified multiple times */
@Input(fullName="variant", shortName="V", doc="Input VCF file, can be specified multiple times", required=true, exclusiveOf="", validation="")
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

/** Include genotypes from this sample. Can be specified multiple times */
@Argument(fullName="sample_name", shortName="sn", doc="Include genotypes from this sample. Can be specified multiple times", required=false, exclusiveOf="", validation="")
var sample_name: Seq[String] = Nil

/**
 * Short name of sample_name
 * @return Short name of sample_name
 */
def sn = this.sample_name

/**
 * Short name of sample_name
 * @param value Short name of sample_name
 */
def sn_=(value: Seq[String]) { this.sample_name = value }

/** Regular expression to select many samples from the ROD tracks provided. Can be specified multiple times */
@Argument(fullName="sample_expressions", shortName="se", doc="Regular expression to select many samples from the ROD tracks provided. Can be specified multiple times", required=false, exclusiveOf="", validation="")
var sample_expressions: Seq[String] = Nil

/**
 * Short name of sample_expressions
 * @return Short name of sample_expressions
 */
def se = this.sample_expressions

/**
 * Short name of sample_expressions
 * @param value Short name of sample_expressions
 */
def se_=(value: Seq[String]) { this.sample_expressions = value }

/** File containing a list of samples (one per line) to include. Can be specified multiple times */
@Input(fullName="sample_file", shortName="sf", doc="File containing a list of samples (one per line) to include. Can be specified multiple times", required=false, exclusiveOf="", validation="")
var sample_file: Seq[File] = Nil

/**
 * Short name of sample_file
 * @return Short name of sample_file
 */
def sf = this.sample_file

/**
 * Short name of sample_file
 * @param value Short name of sample_file
 */
def sf_=(value: Seq[File]) { this.sample_file = value }

/** Sample selection mode */
@Argument(fullName="sampleMode", shortName="sampleMode", doc="Sample selection mode", required=false, exclusiveOf="", validation="")
var sampleMode: org.broadinstitute.gatk.tools.walkers.validation.validationsiteselector.ValidationSiteSelector.SAMPLE_SELECTION_MODE = _

/** GL-based selection mode only: the probability that a site is non-reference in the samples for which to include the site */
@Argument(fullName="samplePNonref", shortName="samplePNonref", doc="GL-based selection mode only: the probability that a site is non-reference in the samples for which to include the site", required=false, exclusiveOf="", validation="")
var samplePNonref: Option[Double] = None

/** Format string for samplePNonref */
@Argument(fullName="samplePNonrefFormat", shortName="", doc="Format string for samplePNonref", required=false, exclusiveOf="", validation="")
var samplePNonrefFormat: String = "%s"

/** Number of output validation sites */
@Argument(fullName="numValidationSites", shortName="numSites", doc="Number of output validation sites", required=true, exclusiveOf="", validation="")
var numValidationSites: Int = _

/**
 * Short name of numValidationSites
 * @return Short name of numValidationSites
 */
def numSites = this.numValidationSites

/**
 * Short name of numValidationSites
 * @param value Short name of numValidationSites
 */
def numSites_=(value: Int) { this.numValidationSites = value }

/** If true, will include filtered sites in set to choose variants from */
@Argument(fullName="includeFilteredSites", shortName="ifs", doc="If true, will include filtered sites in set to choose variants from", required=false, exclusiveOf="", validation="")
var includeFilteredSites: Boolean = _

/**
 * Short name of includeFilteredSites
 * @return Short name of includeFilteredSites
 */
def ifs = this.includeFilteredSites

/**
 * Short name of includeFilteredSites
 * @param value Short name of includeFilteredSites
 */
def ifs_=(value: Boolean) { this.includeFilteredSites = value }

/** If true, will ignore genotypes in VCF, will take AC,AF from annotations and will make no sample selection */
@Argument(fullName="ignoreGenotypes", shortName="ignoreGenotypes", doc="If true, will ignore genotypes in VCF, will take AC,AF from annotations and will make no sample selection", required=false, exclusiveOf="", validation="")
var ignoreGenotypes: Boolean = _

/** If true, will ignore polymorphic status in VCF, and will take VCF record directly without pre-selection */
@Argument(fullName="ignorePolymorphicStatus", shortName="ignorePolymorphicStatus", doc="If true, will ignore polymorphic status in VCF, and will take VCF record directly without pre-selection", required=false, exclusiveOf="", validation="")
var ignorePolymorphicStatus: Boolean = _

/** Number of frequency bins if we're to match AF distribution */
@Argument(fullName="numFrequencyBins", shortName="numBins", doc="Number of frequency bins if we're to match AF distribution", required=false, exclusiveOf="", validation="")
var numFrequencyBins: Option[Int] = None

/**
 * Short name of numFrequencyBins
 * @return Short name of numFrequencyBins
 */
def numBins = this.numFrequencyBins

/**
 * Short name of numFrequencyBins
 * @param value Short name of numFrequencyBins
 */
def numBins_=(value: Option[Int]) { this.numFrequencyBins = value }

/** Allele Frequency selection mode */
@Argument(fullName="frequencySelectionMode", shortName="freqMode", doc="Allele Frequency selection mode", required=false, exclusiveOf="", validation="")
var frequencySelectionMode: org.broadinstitute.gatk.tools.walkers.validation.validationsiteselector.ValidationSiteSelector.AF_COMPUTATION_MODE = _

/**
 * Short name of frequencySelectionMode
 * @return Short name of frequencySelectionMode
 */
def freqMode = this.frequencySelectionMode

/**
 * Short name of frequencySelectionMode
 * @param value Short name of frequencySelectionMode
 */
def freqMode_=(value: org.broadinstitute.gatk.tools.walkers.validation.validationsiteselector.ValidationSiteSelector.AF_COMPUTATION_MODE) { this.frequencySelectionMode = value }

/** Select only a certain type of variants from the input file. Valid types are INDEL, SNP, MIXED, MNP, SYMBOLIC, NO_VARIATION. Can be specified multiple times */
@Argument(fullName="selectTypeToInclude", shortName="selectType", doc="Select only a certain type of variants from the input file. Valid types are INDEL, SNP, MIXED, MNP, SYMBOLIC, NO_VARIATION. Can be specified multiple times", required=false, exclusiveOf="", validation="")
var selectTypeToInclude: Seq[htsjdk.variant.variantcontext.VariantContext.Type] = Nil

/**
 * Short name of selectTypeToInclude
 * @return Short name of selectTypeToInclude
 */
def selectType = this.selectTypeToInclude

/**
 * Short name of selectTypeToInclude
 * @param value Short name of selectTypeToInclude
 */
def selectType_=(value: Seq[htsjdk.variant.variantcontext.VariantContext.Type]) { this.selectTypeToInclude = value }

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
if (out != null && !org.broadinstitute.gatk.utils.io.IOUtils.isSpecialFile(out))
  if (!org.broadinstitute.gatk.utils.commandline.ArgumentTypeDescriptor.isCompressed(out.getPath))
    outIndex = new File(out.getPath + ".idx")
}

override def commandLine = super.commandLine + repeat("-V", variant, formatPrefix=TaggedFile.formatCommandLineParameter, spaceSeparated=true, escape=true, format="%s") + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + repeat("-sn", sample_name, spaceSeparated=true, escape=true, format="%s") + repeat("-se", sample_expressions, spaceSeparated=true, escape=true, format="%s") + repeat("-sf", sample_file, spaceSeparated=true, escape=true, format="%s") + optional("-sampleMode", sampleMode, spaceSeparated=true, escape=true, format="%s") + optional("-samplePNonref", samplePNonref, spaceSeparated=true, escape=true, format=samplePNonrefFormat) + required("-numSites", numValidationSites, spaceSeparated=true, escape=true, format="%s") + conditional(includeFilteredSites, "-ifs", escape=true, format="%s") + conditional(ignoreGenotypes, "-ignoreGenotypes", escape=true, format="%s") + conditional(ignorePolymorphicStatus, "-ignorePolymorphicStatus", escape=true, format="%s") + optional("-numBins", numFrequencyBins, spaceSeparated=true, escape=true, format="%s") + optional("-freqMode", frequencySelectionMode, spaceSeparated=true, escape=true, format="%s") + repeat("-selectType", selectTypeToInclude, spaceSeparated=true, escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
