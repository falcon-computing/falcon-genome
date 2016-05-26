package org.broadinstitute.gatk.queue.extensions.gatk

import org.broadinstitute.gatk.utils.commandline.Argument

trait Platform extends org.broadinstitute.gatk.queue.function.CommandLineFunction {

/** Discard reads with RG:PL attribute containing this string */
@Argument(fullName="PLFilterName", shortName="PLFilterName", doc="Discard reads with RG:PL attribute containing this string", required=false, exclusiveOf="", validation="")
var PLFilterName: Seq[String] = Nil

abstract override def commandLine = super.commandLine + required("--read_filter", "Platform") + repeat("-PLFilterName", PLFilterName, spaceSeparated=true, escape=true, format="%s")
}
