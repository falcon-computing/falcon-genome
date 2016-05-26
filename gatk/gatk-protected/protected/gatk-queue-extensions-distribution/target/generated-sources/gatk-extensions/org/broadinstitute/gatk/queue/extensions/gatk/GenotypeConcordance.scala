package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class GenotypeConcordance extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "GenotypeConcordance"
analysis_type = "GenotypeConcordance"
scatterClass = classOf[LocusScatterFunction]
setupScatterFunction = { case scatter: GATKScatterFunction => scatter.includeUnmapped = false }

/** The variants and genotypes to evaluate */
@Input(fullName="eval", shortName="eval", doc="The variants and genotypes to evaluate", required=true, exclusiveOf="", validation="")
var eval: File = _

/** Dependencies on the index of eval */
@Input(fullName="evalIndex", shortName="", doc="Dependencies on the index of eval", required=false, exclusiveOf="", validation="")
private var evalIndex: Seq[File] = Nil

/** The variants and genotypes to compare against */
@Input(fullName="comp", shortName="comp", doc="The variants and genotypes to compare against", required=true, exclusiveOf="", validation="")
var comp: File = _

/** Dependencies on the index of comp */
@Input(fullName="compIndex", shortName="", doc="Dependencies on the index of comp", required=false, exclusiveOf="", validation="")
private var compIndex: Seq[File] = Nil

/** Filters will be ignored */
@Argument(fullName="ignoreFilters", shortName="", doc="Filters will be ignored", required=false, exclusiveOf="", validation="")
var ignoreFilters: Boolean = _

/** One or more criteria to use to set EVAL genotypes to no-call. These genotype-level filters are only applied to the EVAL rod. */
@Argument(fullName="genotypeFilterExpressionEval", shortName="gfe", doc="One or more criteria to use to set EVAL genotypes to no-call. These genotype-level filters are only applied to the EVAL rod.", required=false, exclusiveOf="", validation="")
var genotypeFilterExpressionEval: Seq[String] = Nil

/**
 * Short name of genotypeFilterExpressionEval
 * @return Short name of genotypeFilterExpressionEval
 */
def gfe = this.genotypeFilterExpressionEval

/**
 * Short name of genotypeFilterExpressionEval
 * @param value Short name of genotypeFilterExpressionEval
 */
def gfe_=(value: Seq[String]) { this.genotypeFilterExpressionEval = value }

/** One or more criteria to use to set COMP genotypes to no-call. These genotype-level filters are only applied to the COMP rod. */
@Argument(fullName="genotypeFilterExpressionComp", shortName="gfc", doc="One or more criteria to use to set COMP genotypes to no-call. These genotype-level filters are only applied to the COMP rod.", required=false, exclusiveOf="", validation="")
var genotypeFilterExpressionComp: Seq[String] = Nil

/**
 * Short name of genotypeFilterExpressionComp
 * @return Short name of genotypeFilterExpressionComp
 */
def gfc = this.genotypeFilterExpressionComp

/**
 * Short name of genotypeFilterExpressionComp
 * @param value Short name of genotypeFilterExpressionComp
 */
def gfc_=(value: Seq[String]) { this.genotypeFilterExpressionComp = value }

/** Molten rather than tabular output */
@Argument(fullName="moltenize", shortName="moltenize", doc="Molten rather than tabular output", required=false, exclusiveOf="", validation="")
var moltenize: Boolean = _

/** File to output the discordant sites and genotypes. */
@Argument(fullName="printInterestingSites", shortName="sites", doc="File to output the discordant sites and genotypes.", required=false, exclusiveOf="", validation="")
var printInterestingSites: File = _

/**
 * Short name of printInterestingSites
 * @return Short name of printInterestingSites
 */
def sites = this.printInterestingSites

/**
 * Short name of printInterestingSites
 * @param value Short name of printInterestingSites
 */
def sites_=(value: File) { this.printInterestingSites = value }

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
if (eval != null)
  evalIndex :+= new File(eval.getPath + ".idx")
if (comp != null)
  compIndex :+= new File(comp.getPath + ".idx")
}

override def commandLine = super.commandLine + required(TaggedFile.formatCommandLineParameter("-eval", eval), eval, spaceSeparated=true, escape=true, format="%s") + required(TaggedFile.formatCommandLineParameter("-comp", comp), comp, spaceSeparated=true, escape=true, format="%s") + conditional(ignoreFilters, "--ignoreFilters", escape=true, format="%s") + repeat("-gfe", genotypeFilterExpressionEval, spaceSeparated=true, escape=true, format="%s") + repeat("-gfc", genotypeFilterExpressionComp, spaceSeparated=true, escape=true, format="%s") + conditional(moltenize, "-moltenize", escape=true, format="%s") + optional("-sites", printInterestingSites, spaceSeparated=true, escape=true, format="%s") + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
