package org.broadinstitute.gatk.queue.extensions.gatk

import org.broadinstitute.gatk.utils.commandline.Argument

trait ReadName extends org.broadinstitute.gatk.queue.function.CommandLineFunction {

/** Read name to whitelist */
@Argument(fullName="readName", shortName="rn", doc="Read name to whitelist", required=true, exclusiveOf="", validation="")
var readName: String = _

/**
 * Short name of readName
 * @return Short name of readName
 */
def rn = this.readName

/**
 * Short name of readName
 * @param value Short name of readName
 */
def rn_=(value: String) { this.readName = value }

abstract override def commandLine = super.commandLine + required("--read_filter", "ReadName") + required("-rn", readName, spaceSeparated=true, escape=true, format="%s")
}
