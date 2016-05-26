package org.broadinstitute.gatk.queue.extensions.gatk

import org.broadinstitute.gatk.utils.commandline.Argument

trait ReadLength extends org.broadinstitute.gatk.queue.function.CommandLineFunction {

/** Discard reads with length greater than the specified value */
@Argument(fullName="maxReadLength", shortName="maxRead", doc="Discard reads with length greater than the specified value", required=true, exclusiveOf="", validation="")
var maxReadLength: Int = _

/**
 * Short name of maxReadLength
 * @return Short name of maxReadLength
 */
def maxRead = this.maxReadLength

/**
 * Short name of maxReadLength
 * @param value Short name of maxReadLength
 */
def maxRead_=(value: Int) { this.maxReadLength = value }

/** Discard reads with length shorter than the specified value */
@Argument(fullName="minReadLength", shortName="minRead", doc="Discard reads with length shorter than the specified value", required=true, exclusiveOf="", validation="")
var minReadLength: Int = _

/**
 * Short name of minReadLength
 * @return Short name of minReadLength
 */
def minRead = this.minReadLength

/**
 * Short name of minReadLength
 * @param value Short name of minReadLength
 */
def minRead_=(value: Int) { this.minReadLength = value }

abstract override def commandLine = super.commandLine + required("--read_filter", "ReadLength") + required("-maxRead", maxReadLength, spaceSeparated=true, escape=true, format="%s") + required("-minRead", minReadLength, spaceSeparated=true, escape=true, format="%s")
}
