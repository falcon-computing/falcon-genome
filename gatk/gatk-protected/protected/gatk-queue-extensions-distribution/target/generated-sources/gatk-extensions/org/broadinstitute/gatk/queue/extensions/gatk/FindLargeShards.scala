package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class FindLargeShards extends org.broadinstitute.gatk.queue.function.JavaCommandLineFunction {
analysisName = "FindLargeShards"
javaMainClass = "org.broadinstitute.gatk.engine.datasources.reads.utilities.FindLargeShards"

/** SAM or BAM file(s) */
@Input(fullName="input_file", shortName="I", doc="SAM or BAM file(s)", required=false, exclusiveOf="", validation="")
var input_file: Seq[File] = Nil

/**
 * Short name of input_file
 * @return Short name of input_file
 */
def I = this.input_file

/**
 * Short name of input_file
 * @param value Short name of input_file
 */
def I_=(value: Seq[File]) { this.input_file = value }

/** Dependencies on any indexes of input_file */
@Input(fullName="input_fileIndexes", shortName="", doc="Dependencies on any indexes of input_file", required=false, exclusiveOf="", validation="")
private var input_fileIndexes: Seq[File] = Nil

/** Reference sequence file */
@Input(fullName="reference_sequence", shortName="R", doc="Reference sequence file", required=false, exclusiveOf="", validation="")
var reference_sequence: File = _

/**
 * Short name of reference_sequence
 * @return Short name of reference_sequence
 */
def R = this.reference_sequence

/**
 * Short name of reference_sequence
 * @param value Short name of reference_sequence
 */
def R_=(value: File) { this.reference_sequence = value }

/** A list of genomic intervals over which to operate. Can be explicitly specified on the command line or in a file. */
@Input(fullName="intervals", shortName="L", doc="A list of genomic intervals over which to operate. Can be explicitly specified on the command line or in a file.", required=false, exclusiveOf="intervalsString", validation="")
var intervals: Seq[File] = Nil

/**
 * Short name of intervals
 * @return Short name of intervals
 */
def L = this.intervals

/**
 * Short name of intervals
 * @param value Short name of intervals
 */
def L_=(value: Seq[File]) { this.intervals = value }

/** A list of genomic intervals over which to operate. Can be explicitly specified on the command line or in a file. */
@Argument(fullName="intervalsString", shortName="L", doc="A list of genomic intervals over which to operate. Can be explicitly specified on the command line or in a file.", required=false, exclusiveOf="intervals", validation="")
var intervalsString: Seq[String] = Nil

/**
 * Short name of intervalsString
 * @return Short name of intervalsString
 */
def LString = this.intervalsString

/**
 * Short name of intervalsString
 * @param value Short name of intervalsString
 */
def LString_=(value: Seq[String]) { this.intervalsString = value }

/** An output file created by the walker.  Will overwrite contents if file exists */
@Output(fullName="out", shortName="o", doc="An output file created by the walker.  Will overwrite contents if file exists", required=false, exclusiveOf="", validation="")
@Gather(classOf[org.broadinstitute.gatk.queue.function.scattergather.SimpleTextGatherFunction])
var out: File = _

/**
 * Short name of out
 * @return Short name of out
 */
def o = this.out

/**
 * Short name of out
 * @param value Short name of out
 */
def o_=(value: File) { this.out = value }

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

override def freezeFieldValues() {
super.freezeFieldValues()
input_fileIndexes ++= input_file.filter(orig => orig != null && orig.getName.endsWith(".bam")).flatMap(orig => Array( new File(orig.getPath + ".bai"), new File(orig.getPath.stripSuffix(".bam") + ".bai") ))
}

override def commandLine = super.commandLine + repeat("-I", input_file, formatPrefix=TaggedFile.formatCommandLineParameter, spaceSeparated=true, escape=true, format="%s") + optional("-R", reference_sequence, spaceSeparated=true, escape=true, format="%s") + repeat("-L", intervals, spaceSeparated=true, escape=true, format="%s") + repeat("-L", intervalsString, spaceSeparated=true, escape=true, format="%s") + optional("-o", out, spaceSeparated=true, escape=true, format="%s") + optional("-l", logging_level, spaceSeparated=true, escape=true, format="%s") + optional("-log", log_to_file, spaceSeparated=true, escape=true, format="%s") + conditional(help, "-h", escape=true, format="%s") + conditional(version, "-version", escape=true, format="%s")
}
