package org.broadinstitute.gatk.queue.extensions.gatk

import org.broadinstitute.gatk.utils.commandline.Argument

trait ReassignMappingQuality extends org.broadinstitute.gatk.queue.function.CommandLineFunction {

/** Default read mapping quality to assign to all reads */
@Argument(fullName="default_mapping_quality", shortName="DMQ", doc="Default read mapping quality to assign to all reads", required=false, exclusiveOf="", validation="")
var default_mapping_quality: Option[Int] = None

/**
 * Short name of default_mapping_quality
 * @return Short name of default_mapping_quality
 */
def DMQ = this.default_mapping_quality

/**
 * Short name of default_mapping_quality
 * @param value Short name of default_mapping_quality
 */
def DMQ_=(value: Option[Int]) { this.default_mapping_quality = value }

abstract override def commandLine = super.commandLine + required("--read_filter", "ReassignMappingQuality") + optional("-DMQ", default_mapping_quality, spaceSeparated=true, escape=true, format="%s")
}
