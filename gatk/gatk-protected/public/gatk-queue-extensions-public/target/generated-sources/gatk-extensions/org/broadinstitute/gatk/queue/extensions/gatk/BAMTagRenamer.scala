package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Output

class BAMTagRenamer extends org.broadinstitute.gatk.queue.function.JavaCommandLineFunction {
analysisName = "BAMTagRenamer"
javaMainClass = "org.broadinstitute.gatk.engine.datasources.reads.utilities.BAMTagRenamer"

/** Input file to process */
@Argument(fullName="input", shortName="I", doc="Input file to process", required=true, exclusiveOf="", validation="")
var input: File = _

/**
 * Short name of input
 * @return Short name of input
 */
def I = this.input

/**
 * Short name of input
 * @param value Short name of input
 */
def I_=(value: File) { this.input = value }

/** Output file to create */
@Argument(fullName="output", shortName="O", doc="Output file to create", required=true, exclusiveOf="", validation="")
var output: File = _

/**
 * Short name of output
 * @return Short name of output
 */
def O = this.output

/**
 * Short name of output
 * @param value Short name of output
 */
def O_=(value: File) { this.output = value }

/** Compression level to use when writing the BAM file. */
@Argument(fullName="bam_compression", shortName="compress", doc="Compression level to use when writing the BAM file.", required=false, exclusiveOf="", validation="")
var bam_compression: Option[Int] = None

/**
 * Short name of bam_compression
 * @return Short name of bam_compression
 */
def compress = this.bam_compression

/**
 * Short name of bam_compression
 * @param value Short name of bam_compression
 */
def compress_=(value: Option[Int]) { this.bam_compression = value }

/** Tag name to be replaced. */
@Argument(fullName="original_tag_name", shortName="otn", doc="Tag name to be replaced.", required=true, exclusiveOf="", validation="")
var original_tag_name: String = _

/**
 * Short name of original_tag_name
 * @return Short name of original_tag_name
 */
def otn = this.original_tag_name

/**
 * Short name of original_tag_name
 * @param value Short name of original_tag_name
 */
def otn_=(value: String) { this.original_tag_name = value }

/** Tag name to be used as a replacement. */
@Argument(fullName="replacement_tag_name", shortName="rtn", doc="Tag name to be used as a replacement.", required=true, exclusiveOf="", validation="")
var replacement_tag_name: String = _

/**
 * Short name of replacement_tag_name
 * @return Short name of replacement_tag_name
 */
def rtn = this.replacement_tag_name

/**
 * Short name of replacement_tag_name
 * @param value Short name of replacement_tag_name
 */
def rtn_=(value: String) { this.replacement_tag_name = value }

/** Set the minimum level of logging */
@Argument(fullName="logging_level", shortName="l", doc="Set the minimum level of logging", required=false, exclusiveOf="", validation="")
var logging_level: String = _

/**
 * Short name of logging_level
 * @return Short name of logging_level
 */
def l = this.logging_level

/**
 * Short name of logging_level
 * @param value Short name of logging_level
 */
def l_=(value: String) { this.logging_level = value }

/** Set the logging location */
@Output(fullName="log_to_file", shortName="log", doc="Set the logging location", required=false, exclusiveOf="", validation="")
@Gather(classOf[org.broadinstitute.gatk.queue.function.scattergather.SimpleTextGatherFunction])
var log_to_file: File = _

/**
 * Short name of log_to_file
 * @return Short name of log_to_file
 */
def log = this.log_to_file

/**
 * Short name of log_to_file
 * @param value Short name of log_to_file
 */
def log_=(value: File) { this.log_to_file = value }

/** Generate the help message */
@Argument(fullName="help", shortName="h", doc="Generate the help message", required=false, exclusiveOf="", validation="")
var help: Boolean = _

/**
 * Short name of help
 * @return Short name of help
 */
def h = this.help

/**
 * Short name of help
 * @param value Short name of help
 */
def h_=(value: Boolean) { this.help = value }

/** Output version information */
@Argument(fullName="version", shortName="version", doc="Output version information", required=false, exclusiveOf="", validation="")
var version: Boolean = _

override def commandLine = super.commandLine + required("-I", input, spaceSeparated=true, escape=true, format="%s") + required("-O", output, spaceSeparated=true, escape=true, format="%s") + optional("-compress", bam_compression, spaceSeparated=true, escape=true, format="%s") + required("-otn", original_tag_name, spaceSeparated=true, escape=true, format="%s") + required("-rtn", replacement_tag_name, spaceSeparated=true, escape=true, format="%s") + optional("-l", logging_level, spaceSeparated=true, escape=true, format="%s") + optional("-log", log_to_file, spaceSeparated=true, escape=true, format="%s") + conditional(help, "-h", escape=true, format="%s") + conditional(version, "-version", escape=true, format="%s")
}
