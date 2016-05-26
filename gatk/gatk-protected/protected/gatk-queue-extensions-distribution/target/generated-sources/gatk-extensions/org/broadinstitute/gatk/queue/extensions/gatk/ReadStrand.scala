package org.broadinstitute.gatk.queue.extensions.gatk

import org.broadinstitute.gatk.utils.commandline.Argument

trait ReadStrand extends org.broadinstitute.gatk.queue.function.CommandLineFunction {

/** Discard reads on the forward strand */
@Argument(fullName="filterPositive", shortName="fp", doc="Discard reads on the forward strand", required=false, exclusiveOf="", validation="")
var filterPositive: Boolean = _

/**
 * Short name of filterPositive
 * @return Short name of filterPositive
 */
def fp = this.filterPositive

/**
 * Short name of filterPositive
 * @param value Short name of filterPositive
 */
def fp_=(value: Boolean) { this.filterPositive = value }

abstract override def commandLine = super.commandLine + required("--read_filter", "ReadStrand") + conditional(filterPositive, "-fp", escape=true, format="%s")
}
