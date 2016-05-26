package org.broadinstitute.gatk.queue.extensions.gatk

import org.broadinstitute.gatk.utils.commandline.Argument

trait ReassignOneMappingQuality extends org.broadinstitute.gatk.queue.function.CommandLineFunction {

/** Original mapping quality */
@Argument(fullName="reassign_mapping_quality_from", shortName="RMQF", doc="Original mapping quality", required=false, exclusiveOf="", validation="")
var reassign_mapping_quality_from: Option[Int] = None

/**
 * Short name of reassign_mapping_quality_from
 * @return Short name of reassign_mapping_quality_from
 */
def RMQF = this.reassign_mapping_quality_from

/**
 * Short name of reassign_mapping_quality_from
 * @param value Short name of reassign_mapping_quality_from
 */
def RMQF_=(value: Option[Int]) { this.reassign_mapping_quality_from = value }

/** Desired mapping quality */
@Argument(fullName="reassign_mapping_quality_to", shortName="RMQT", doc="Desired mapping quality", required=false, exclusiveOf="", validation="")
var reassign_mapping_quality_to: Option[Int] = None

/**
 * Short name of reassign_mapping_quality_to
 * @return Short name of reassign_mapping_quality_to
 */
def RMQT = this.reassign_mapping_quality_to

/**
 * Short name of reassign_mapping_quality_to
 * @param value Short name of reassign_mapping_quality_to
 */
def RMQT_=(value: Option[Int]) { this.reassign_mapping_quality_to = value }

abstract override def commandLine = super.commandLine + required("--read_filter", "ReassignOneMappingQuality") + optional("-RMQF", reassign_mapping_quality_from, spaceSeparated=true, escape=true, format="%s") + optional("-RMQT", reassign_mapping_quality_to, spaceSeparated=true, escape=true, format="%s")
}
