package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Output

class BAMFileStat extends org.broadinstitute.gatk.queue.function.JavaCommandLineFunction {
analysisName = "BAMFileStat"
javaMainClass = "org.broadinstitute.gatk.engine.datasources.reads.utilities.BAMFileStat"

/** Which operation to run. */
@Argument(fullName="command", shortName="", doc="Which operation to run.", required=true, exclusiveOf="", validation="")
var command: org.broadinstitute.gatk.engine.datasources.reads.utilities.BAMFileStat.CommandType = _

/** The BAM file to inspect. */
@Argument(fullName="bamfilename", shortName="", doc="The BAM file to inspect.", required=true, exclusiveOf="", validation="")
var bamfilename: String = _

/** The range to inspect. */
@Argument(fullName="range", shortName="", doc="The range to inspect.", required=false, exclusiveOf="", validation="")
var range: String = _

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

override def commandLine = super.commandLine + required("--command", command, spaceSeparated=true, escape=true, format="%s") + required("--bamfilename", bamfilename, spaceSeparated=true, escape=true, format="%s") + optional("--range", range, spaceSeparated=true, escape=true, format="%s") + optional("-l", logging_level, spaceSeparated=true, escape=true, format="%s") + optional("-log", log_to_file, spaceSeparated=true, escape=true, format="%s") + conditional(help, "-h", escape=true, format="%s") + conditional(version, "-version", escape=true, format="%s")
}
