package org.broadinstitute.gatk.queue.extensions.gatk

import org.broadinstitute.gatk.utils.commandline.Argument

trait OverclippedRead extends org.broadinstitute.gatk.queue.function.CommandLineFunction {

/** Value for which reads with less than this number of aligned bases is considered too short */
@Argument(fullName="filter_is_too_short_value", shortName="filterTooShort", doc="Value for which reads with less than this number of aligned bases is considered too short", required=false, exclusiveOf="", validation="")
var filter_is_too_short_value: Option[Int] = None

/**
 * Short name of filter_is_too_short_value
 * @return Short name of filter_is_too_short_value
 */
def filterTooShort = this.filter_is_too_short_value

/**
 * Short name of filter_is_too_short_value
 * @param value Short name of filter_is_too_short_value
 */
def filterTooShort_=(value: Option[Int]) { this.filter_is_too_short_value = value }

/** Allow a read to be filtered out based on having only 1 soft-clipped block. By default, both ends must have a soft-clipped block, setting this flag requires only 1 soft-clipped block. */
@Argument(fullName="do_not_require_softclips_both_ends", shortName="NoRequireSCBothEnds", doc="Allow a read to be filtered out based on having only 1 soft-clipped block. By default, both ends must have a soft-clipped block, setting this flag requires only 1 soft-clipped block.", required=false, exclusiveOf="", validation="")
var do_not_require_softclips_both_ends: Boolean = _

/**
 * Short name of do_not_require_softclips_both_ends
 * @return Short name of do_not_require_softclips_both_ends
 */
def NoRequireSCBothEnds = this.do_not_require_softclips_both_ends

/**
 * Short name of do_not_require_softclips_both_ends
 * @param value Short name of do_not_require_softclips_both_ends
 */
def NoRequireSCBothEnds_=(value: Boolean) { this.do_not_require_softclips_both_ends = value }

abstract override def commandLine = super.commandLine + required("--read_filter", "OverclippedRead") + optional("-filterTooShort", filter_is_too_short_value, spaceSeparated=true, escape=true, format="%s") + conditional(do_not_require_softclips_both_ends, "-NoRequireSCBothEnds", escape=true, format="%s")
}
