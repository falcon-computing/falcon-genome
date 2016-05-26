package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class UnzipSingleBlock extends org.broadinstitute.gatk.queue.function.JavaCommandLineFunction {
analysisName = "UnzipSingleBlock"
javaMainClass = "org.broadinstitute.gatk.engine.datasources.reads.utilities.UnzipSingleBlock"

/** block file over which to test unzipping */
@Input(fullName="block_file", shortName="b", doc="block file over which to test unzipping", required=true, exclusiveOf="", validation="")
var block_file: File = _

/**
 * Short name of block_file
 * @return Short name of block_file
 */
def b = this.block_file

/**
 * Short name of block_file
 * @param value Short name of block_file
 */
def b_=(value: File) { this.block_file = value }

/** size of compressed block */
@Input(fullName="compressed_block_size", shortName="cbs", doc="size of compressed block", required=true, exclusiveOf="", validation="")
var compressed_block_size: File = _

/**
 * Short name of compressed_block_size
 * @return Short name of compressed_block_size
 */
def cbs = this.compressed_block_size

/**
 * Short name of compressed_block_size
 * @param value Short name of compressed_block_size
 */
def cbs_=(value: File) { this.compressed_block_size = value }

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

override def commandLine = super.commandLine + required("-b", block_file, spaceSeparated=true, escape=true, format="%s") + required("-cbs", compressed_block_size, spaceSeparated=true, escape=true, format="%s") + optional("-l", logging_level, spaceSeparated=true, escape=true, format="%s") + optional("-log", log_to_file, spaceSeparated=true, escape=true, format="%s") + conditional(help, "-h", escape=true, format="%s") + conditional(version, "-version", escape=true, format="%s")
}
