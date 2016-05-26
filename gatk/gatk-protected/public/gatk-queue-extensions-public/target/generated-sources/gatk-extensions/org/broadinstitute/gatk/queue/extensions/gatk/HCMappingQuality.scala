package org.broadinstitute.gatk.queue.extensions.gatk

import org.broadinstitute.gatk.utils.commandline.Argument

trait HCMappingQuality extends org.broadinstitute.gatk.queue.function.CommandLineFunction {

/** Minimum read mapping quality required to consider a read for analysis with the HaplotypeCaller */
@Argument(fullName="min_mapping_quality_score", shortName="mmq", doc="Minimum read mapping quality required to consider a read for analysis with the HaplotypeCaller", required=false, exclusiveOf="", validation="")
var min_mapping_quality_score: Option[Int] = None

/**
 * Short name of min_mapping_quality_score
 * @return Short name of min_mapping_quality_score
 */
def mmq = this.min_mapping_quality_score

/**
 * Short name of min_mapping_quality_score
 * @param value Short name of min_mapping_quality_score
 */
def mmq_=(value: Option[Int]) { this.min_mapping_quality_score = value }

abstract override def commandLine = super.commandLine + required("--read_filter", "HCMappingQuality") + optional("-mmq", min_mapping_quality_score, spaceSeparated=true, escape=true, format="%s")
}
