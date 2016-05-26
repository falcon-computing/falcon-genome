package org.broadinstitute.gatk.queue.extensions.gatk

import org.broadinstitute.gatk.utils.commandline.Argument

trait Sample extends org.broadinstitute.gatk.queue.function.CommandLineFunction {

/** The name of the sample(s) to keep, filtering out all others */
@Argument(fullName="sample_to_keep", shortName="goodSM", doc="The name of the sample(s) to keep, filtering out all others", required=true, exclusiveOf="", validation="")
var sample_to_keep: Seq[String] = Nil

/**
 * Short name of sample_to_keep
 * @return Short name of sample_to_keep
 */
def goodSM = this.sample_to_keep

/**
 * Short name of sample_to_keep
 * @param value Short name of sample_to_keep
 */
def goodSM_=(value: Seq[String]) { this.sample_to_keep = value }

abstract override def commandLine = super.commandLine + required("--read_filter", "Sample") + repeat("-goodSM", sample_to_keep, spaceSeparated=true, escape=true, format="%s")
}
