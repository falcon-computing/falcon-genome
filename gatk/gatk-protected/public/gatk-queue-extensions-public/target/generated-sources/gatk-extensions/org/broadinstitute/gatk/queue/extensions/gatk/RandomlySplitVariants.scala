package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class RandomlySplitVariants extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "RandomlySplitVariants"
analysis_type = "RandomlySplitVariants"
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

/** File #1 to which variants should be written */
@Output(fullName="out1", shortName="o1", doc="File #1 to which variants should be written", required=false, exclusiveOf="splitToManyFiles", validation="")
@Gather(classOf[CatVariantsGatherer])
var out1: File = _

/**
 * Short name of out1
 * @return Short name of out1
 */
def o1 = this.out1

/**
 * Short name of out1
 * @param value Short name of out1
 */
def o1_=(value: File) { this.out1 = value }

/** Automatically generated index for out1 */
@Output(fullName="out1Index", shortName="", doc="Automatically generated index for out1", required=false, exclusiveOf="", validation="")
@Gather(enabled=false)
private var out1Index: File = _

/** File #2 to which variants should be written */
@Output(fullName="out2", shortName="o2", doc="File #2 to which variants should be written", required=false, exclusiveOf="splitToManyFiles", validation="")
@Gather(classOf[org.broadinstitute.gatk.queue.function.scattergather.SimpleTextGatherFunction])
var out2: File = _

/**
 * Short name of out2
 * @return Short name of out2
 */
def o2 = this.out2

/**
 * Short name of out2
 * @param value Short name of out2
 */
def o2_=(value: File) { this.out2 = value }

/** Fraction of records to be placed in out1 (must be 0 >= fraction <= 1); all other records are placed in out2 */
@Argument(fullName="fractionToOut1", shortName="fraction", doc="Fraction of records to be placed in out1 (must be 0 >= fraction <= 1); all other records are placed in out2", required=false, exclusiveOf="", validation="")
var fractionToOut1: Option[Double] = None

/**
 * Short name of fractionToOut1
 * @return Short name of fractionToOut1
 */
def fraction = this.fractionToOut1

/**
 * Short name of fractionToOut1
 * @param value Short name of fractionToOut1
 */
def fraction_=(value: Option[Double]) { this.fractionToOut1 = value }

/** Format string for fractionToOut1 */
@Argument(fullName="fractionToOut1Format", shortName="", doc="Format string for fractionToOut1", required=false, exclusiveOf="", validation="")
var fractionToOut1Format: String = "%s"

/** split (with uniform distribution) to more than 2 files. numOfFiles and baseOutputName parameters are required */
@Argument(fullName="splitToManyFiles", shortName="splitToMany", doc="split (with uniform distribution) to more than 2 files. numOfFiles and baseOutputName parameters are required", required=false, exclusiveOf="", validation="")
var splitToManyFiles: Boolean = _

/**
 * Short name of splitToManyFiles
 * @return Short name of splitToManyFiles
 */
def splitToMany = this.splitToManyFiles

/**
 * Short name of splitToManyFiles
 * @param value Short name of splitToManyFiles
 */
def splitToMany_=(value: Boolean) { this.splitToManyFiles = value }

/** number of output VCF files. Only works with SplitToMany = true */
@Argument(fullName="numOfOutputVCFFiles", shortName="N", doc="number of output VCF files. Only works with SplitToMany = true", required=false, exclusiveOf="", validation="")
var numOfOutputVCFFiles: Option[Int] = None

/**
 * Short name of numOfOutputVCFFiles
 * @return Short name of numOfOutputVCFFiles
 */
def N = this.numOfOutputVCFFiles

/**
 * Short name of numOfOutputVCFFiles
 * @param value Short name of numOfOutputVCFFiles
 */
def N_=(value: Option[Int]) { this.numOfOutputVCFFiles = value }

/** the name of the output VCF file will be: <baseOutputName>.split.<number>.vcf. Required with SplitToMany option */
@Argument(fullName="prefixForAllOutputFileNames", shortName="baseOutputName", doc="the name of the output VCF file will be: <baseOutputName>.split.<number>.vcf. Required with SplitToMany option", required=false, exclusiveOf="", validation="")
var prefixForAllOutputFileNames: String = _

/**
 * Short name of prefixForAllOutputFileNames
 * @return Short name of prefixForAllOutputFileNames
 */
def baseOutputName = this.prefixForAllOutputFileNames

/**
 * Short name of prefixForAllOutputFileNames
 * @param value Short name of prefixForAllOutputFileNames
 */
def baseOutputName_=(value: String) { this.prefixForAllOutputFileNames = value }

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
if (out1 != null && !org.broadinstitute.gatk.utils.io.IOUtils.isSpecialFile(out1))
  if (!org.broadinstitute.gatk.utils.commandline.ArgumentTypeDescriptor.isCompressed(out1.getPath))
    out1Index = new File(out1.getPath + ".idx")
}

override def commandLine = super.commandLine + required(TaggedFile.formatCommandLineParameter("-V", variant), variant, spaceSeparated=true, escape=true, format="%s") + optional("-o1", out1, spaceSeparated=true, escape=true, format="%s") + optional("-o2", out2, spaceSeparated=true, escape=true, format="%s") + optional("-fraction", fractionToOut1, spaceSeparated=true, escape=true, format=fractionToOut1Format) + conditional(splitToManyFiles, "-splitToMany", escape=true, format="%s") + optional("-N", numOfOutputVCFFiles, spaceSeparated=true, escape=true, format="%s") + optional("-baseOutputName", prefixForAllOutputFileNames, spaceSeparated=true, escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
