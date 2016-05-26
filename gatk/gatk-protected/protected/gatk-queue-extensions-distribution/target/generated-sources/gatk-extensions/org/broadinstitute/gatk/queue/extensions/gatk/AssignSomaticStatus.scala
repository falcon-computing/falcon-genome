package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class AssignSomaticStatus extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "AssignSomaticStatus"
analysis_type = "AssignSomaticStatus"
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

/** The normal sample */
@Argument(fullName="normalSample", shortName="n", doc="The normal sample", required=true, exclusiveOf="", validation="")
var normalSample: String = _

/**
 * Short name of normalSample
 * @return Short name of normalSample
 */
def n = this.normalSample

/**
 * Short name of normalSample
 * @param value Short name of normalSample
 */
def n_=(value: String) { this.normalSample = value }

/** The tumor sample */
@Argument(fullName="tumorSample", shortName="t", doc="The tumor sample", required=true, exclusiveOf="", validation="")
var tumorSample: String = _

/**
 * Short name of tumorSample
 * @return Short name of tumorSample
 */
def t = this.tumorSample

/**
 * Short name of tumorSample
 * @param value Short name of tumorSample
 */
def t_=(value: String) { this.tumorSample = value }

/** Phred-scaled probability that a site is a somatic mutation */
@Argument(fullName="somaticPriorQ", shortName="somaticPriorQ", doc="Phred-scaled probability that a site is a somatic mutation", required=false, exclusiveOf="", validation="")
var somaticPriorQ: Option[Byte] = None

/** Phred-scaled min probability that a site should be called somatic mutation */
@Argument(fullName="somaticMinLOD", shortName="somaticMinLOD", doc="Phred-scaled min probability that a site should be called somatic mutation", required=false, exclusiveOf="", validation="")
var somaticMinLOD: Option[Byte] = None

/** If provided, the attributes of the output VCF will only contain the somatic status fields */
@Argument(fullName="minimalVCF", shortName="minimalVCF", doc="If provided, the attributes of the output VCF will only contain the somatic status fields", required=false, exclusiveOf="", validation="")
var minimalVCF: Boolean = _

/** An output file created by the walker.  Will overwrite contents if file exists */
@Output(fullName="out", shortName="o", doc="An output file created by the walker.  Will overwrite contents if file exists", required=false, exclusiveOf="", validation="")
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
if (out != null && !org.broadinstitute.gatk.utils.io.IOUtils.isSpecialFile(out))
  if (!org.broadinstitute.gatk.utils.commandline.ArgumentTypeDescriptor.isCompressed(out.getPath))
    outIndex = new File(out.getPath + ".idx")
}

override def commandLine = super.commandLine + required(TaggedFile.formatCommandLineParameter("-V", variant), variant, spaceSeparated=true, escape=true, format="%s") + required("-n", normalSample, spaceSeparated=true, escape=true, format="%s") + required("-t", tumorSample, spaceSeparated=true, escape=true, format="%s") + optional("-somaticPriorQ", somaticPriorQ, spaceSeparated=true, escape=true, format="%s") + optional("-somaticMinLOD", somaticMinLOD, spaceSeparated=true, escape=true, format="%s") + conditional(minimalVCF, "-minimalVCF", escape=true, format="%s") + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
