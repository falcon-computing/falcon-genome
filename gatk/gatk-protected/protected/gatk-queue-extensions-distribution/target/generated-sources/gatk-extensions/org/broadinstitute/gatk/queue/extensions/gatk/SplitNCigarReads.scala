package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Output

class SplitNCigarReads extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "SplitNCigarReads"
analysis_type = "SplitNCigarReads"
scatterClass = classOf[ReadScatterFunction]
setupScatterFunction = { case scatter: GATKScatterFunction => scatter.includeUnmapped = true }

/** Write output to this BAM filename instead of STDOUT */
@Output(fullName="out", shortName="o", doc="Write output to this BAM filename instead of STDOUT", required=false, exclusiveOf="", validation="")
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

/** max reads allowed to be kept in memory at a time by the BAM writer */
@Argument(fullName="maxReadsInMemory", shortName="maxInMemory", doc="max reads allowed to be kept in memory at a time by the BAM writer", required=false, exclusiveOf="", validation="")
var maxReadsInMemory: Option[Int] = None

/**
 * Short name of maxReadsInMemory
 * @return Short name of maxReadsInMemory
 */
def maxInMemory = this.maxReadsInMemory

/**
 * Short name of maxReadsInMemory
 * @param value Short name of maxReadsInMemory
 */
def maxInMemory_=(value: Option[Int]) { this.maxReadsInMemory = value }

/** max number of mismatches allowed in the overhang */
@Argument(fullName="maxMismatchesInOverhang", shortName="maxMismatches", doc="max number of mismatches allowed in the overhang", required=false, exclusiveOf="", validation="")
var maxMismatchesInOverhang: Option[Int] = None

/**
 * Short name of maxMismatchesInOverhang
 * @return Short name of maxMismatchesInOverhang
 */
def maxMismatches = this.maxMismatchesInOverhang

/**
 * Short name of maxMismatchesInOverhang
 * @param value Short name of maxMismatchesInOverhang
 */
def maxMismatches_=(value: Option[Int]) { this.maxMismatchesInOverhang = value }

/** max number of bases allowed in the overhang */
@Argument(fullName="maxBasesInOverhang", shortName="maxOverhang", doc="max number of bases allowed in the overhang", required=false, exclusiveOf="", validation="")
var maxBasesInOverhang: Option[Int] = None

/**
 * Short name of maxBasesInOverhang
 * @return Short name of maxBasesInOverhang
 */
def maxOverhang = this.maxBasesInOverhang

/**
 * Short name of maxBasesInOverhang
 * @param value Short name of maxBasesInOverhang
 */
def maxOverhang_=(value: Option[Int]) { this.maxBasesInOverhang = value }

/** do not have the walker hard-clip overhanging sections of the reads */
@Argument(fullName="doNotFixOverhangs", shortName="doNotFixOverhangs", doc="do not have the walker hard-clip overhanging sections of the reads", required=false, exclusiveOf="", validation="")
var doNotFixOverhangs: Boolean = _

/** Necessary for integration tests */
@Argument(fullName="no_pg_tag", shortName="npt", doc="Necessary for integration tests", required=false, exclusiveOf="", validation="")
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
if (out != null && !org.broadinstitute.gatk.utils.io.IOUtils.isSpecialFile(out))
  if (!disable_bam_indexing)
    outIndex = new File(out.getPath.stripSuffix(".bam") + ".bai")
if (out != null && !org.broadinstitute.gatk.utils.io.IOUtils.isSpecialFile(out))
  if (generate_md5)
    outMD5 = new File(out.getPath + ".md5")
}

override def commandLine = super.commandLine + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + optional("-maxInMemory", maxReadsInMemory, spaceSeparated=true, escape=true, format="%s") + optional("-maxMismatches", maxMismatchesInOverhang, spaceSeparated=true, escape=true, format="%s") + optional("-maxOverhang", maxBasesInOverhang, spaceSeparated=true, escape=true, format="%s") + conditional(doNotFixOverhangs, "-doNotFixOverhangs", escape=true, format="%s") + conditional(no_pg_tag, "-npt", escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
