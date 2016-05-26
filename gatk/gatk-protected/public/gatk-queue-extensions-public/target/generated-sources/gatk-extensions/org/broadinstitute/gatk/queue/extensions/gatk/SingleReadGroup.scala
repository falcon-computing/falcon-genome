package org.broadinstitute.gatk.queue.extensions.gatk

import org.broadinstitute.gatk.utils.commandline.Argument

trait SingleReadGroup extends org.broadinstitute.gatk.queue.function.CommandLineFunction {

/** The name of the read group to keep, filtering out all others */
@Argument(fullName="read_group_to_keep", shortName="goodRG", doc="The name of the read group to keep, filtering out all others", required=true, exclusiveOf="", validation="")
var read_group_to_keep: String = _

/**
 * Short name of read_group_to_keep
 * @return Short name of read_group_to_keep
 */
def goodRG = this.read_group_to_keep

/**
 * Short name of read_group_to_keep
 * @param value Short name of read_group_to_keep
 */
def goodRG_=(value: String) { this.read_group_to_keep = value }

abstract override def commandLine = super.commandLine + required("--read_filter", "SingleReadGroup") + required("-goodRG", read_group_to_keep, spaceSeparated=true, escape=true, format="%s")
}
