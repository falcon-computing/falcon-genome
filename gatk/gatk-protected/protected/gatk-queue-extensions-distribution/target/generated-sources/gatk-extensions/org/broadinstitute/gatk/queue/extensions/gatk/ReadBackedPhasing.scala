package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class ReadBackedPhasing extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "ReadBackedPhasing"
analysis_type = "ReadBackedPhasing"
scatterClass = classOf[LocusScatterFunction]
setupScatterFunction = { case scatter: GATKScatterFunction => scatter.includeUnmapped = false }

/** If specified, print out very verbose debug information (if -l DEBUG is also specified) */
@Argument(fullName="debug", shortName="debug", doc="If specified, print out very verbose debug information (if -l DEBUG is also specified)", required=false, exclusiveOf="", validation="")
var debug: Boolean = _

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

/** The window size (in bases) to cache variant sites and their reads for the phasing procedure */
@Argument(fullName="cacheWindowSize", shortName="cacheWindow", doc="The window size (in bases) to cache variant sites and their reads for the phasing procedure", required=false, exclusiveOf="", validation="")
var cacheWindowSize: Option[Int] = None

/**
 * Short name of cacheWindowSize
 * @return Short name of cacheWindowSize
 */
def cacheWindow = this.cacheWindowSize

/**
 * Short name of cacheWindowSize
 * @param value Short name of cacheWindowSize
 */
def cacheWindow_=(value: Option[Int]) { this.cacheWindowSize = value }

/** The maximum number of successive heterozygous sites permitted to be used by the phasing algorithm */
@Argument(fullName="maxPhaseSites", shortName="maxSites", doc="The maximum number of successive heterozygous sites permitted to be used by the phasing algorithm", required=false, exclusiveOf="", validation="")
var maxPhaseSites: Option[Int] = None

/**
 * Short name of maxPhaseSites
 * @return Short name of maxPhaseSites
 */
def maxSites = this.maxPhaseSites

/**
 * Short name of maxPhaseSites
 * @param value Short name of maxPhaseSites
 */
def maxSites_=(value: Option[Int]) { this.maxPhaseSites = value }

/** The minimum phasing quality score required to output phasing */
@Argument(fullName="phaseQualityThresh", shortName="phaseThresh", doc="The minimum phasing quality score required to output phasing", required=false, exclusiveOf="", validation="")
var phaseQualityThresh: Option[Double] = None

/**
 * Short name of phaseQualityThresh
 * @return Short name of phaseQualityThresh
 */
def phaseThresh = this.phaseQualityThresh

/**
 * Short name of phaseQualityThresh
 * @param value Short name of phaseQualityThresh
 */
def phaseThresh_=(value: Option[Double]) { this.phaseQualityThresh = value }

/** Format string for phaseQualityThresh */
@Argument(fullName="phaseQualityThreshFormat", shortName="", doc="Format string for phaseQualityThresh", required=false, exclusiveOf="", validation="")
var phaseQualityThreshFormat: String = "%s"

/** The prefix of the VCF/phasing statistics files [For DEBUGGING purposes only - DO NOT USE!] */
@Argument(fullName="variantStatsFilePrefix", shortName="variantStats", doc="The prefix of the VCF/phasing statistics files [For DEBUGGING purposes only - DO NOT USE!]", required=false, exclusiveOf="", validation="")
var variantStatsFilePrefix: String = _

/**
 * Short name of variantStatsFilePrefix
 * @return Short name of variantStatsFilePrefix
 */
def variantStats = this.variantStatsFilePrefix

/**
 * Short name of variantStatsFilePrefix
 * @param value Short name of variantStatsFilePrefix
 */
def variantStats_=(value: String) { this.variantStatsFilePrefix = value }

/** Minimum base quality required to consider a base for phasing */
@Argument(fullName="min_base_quality_score", shortName="mbq", doc="Minimum base quality required to consider a base for phasing", required=false, exclusiveOf="", validation="")
var min_base_quality_score: Option[Int] = None

/**
 * Short name of min_base_quality_score
 * @return Short name of min_base_quality_score
 */
def mbq = this.min_base_quality_score

/**
 * Short name of min_base_quality_score
 * @param value Short name of min_base_quality_score
 */
def mbq_=(value: Option[Int]) { this.min_base_quality_score = value }

/** Minimum read mapping quality required to consider a read for phasing */
@Argument(fullName="min_mapping_quality_score", shortName="mmq", doc="Minimum read mapping quality required to consider a read for phasing", required=false, exclusiveOf="", validation="")
var min_mapping_quality_score: Option[Int] = None

/**
 * Short name of min_mapping_quality_score
 * @return Short name of min_mapping_quality_score
 */
def mmq = this.min_mapping_quality_score

/**
 * Short name of min_mapping_quality_score
 * @param value Short name of min_mapping_quality_score
 */
def mmq_=(value: Option[Int]) { this.min_mapping_quality_score = value }

/** Only include these samples when phasing */
@Argument(fullName="sampleToPhase", shortName="sampleToPhase", doc="Only include these samples when phasing", required=false, exclusiveOf="", validation="")
var sampleToPhase: Seq[String] = Nil

/** Don't exit (just WARN) when the VCF and BAMs do not overlap in samples */
@Argument(fullName="permitNoSampleOverlap", shortName="permitNoSampleOverlap", doc="Don't exit (just WARN) when the VCF and BAMs do not overlap in samples", required=false, exclusiveOf="", validation="")
var permitNoSampleOverlap: Boolean = _

/** Merge consecutive phased sites into MNP records */
@Argument(fullName="enableMergePhasedSegregatingPolymorphismsToMNP", shortName="enableMergeToMNP", doc="Merge consecutive phased sites into MNP records", required=false, exclusiveOf="", validation="")
var enableMergePhasedSegregatingPolymorphismsToMNP: Boolean = _

/**
 * Short name of enableMergePhasedSegregatingPolymorphismsToMNP
 * @return Short name of enableMergePhasedSegregatingPolymorphismsToMNP
 */
def enableMergeToMNP = this.enableMergePhasedSegregatingPolymorphismsToMNP

/**
 * Short name of enableMergePhasedSegregatingPolymorphismsToMNP
 * @param value Short name of enableMergePhasedSegregatingPolymorphismsToMNP
 */
def enableMergeToMNP_=(value: Boolean) { this.enableMergePhasedSegregatingPolymorphismsToMNP = value }

/** The maximum reference-genome distance between consecutive heterozygous sites to permit merging phased VCF records into a MNP record */
@Argument(fullName="maxGenomicDistanceForMNP", shortName="maxDistMNP", doc="The maximum reference-genome distance between consecutive heterozygous sites to permit merging phased VCF records into a MNP record", required=false, exclusiveOf="", validation="")
var maxGenomicDistanceForMNP: Option[Int] = None

/**
 * Short name of maxGenomicDistanceForMNP
 * @return Short name of maxGenomicDistanceForMNP
 */
def maxDistMNP = this.maxGenomicDistanceForMNP

/**
 * Short name of maxGenomicDistanceForMNP
 * @param value Short name of maxGenomicDistanceForMNP
 */
def maxDistMNP_=(value: Option[Int]) { this.maxGenomicDistanceForMNP = value }

/** File to output cases where a single read has multiple bases at the same position [For DEBUGGING purposes only - DO NOT USE!] */
@Argument(fullName="outputMultipleBaseCountsFile", shortName="outputMultipleBaseCountsFile", doc="File to output cases where a single read has multiple bases at the same position [For DEBUGGING purposes only - DO NOT USE!]", required=false, exclusiveOf="", validation="")
var outputMultipleBaseCountsFile: File = _

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

override def commandLine = super.commandLine + conditional(debug, "-debug", escape=true, format="%s") + required(TaggedFile.formatCommandLineParameter("-V", variant), variant, spaceSeparated=true, escape=true, format="%s") + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + optional("-cacheWindow", cacheWindowSize, spaceSeparated=true, escape=true, format="%s") + optional("-maxSites", maxPhaseSites, spaceSeparated=true, escape=true, format="%s") + optional("-phaseThresh", phaseQualityThresh, spaceSeparated=true, escape=true, format=phaseQualityThreshFormat) + optional("-variantStats", variantStatsFilePrefix, spaceSeparated=true, escape=true, format="%s") + optional("-mbq", min_base_quality_score, spaceSeparated=true, escape=true, format="%s") + optional("-mmq", min_mapping_quality_score, spaceSeparated=true, escape=true, format="%s") + repeat("-sampleToPhase", sampleToPhase, spaceSeparated=true, escape=true, format="%s") + conditional(permitNoSampleOverlap, "-permitNoSampleOverlap", escape=true, format="%s") + conditional(enableMergePhasedSegregatingPolymorphismsToMNP, "-enableMergeToMNP", escape=true, format="%s") + optional("-maxDistMNP", maxGenomicDistanceForMNP, spaceSeparated=true, escape=true, format="%s") + optional("-outputMultipleBaseCountsFile", outputMultipleBaseCountsFile, spaceSeparated=true, escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
