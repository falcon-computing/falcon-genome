package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class GenotypeAndValidate extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "GenotypeAndValidate"
analysis_type = "GenotypeAndValidate"
scatterClass = classOf[LocusScatterFunction]
setupScatterFunction = { case scatter: GATKScatterFunction => scatter.includeUnmapped = false }

/** Output VCF file with annotated variants */
@Output(fullName="out", shortName="o", doc="Output VCF file with annotated variants", required=false, exclusiveOf="", validation="")
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

/** The set of alleles at which to genotype */
@Input(fullName="alleles", shortName="alleles", doc="The set of alleles at which to genotype", required=true, exclusiveOf="", validation="")
var alleles: File = _

/** Dependencies on the index of alleles */
@Input(fullName="allelesIndex", shortName="", doc="Dependencies on the index of alleles", required=false, exclusiveOf="", validation="")
private var allelesIndex: Seq[File] = Nil

/** Use the calls on the reads (bam file) as the truth dataset and validate the calls on the VCF */
@Argument(fullName="set_bam_truth", shortName="bt", doc="Use the calls on the reads (bam file) as the truth dataset and validate the calls on the VCF", required=false, exclusiveOf="", validation="")
var set_bam_truth: Boolean = _

/**
 * Short name of set_bam_truth
 * @return Short name of set_bam_truth
 */
def bt = this.set_bam_truth

/**
 * Short name of set_bam_truth
 * @param value Short name of set_bam_truth
 */
def bt_=(value: Boolean) { this.set_bam_truth = value }

/** Minimum base quality score for calling a genotype */
@Argument(fullName="minimum_base_quality_score", shortName="mbq", doc="Minimum base quality score for calling a genotype", required=false, exclusiveOf="", validation="")
var minimum_base_quality_score: Option[Int] = None

/**
 * Short name of minimum_base_quality_score
 * @return Short name of minimum_base_quality_score
 */
def mbq = this.minimum_base_quality_score

/**
 * Short name of minimum_base_quality_score
 * @param value Short name of minimum_base_quality_score
 */
def mbq_=(value: Option[Int]) { this.minimum_base_quality_score = value }

/** Maximum deletion fraction for calling a genotype */
@Argument(fullName="maximum_deletion_fraction", shortName="deletions", doc="Maximum deletion fraction for calling a genotype", required=false, exclusiveOf="", validation="")
var maximum_deletion_fraction: Option[Double] = None

/**
 * Short name of maximum_deletion_fraction
 * @return Short name of maximum_deletion_fraction
 */
def deletions = this.maximum_deletion_fraction

/**
 * Short name of maximum_deletion_fraction
 * @param value Short name of maximum_deletion_fraction
 */
def deletions_=(value: Option[Double]) { this.maximum_deletion_fraction = value }

/** Format string for maximum_deletion_fraction */
@Argument(fullName="maximum_deletion_fractionFormat", shortName="", doc="Format string for maximum_deletion_fraction", required=false, exclusiveOf="", validation="")
var maximum_deletion_fractionFormat: String = "%s"

/** the minimum phred-scaled Qscore threshold to separate high confidence from low confidence calls */
@Argument(fullName="standard_min_confidence_threshold_for_calling", shortName="stand_call_conf", doc="the minimum phred-scaled Qscore threshold to separate high confidence from low confidence calls", required=false, exclusiveOf="", validation="")
var standard_min_confidence_threshold_for_calling: Option[Double] = None

/**
 * Short name of standard_min_confidence_threshold_for_calling
 * @return Short name of standard_min_confidence_threshold_for_calling
 */
def stand_call_conf = this.standard_min_confidence_threshold_for_calling

/**
 * Short name of standard_min_confidence_threshold_for_calling
 * @param value Short name of standard_min_confidence_threshold_for_calling
 */
def stand_call_conf_=(value: Option[Double]) { this.standard_min_confidence_threshold_for_calling = value }

/** Format string for standard_min_confidence_threshold_for_calling */
@Argument(fullName="standard_min_confidence_threshold_for_callingFormat", shortName="", doc="Format string for standard_min_confidence_threshold_for_calling", required=false, exclusiveOf="", validation="")
var standard_min_confidence_threshold_for_callingFormat: String = "%s"

/** the minimum phred-scaled Qscore threshold to emit low confidence calls */
@Argument(fullName="standard_min_confidence_threshold_for_emitting", shortName="stand_emit_conf", doc="the minimum phred-scaled Qscore threshold to emit low confidence calls", required=false, exclusiveOf="", validation="")
var standard_min_confidence_threshold_for_emitting: Option[Double] = None

/**
 * Short name of standard_min_confidence_threshold_for_emitting
 * @return Short name of standard_min_confidence_threshold_for_emitting
 */
def stand_emit_conf = this.standard_min_confidence_threshold_for_emitting

/**
 * Short name of standard_min_confidence_threshold_for_emitting
 * @param value Short name of standard_min_confidence_threshold_for_emitting
 */
def stand_emit_conf_=(value: Option[Double]) { this.standard_min_confidence_threshold_for_emitting = value }

/** Format string for standard_min_confidence_threshold_for_emitting */
@Argument(fullName="standard_min_confidence_threshold_for_emittingFormat", shortName="", doc="Format string for standard_min_confidence_threshold_for_emitting", required=false, exclusiveOf="", validation="")
var standard_min_confidence_threshold_for_emittingFormat: String = "%s"

/** Condition validation on a minimum depth of coverage by the reads */
@Argument(fullName="condition_on_depth", shortName="depth", doc="Condition validation on a minimum depth of coverage by the reads", required=false, exclusiveOf="", validation="")
var condition_on_depth: Option[Int] = None

/**
 * Short name of condition_on_depth
 * @return Short name of condition_on_depth
 */
def depth = this.condition_on_depth

/**
 * Short name of condition_on_depth
 * @param value Short name of condition_on_depth
 */
def depth_=(value: Option[Int]) { this.condition_on_depth = value }

/** Print out interesting sites to standard out */
@Argument(fullName="print_interesting_sites", shortName="print_interesting", doc="Print out interesting sites to standard out", required=false, exclusiveOf="", validation="")
var print_interesting_sites: Boolean = _

/**
 * Short name of print_interesting_sites
 * @return Short name of print_interesting_sites
 */
def print_interesting = this.print_interesting_sites

/**
 * Short name of print_interesting_sites
 * @param value Short name of print_interesting_sites
 */
def print_interesting_=(value: Boolean) { this.print_interesting_sites = value }

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
  if (!org.broadinstitute.gatk.utils.commandline.ArgumentTypeDescriptor.isCompressed(out.getPath))
    outIndex = new File(out.getPath + ".idx")
if (alleles != null)
  allelesIndex :+= new File(alleles.getPath + ".idx")
}

override def commandLine = super.commandLine + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + required(TaggedFile.formatCommandLineParameter("-alleles", alleles), alleles, spaceSeparated=true, escape=true, format="%s") + conditional(set_bam_truth, "-bt", escape=true, format="%s") + optional("-mbq", minimum_base_quality_score, spaceSeparated=true, escape=true, format="%s") + optional("-deletions", maximum_deletion_fraction, spaceSeparated=true, escape=true, format=maximum_deletion_fractionFormat) + optional("-stand_call_conf", standard_min_confidence_threshold_for_calling, spaceSeparated=true, escape=true, format=standard_min_confidence_threshold_for_callingFormat) + optional("-stand_emit_conf", standard_min_confidence_threshold_for_emitting, spaceSeparated=true, escape=true, format=standard_min_confidence_threshold_for_emittingFormat) + optional("-depth", condition_on_depth, spaceSeparated=true, escape=true, format="%s") + conditional(print_interesting_sites, "-print_interesting", escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
