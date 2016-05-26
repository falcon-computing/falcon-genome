package org.broadinstitute.gatk.queue.extensions.gatk

import org.broadinstitute.gatk.utils.commandline.Argument

trait LibraryRead extends org.broadinstitute.gatk.queue.function.CommandLineFunction {

/** The name of the library to keep, filtering out all others */
@Argument(fullName="library", shortName="library", doc="The name of the library to keep, filtering out all others", required=true, exclusiveOf="", validation="")
var library: String = _

abstract override def commandLine = super.commandLine + required("--read_filter", "LibraryRead") + required("-library", library, spaceSeparated=true, escape=true, format="%s")
}
