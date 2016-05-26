package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class DocumentationTest extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "DocumentationTest"
analysis_type = "DocumentationTest"
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

/** Output variants that were not called in this Feature comparison track */
@Input(fullName="listofRodBinding", shortName="disc", doc="Output variants that were not called in this Feature comparison track", required=false, exclusiveOf="", validation="")
var listofRodBinding: Seq[File] = Nil

/**
 * Short name of listofRodBinding
 * @return Short name of listofRodBinding
 */
def disc = this.listofRodBinding

/**
 * Short name of listofRodBinding
 * @param value Short name of listofRodBinding
 */
def disc_=(value: Seq[File]) { this.listofRodBinding = value }

/** Dependencies on any indexes of listofRodBinding */
@Input(fullName="listofRodBindingIndexes", shortName="", doc="Dependencies on any indexes of listofRodBinding", required=false, exclusiveOf="", validation="")
private var listofRodBindingIndexes: Seq[File] = Nil

/** Output variants that were also called in this Feature comparison track */
@Input(fullName="optionalRodBinding", shortName="conc", doc="Output variants that were also called in this Feature comparison track", required=false, exclusiveOf="", validation="")
var optionalRodBinding: File = _

/**
 * Short name of optionalRodBinding
 * @return Short name of optionalRodBinding
 */
def conc = this.optionalRodBinding

/**
 * Short name of optionalRodBinding
 * @param value Short name of optionalRodBinding
 */
def conc_=(value: File) { this.optionalRodBinding = value }

/** Dependencies on the index of optionalRodBinding */
@Input(fullName="optionalRodBindingIndex", shortName="", doc="Dependencies on the index of optionalRodBinding", required=false, exclusiveOf="", validation="")
private var optionalRodBindingIndex: Seq[File] = Nil

/** Output variants that were also called in this Feature comparison track */
@Input(fullName="optionalRodBindingWithoutDefault", shortName="optionalRodBindingWithoutDefault", doc="Output variants that were also called in this Feature comparison track", required=false, exclusiveOf="", validation="")
var optionalRodBindingWithoutDefault: File = _

/** Dependencies on the index of optionalRodBindingWithoutDefault */
@Input(fullName="optionalRodBindingWithoutDefaultIndex", shortName="", doc="Dependencies on the index of optionalRodBindingWithoutDefault", required=false, exclusiveOf="", validation="")
private var optionalRodBindingWithoutDefaultIndex: Seq[File] = Nil

/** Output variants that were also called in this Feature comparison track */
@Input(fullName="optionalRodBindingWithoutDefaultNull", shortName="shortTest", doc="Output variants that were also called in this Feature comparison track", required=false, exclusiveOf="", validation="")
var optionalRodBindingWithoutDefaultNull: File = _

/**
 * Short name of optionalRodBindingWithoutDefaultNull
 * @return Short name of optionalRodBindingWithoutDefaultNull
 */
def shortTest = this.optionalRodBindingWithoutDefaultNull

/**
 * Short name of optionalRodBindingWithoutDefaultNull
 * @param value Short name of optionalRodBindingWithoutDefaultNull
 */
def shortTest_=(value: File) { this.optionalRodBindingWithoutDefaultNull = value }

/** Dependencies on the index of optionalRodBindingWithoutDefaultNull */
@Input(fullName="optionalRodBindingWithoutDefaultNullIndex", shortName="", doc="Dependencies on the index of optionalRodBindingWithoutDefaultNull", required=false, exclusiveOf="", validation="")
private var optionalRodBindingWithoutDefaultNullIndex: Seq[File] = Nil

/** A RodBinding of feature */
@Input(fullName="featureArg", shortName="featureArg", doc="A RodBinding of feature", required=false, exclusiveOf="", validation="")
var featureArg: File = _

/** Dependencies on the index of featureArg */
@Input(fullName="featureArgIndex", shortName="", doc="Dependencies on the index of featureArg", required=false, exclusiveOf="", validation="")
private var featureArgIndex: Seq[File] = Nil

/** VCFWriter */
@Output(fullName="out", shortName="o", doc="VCFWriter", required=false, exclusiveOf="", validation="")
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

/** Sample name to be included in the analysis. Can be specified multiple times. */
@Argument(fullName="setString", shortName="sn", doc="Sample name to be included in the analysis. Can be specified multiple times.", required=false, exclusiveOf="", validation="")
var setString: Seq[String] = Nil

/**
 * Short name of setString
 * @return Short name of setString
 */
def sn = this.setString

/**
 * Short name of setString
 * @param value Short name of setString
 */
def sn_=(value: Seq[String]) { this.setString = value }

/** Sample name to be included in the analysis. Can be specified multiple times. */
@Argument(fullName="setStringInitialized", shortName="setStringInitialized", doc="Sample name to be included in the analysis. Can be specified multiple times.", required=false, exclusiveOf="", validation="")
var setStringInitialized: Seq[String] = Nil

/** One or more criteria to use when selecting the data.  Evaluated *after* the specified samples are extracted and the INFO-field annotations are updated. */
@Argument(fullName="select_expressions", shortName="optionalArgWithMissinglessDefault", doc="One or more criteria to use when selecting the data.  Evaluated *after* the specified samples are extracted and the INFO-field annotations are updated.", required=false, exclusiveOf="", validation="")
var select_expressions: Seq[String] = Nil

/**
 * Short name of select_expressions
 * @return Short name of select_expressions
 */
def optionalArgWithMissinglessDefault = this.select_expressions

/**
 * Short name of select_expressions
 * @param value Short name of select_expressions
 */
def optionalArgWithMissinglessDefault_=(value: Seq[String]) { this.select_expressions = value }

/** Should be the first argument */
@Argument(fullName="AAAAA", shortName="AAAAA", doc="Should be the first argument", required=false, exclusiveOf="", validation="")
var AAAAA: Boolean = _

/** Don't include loci found to be non-variant after the subsetting procedure. */
@Argument(fullName="booleanArg", shortName="env", doc="Don't include loci found to be non-variant after the subsetting procedure.", required=false, exclusiveOf="", validation="")
var booleanArg: Boolean = _

/**
 * Short name of booleanArg
 * @return Short name of booleanArg
 */
def env = this.booleanArg

/**
 * Short name of booleanArg
 * @param value Short name of booleanArg
 */
def env_=(value: Boolean) { this.booleanArg = value }

/** x */
@Argument(fullName="booleanArray", shortName="booleanArray", doc="x", required=false, exclusiveOf="", validation="")
var booleanArray: Seq[String] = Nil

/** Test enum */
@Argument(fullName="enumTest", shortName="enumTest", doc="Test enum", required=false, exclusiveOf="", validation="")
var enumTest: org.broadinstitute.gatk.tools.walkers.qc.DocumentationTest.TestEnum = _

/** Don't include loci found to be non-variant after the subsetting procedure. */
@Argument(fullName="hiddenArg", shortName="keepAF", doc="Don't include loci found to be non-variant after the subsetting procedure.", required=false, exclusiveOf="", validation="")
var hiddenArg: Boolean = _

/**
 * Short name of hiddenArg
 * @return Short name of hiddenArg
 */
def keepAF = this.hiddenArg

/**
 * Short name of hiddenArg
 * @param value Short name of hiddenArg
 */
def keepAF_=(value: Boolean) { this.hiddenArg = value }

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
listofRodBindingIndexes ++= listofRodBinding.filter(orig => orig != null && (!orig.getName.endsWith(".list"))).map(orig => new File(orig.getPath + ".idx"))
if (optionalRodBinding != null)
  optionalRodBindingIndex :+= new File(optionalRodBinding.getPath + ".idx")
if (optionalRodBindingWithoutDefault != null)
  optionalRodBindingWithoutDefaultIndex :+= new File(optionalRodBindingWithoutDefault.getPath + ".idx")
if (optionalRodBindingWithoutDefaultNull != null)
  optionalRodBindingWithoutDefaultNullIndex :+= new File(optionalRodBindingWithoutDefaultNull.getPath + ".idx")
if (featureArg != null)
  featureArgIndex :+= new File(featureArg.getPath + ".idx")
if (out != null && !org.broadinstitute.gatk.utils.io.IOUtils.isSpecialFile(out))
  if (!org.broadinstitute.gatk.utils.commandline.ArgumentTypeDescriptor.isCompressed(out.getPath))
    outIndex = new File(out.getPath + ".idx")
}

override def commandLine = super.commandLine + required(TaggedFile.formatCommandLineParameter("-V", variant), variant, spaceSeparated=true, escape=true, format="%s") + optional(TaggedFile.formatCommandLineParameter("-D", dbsnp), dbsnp, spaceSeparated=true, escape=true, format="%s") + repeat("-disc", listofRodBinding, formatPrefix=TaggedFile.formatCommandLineParameter, spaceSeparated=true, escape=true, format="%s") + optional(TaggedFile.formatCommandLineParameter("-conc", optionalRodBinding), optionalRodBinding, spaceSeparated=true, escape=true, format="%s") + optional(TaggedFile.formatCommandLineParameter("-optionalRodBindingWithoutDefault", optionalRodBindingWithoutDefault), optionalRodBindingWithoutDefault, spaceSeparated=true, escape=true, format="%s") + optional(TaggedFile.formatCommandLineParameter("-shortTest", optionalRodBindingWithoutDefaultNull), optionalRodBindingWithoutDefaultNull, spaceSeparated=true, escape=true, format="%s") + optional(TaggedFile.formatCommandLineParameter("-featureArg", featureArg), featureArg, spaceSeparated=true, escape=true, format="%s") + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + repeat("-sn", setString, spaceSeparated=true, escape=true, format="%s") + repeat("-setStringInitialized", setStringInitialized, spaceSeparated=true, escape=true, format="%s") + repeat("-optionalArgWithMissinglessDefault", select_expressions, spaceSeparated=true, escape=true, format="%s") + conditional(AAAAA, "-AAAAA", escape=true, format="%s") + conditional(booleanArg, "-env", escape=true, format="%s") + repeat("-booleanArray", booleanArray, spaceSeparated=true, escape=true, format="%s") + optional("-enumTest", enumTest, spaceSeparated=true, escape=true, format="%s") + conditional(hiddenArg, "-keepAF", escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
