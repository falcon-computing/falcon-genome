package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.queue.function.scattergather.ScatterGatherableFunction
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Input

class ValidateVariants extends org.broadinstitute.gatk.queue.extensions.gatk.CommandLineGATK with ScatterGatherableFunction {
analysisName = "ValidateVariants"
analysis_type = "ValidateVariants"
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

/** which validation type to exclude from a full strict validation */
@Argument(fullName="validationTypeToExclude", shortName="Xtype", doc="which validation type to exclude from a full strict validation", required=false, exclusiveOf="", validation="")
var validationTypeToExclude: Seq[org.broadinstitute.gatk.tools.walkers.variantutils.ValidateVariants.ValidationType] = Nil

/**
 * Short name of validationTypeToExclude
 * @return Short name of validationTypeToExclude
 */
def Xtype = this.validationTypeToExclude

/**
 * Short name of validationTypeToExclude
 * @param value Short name of validationTypeToExclude
 */
def Xtype_=(value: Seq[org.broadinstitute.gatk.tools.walkers.variantutils.ValidateVariants.ValidationType]) { this.validationTypeToExclude = value }

/** skip validation on filtered records */
@Argument(fullName="doNotValidateFilteredRecords", shortName="doNotValidateFilteredRecords", doc="skip validation on filtered records", required=false, exclusiveOf="", validation="")
var doNotValidateFilteredRecords: Boolean = _

/** just emit warnings on errors instead of terminating the run at the first instance */
@Argument(fullName="warnOnErrors", shortName="warnOnErrors", doc="just emit warnings on errors instead of terminating the run at the first instance", required=false, exclusiveOf="", validation="")
var warnOnErrors: Boolean = _

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
}

override def commandLine = super.commandLine + required(TaggedFile.formatCommandLineParameter("-V", variant), variant, spaceSeparated=true, escape=true, format="%s") + optional(TaggedFile.formatCommandLineParameter("-D", dbsnp), dbsnp, spaceSeparated=true, escape=true, format="%s") + repeat("-Xtype", validationTypeToExclude, spaceSeparated=true, escape=true, format="%s") + conditional(doNotValidateFilteredRecords, "-doNotValidateFilteredRecords", escape=true, format="%s") + conditional(warnOnErrors, "-warnOnErrors", escape=true, format="%s") + conditional(filter_reads_with_N_cigar, "-filterRNC", escape=true, format="%s") + conditional(filter_mismatching_base_and_quals, "-filterMBQ", escape=true, format="%s") + conditional(filter_bases_not_stored, "-filterNoBases", escape=true, format="%s")
}
