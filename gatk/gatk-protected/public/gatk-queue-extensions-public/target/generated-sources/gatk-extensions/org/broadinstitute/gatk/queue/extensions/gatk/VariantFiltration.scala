package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class VariantFiltration extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "VariantFiltration"
analysis_type = "VariantFiltration"
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

/** Input ROD mask */
@Input(fullName="mask", shortName="mask", doc="Input ROD mask", required=false, exclusiveOf="", validation="")
var mask: File = _

/** Dependencies on the index of mask */
@Input(fullName="maskIndex", shortName="", doc="Dependencies on the index of mask", required=false, exclusiveOf="", validation="")
private var maskIndex: Seq[File] = Nil

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

/** One or more expression used with INFO fields to filter */
@Argument(fullName="filterExpression", shortName="filter", doc="One or more expression used with INFO fields to filter", required=false, exclusiveOf="", validation="")
var filterExpression: Seq[String] = Nil

/**
 * Short name of filterExpression
 * @return Short name of filterExpression
 */
def filter = this.filterExpression

/**
 * Short name of filterExpression
 * @param value Short name of filterExpression
 */
def filter_=(value: Seq[String]) { this.filterExpression = value }

/** Names to use for the list of filters */
@Argument(fullName="filterName", shortName="filterName", doc="Names to use for the list of filters", required=false, exclusiveOf="", validation="")
var filterName: Seq[String] = Nil

/** One or more expression used with FORMAT (sample/genotype-level) fields to filter (see documentation guide for more info) */
@Argument(fullName="genotypeFilterExpression", shortName="G_filter", doc="One or more expression used with FORMAT (sample/genotype-level) fields to filter (see documentation guide for more info)", required=false, exclusiveOf="", validation="")
var genotypeFilterExpression: Seq[String] = Nil

/**
 * Short name of genotypeFilterExpression
 * @return Short name of genotypeFilterExpression
 */
def G_filter = this.genotypeFilterExpression

/**
 * Short name of genotypeFilterExpression
 * @param value Short name of genotypeFilterExpression
 */
def G_filter_=(value: Seq[String]) { this.genotypeFilterExpression = value }

/** Names to use for the list of sample/genotype filters (must be a 1-to-1 mapping); this name is put in the FILTER field for variants that get filtered */
@Argument(fullName="genotypeFilterName", shortName="G_filterName", doc="Names to use for the list of sample/genotype filters (must be a 1-to-1 mapping); this name is put in the FILTER field for variants that get filtered", required=false, exclusiveOf="", validation="")
var genotypeFilterName: Seq[String] = Nil

/**
 * Short name of genotypeFilterName
 * @return Short name of genotypeFilterName
 */
def G_filterName = this.genotypeFilterName

/**
 * Short name of genotypeFilterName
 * @param value Short name of genotypeFilterName
 */
def G_filterName_=(value: Seq[String]) { this.genotypeFilterName = value }

/** The number of SNPs which make up a cluster */
@Argument(fullName="clusterSize", shortName="cluster", doc="The number of SNPs which make up a cluster", required=false, exclusiveOf="", validation="")
var clusterSize: Option[Int] = None

/**
 * Short name of clusterSize
 * @return Short name of clusterSize
 */
def cluster = this.clusterSize

/**
 * Short name of clusterSize
 * @param value Short name of clusterSize
 */
def cluster_=(value: Option[Int]) { this.clusterSize = value }

/** The window size (in bases) in which to evaluate clustered SNPs */
@Argument(fullName="clusterWindowSize", shortName="window", doc="The window size (in bases) in which to evaluate clustered SNPs", required=false, exclusiveOf="", validation="")
var clusterWindowSize: Option[Int] = None

/**
 * Short name of clusterWindowSize
 * @return Short name of clusterWindowSize
 */
def window = this.clusterWindowSize

/**
 * Short name of clusterWindowSize
 * @param value Short name of clusterWindowSize
 */
def window_=(value: Option[Int]) { this.clusterWindowSize = value }

/** How many bases beyond records from a provided 'mask' rod should variants be filtered */
@Argument(fullName="maskExtension", shortName="maskExtend", doc="How many bases beyond records from a provided 'mask' rod should variants be filtered", required=false, exclusiveOf="", validation="")
var maskExtension: Option[Int] = None

/**
 * Short name of maskExtension
 * @return Short name of maskExtension
 */
def maskExtend = this.maskExtension

/**
 * Short name of maskExtension
 * @param value Short name of maskExtension
 */
def maskExtend_=(value: Option[Int]) { this.maskExtension = value }

/** The text to put in the FILTER field if a 'mask' rod is provided and overlaps with a variant call */
@Argument(fullName="maskName", shortName="maskName", doc="The text to put in the FILTER field if a 'mask' rod is provided and overlaps with a variant call", required=false, exclusiveOf="", validation="")
var maskName: String = _

/** Filter records NOT in given input mask. */
@Argument(fullName="filterNotInMask", shortName="filterNotInMask", doc="Filter records NOT in given input mask.", required=false, exclusiveOf="", validation="")
var filterNotInMask: Boolean = _

/** When evaluating the JEXL expressions, missing values should be considered failing the expression */
@Argument(fullName="missingValuesInExpressionsShouldEvaluateAsFailing", shortName="", doc="When evaluating the JEXL expressions, missing values should be considered failing the expression", required=false, exclusiveOf="", validation="")
var missingValuesInExpressionsShouldEvaluateAsFailing: Boolean = _

/** Remove previous filters applied to the VCF */
@Argument(fullName="invalidatePreviousFilters", shortName="", doc="Remove previous filters applied to the VCF", required=false, exclusiveOf="", validation="")
var invalidatePreviousFilters: Boolean = _

/** Invert the selection criteria for --filterExpression */
@Argument(fullName="invertFilterExpression", shortName="invfilter", doc="Invert the selection criteria for --filterExpression", required=false, exclusiveOf="", validation="")
var invertFilterExpression: Boolean = _

/**
 * Short name of invertFilterExpression
 * @return Short name of invertFilterExpression
 */
def invfilter = this.invertFilterExpression

/**
 * Short name of invertFilterExpression
 * @param value Short name of invertFilterExpression
 */
def invfilter_=(value: Boolean) { this.invertFilterExpression = value }

/** Invert the selection criteria for --genotypeFilterExpression */
@Argument(fullName="invertGenotypeFilterExpression", shortName="invG_filter", doc="Invert the selection criteria for --genotypeFilterExpression", required=false, exclusiveOf="", validation="")
var invertGenotypeFilterExpression: Boolean = _

/**
 * Short name of invertGenotypeFilterExpression
 * @return Short name of invertGenotypeFilterExpression
 */
def invG_filter = this.invertGenotypeFilterExpression

/**
 * Short name of invertGenotypeFilterExpression
 * @param value Short name of invertGenotypeFilterExpression
 */
def invG_filter_=(value: Boolean) { this.invertGenotypeFilterExpression = value }

/** Set filtered genotypes to no-call */
@Argument(fullName="setFilteredGtToNocall", shortName="", doc="Set filtered genotypes to no-call", required=false, exclusiveOf="", validation="")
var setFilteredGtToNocall: Boolean = _

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
if (mask != null)
  maskIndex :+= new File(mask.getPath + ".idx")
if (out != null && !org.broadinstitute.gatk.utils.io.IOUtils.isSpecialFile(out))
  if (!org.broadinstitute.gatk.utils.commandline.ArgumentTypeDescriptor.isCompressed(out.getPath))
    outIndex = new File(out.getPath + ".idx")
}

override def commandLine = super.commandLine + required(TaggedFile.formatCommandLineParameter("-V", variant), variant, spaceSeparated=true, escape=true, format="%s") + optional(TaggedFile.formatCommandLineParameter("-mask", mask), mask, spaceSeparated=true, escape=true, format="%s") + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + repeat("-filter", filterExpression, spaceSeparated=true, escape=true, format="%s") + repeat("-filterName", filterName, spaceSeparated=true, escape=true, format="%s") + repeat("-G_filter", genotypeFilterExpression, spaceSeparated=true, escape=true, format="%s") + repeat("-G_filterName", genotypeFilterName, spaceSeparated=true, escape=true, format="%s") + optional("-cluster", clusterSize, spaceSeparated=true, escape=true, format="%s") + optional("-window", clusterWindowSize, spaceSeparated=true, escape=true, format="%s") + optional("-maskExtend", maskExtension, spaceSeparated=true, escape=true, format="%s") + optional("-maskName", maskName, spaceSeparated=true, escape=true, format="%s") + conditional(filterNotInMask, "-filterNotInMask", escape=true, format="%s") + conditional(missingValuesInExpressionsShouldEvaluateAsFailing, "--missingValuesInExpressionsShouldEvaluateAsFailing", escape=true, format="%s") + conditional(invalidatePreviousFilters, "--invalidatePreviousFilters", escape=true, format="%s") + conditional(invertFilterExpression, "-invfilter", escape=true, format="%s") + conditional(invertGenotypeFilterExpression, "-invG_filter", escape=true, format="%s") + conditional(setFilteredGtToNocall, "--setFilteredGtToNocall", escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
