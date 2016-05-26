package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class SimulateReadsForVariants extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "SimulateReadsForVariants"
analysis_type = "SimulateReadsForVariants"
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

/** Reads corresponding to variants */
@Output(fullName="out", shortName="o", doc="Reads corresponding to variants", required=true, exclusiveOf="", validation="")
@Gather(classOf[BamGatherFunction])
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

/** Automatically generated md5 for out */
@Output(fullName="outMD5", shortName="", doc="Automatically generated md5 for out", required=false, exclusiveOf="", validation="")
@Gather(enabled=false)
private var outMD5: File = _

/** Read depth to generate */
@Argument(fullName="readDepth", shortName="DP", doc="Read depth to generate", required=false, exclusiveOf="", validation="")
var readDepth: Option[Int] = None

/**
 * Short name of readDepth
 * @return Short name of readDepth
 */
def DP = this.readDepth

/**
 * Short name of readDepth
 * @param value Short name of readDepth
 */
def DP_=(value: Option[Int]) { this.readDepth = value }

/** Base error rate (Phred-scaled) */
@Argument(fullName="errorRate", shortName="ER", doc="Base error rate (Phred-scaled)", required=false, exclusiveOf="", validation="")
var errorRate: Option[Int] = None

/**
 * Short name of errorRate
 * @return Short name of errorRate
 */
def ER = this.errorRate

/**
 * Short name of errorRate
 * @param value Short name of errorRate
 */
def ER_=(value: Option[Int]) { this.errorRate = value }

/** Read lengths (bp) */
@Argument(fullName="readLength", shortName="RL", doc="Read lengths (bp)", required=false, exclusiveOf="", validation="")
var readLength: Option[Int] = None

/**
 * Short name of readLength
 * @return Short name of readLength
 */
def RL = this.readLength

/**
 * Short name of readLength
 * @param value Short name of readLength
 */
def RL_=(value: Option[Int]) { this.readLength = value }

/** Use AF in VCF as event allele fraction  */
@Argument(fullName="useAFAsAlleleFraction", shortName="AF", doc="Use AF in VCF as event allele fraction ", required=false, exclusiveOf="", validation="")
var useAFAsAlleleFraction: Boolean = _

/**
 * Short name of useAFAsAlleleFraction
 * @return Short name of useAFAsAlleleFraction
 */
def AF = this.useAFAsAlleleFraction

/**
 * Short name of useAFAsAlleleFraction
 * @param value Short name of useAFAsAlleleFraction
 */
def AF_=(value: Boolean) { this.useAFAsAlleleFraction = value }

/** Sequencing platform */
@Argument(fullName="rgPlatform", shortName="RGPL", doc="Sequencing platform", required=false, exclusiveOf="", validation="")
var rgPlatform: org.broadinstitute.gatk.utils.NGSPlatform = _

/**
 * Short name of rgPlatform
 * @return Short name of rgPlatform
 */
def RGPL = this.rgPlatform

/**
 * Short name of rgPlatform
 * @param value Short name of rgPlatform
 */
def RGPL_=(value: org.broadinstitute.gatk.utils.NGSPlatform) { this.rgPlatform = value }

/** Sampling mode */
@Argument(fullName="readSamplingMode", shortName="RSM", doc="Sampling mode", required=false, exclusiveOf="", validation="")
var readSamplingMode: org.broadinstitute.gatk.tools.walkers.simulatereads.SimulateReadsForVariants.ReadSamplingMode = _

/**
 * Short name of readSamplingMode
 * @return Short name of readSamplingMode
 */
def RSM = this.readSamplingMode

/**
 * Short name of readSamplingMode
 * @param value Short name of readSamplingMode
 */
def RSM_=(value: org.broadinstitute.gatk.tools.walkers.simulatereads.SimulateReadsForVariants.ReadSamplingMode) { this.readSamplingMode = value }

/** Discard program tags, for integration tests */
@Argument(fullName="no_pg_tag", shortName="npt", doc="Discard program tags, for integration tests", required=false, exclusiveOf="", validation="")
var no_pg_tag: Boolean = _

/**
 * Short name of no_pg_tag
 * @return Short name of no_pg_tag
 */
def npt = this.no_pg_tag

/**
 * Short name of no_pg_tag
 * @param value Short name of no_pg_tag
 */
def npt_=(value: Boolean) { this.no_pg_tag = value }

/** Verbose */
@Argument(fullName="verbose", shortName="verbose", doc="Verbose", required=false, exclusiveOf="", validation="")
var verbose: Boolean = _

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
  if (!disable_bam_indexing)
    outIndex = new File(out.getPath.stripSuffix(".bam") + ".bai")
if (out != null && !org.broadinstitute.gatk.utils.io.IOUtils.isSpecialFile(out))
  if (generate_md5)
    outMD5 = new File(out.getPath + ".md5")
}

override def commandLine = super.commandLine + required(TaggedFile.formatCommandLineParameter("-V", variant), variant, spaceSeparated=true, escape=true, format="%s") + required("-o", out, spaceSeparated=true, escape=true, format="%s") + optional("-DP", readDepth, spaceSeparated=true, escape=true, format="%s") + optional("-ER", errorRate, spaceSeparated=true, escape=true, format="%s") + optional("-RL", readLength, spaceSeparated=true, escape=true, format="%s") + conditional(useAFAsAlleleFraction, "-AF", escape=true, format="%s") + optional("-RGPL", rgPlatform, spaceSeparated=true, escape=true, format="%s") + optional("-RSM", readSamplingMode, spaceSeparated=true, escape=true, format="%s") + conditional(no_pg_tag, "-npt", escape=true, format="%s") + conditional(verbose, "-verbose", escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
