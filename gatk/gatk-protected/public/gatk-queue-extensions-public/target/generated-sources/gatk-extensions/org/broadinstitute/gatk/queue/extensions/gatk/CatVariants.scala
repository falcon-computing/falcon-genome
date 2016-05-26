package org.broadinstitute.gatk.queue.extensions.gatk

import java.io.File
import org.broadinstitute.gatk.utils.commandline.Argument
import org.broadinstitute.gatk.utils.commandline.Gather
import org.broadinstitute.gatk.utils.commandline.Input
import org.broadinstitute.gatk.utils.commandline.Output

class CatVariants extends org.broadinstitute.gatk.queue.function.JavaCommandLineFunction {
analysisName = "CatVariants"
javaMainClass = "org.broadinstitute.gatk.tools.CatVariants"

/** genome reference file <name>.fasta */
@Input(fullName="reference", shortName="R", doc="genome reference file <name>.fasta", required=true, exclusiveOf="", validation="")
var reference: File = _

/**
 * Short name of reference
 * @return Short name of reference
 */
def R = this.reference

/**
 * Short name of reference
 * @param value Short name of reference
 */
def R_=(value: File) { this.reference = value }

/** Input VCF file/s */
@Input(fullName="variant", shortName="V", doc="Input VCF file/s", required=true, exclusiveOf="", validation="")
var variant: Seq[File] = Nil

/**
 * Short name of variant
 * @return Short name of variant
 */
def V = this.variant

/**
 * Short name of variant
 * @param value Short name of variant
 */
def V_=(value: Seq[File]) { this.variant = value }

/** output file */
@Output(fullName="outputFile", shortName="out", doc="output file", required=true, exclusiveOf="", validation="")
@Gather(classOf[org.broadinstitute.gatk.queue.function.scattergather.SimpleTextGatherFunction])
var outputFile: File = _

/**
 * Short name of outputFile
 * @return Short name of outputFile
 */
def out = this.outputFile

/**
 * Short name of outputFile
 * @param value Short name of outputFile
 */
def out_=(value: File) { this.outputFile = value }

/** assumeSorted should be true if the input files are already sorted (based on the position of the variants) */
@Argument(fullName="assumeSorted", shortName="assumeSorted", doc="assumeSorted should be true if the input files are already sorted (based on the position of the variants)", required=false, exclusiveOf="", validation="")
var assumeSorted: Boolean = _

/** which type of IndexCreator to use for VCF/BCF indices */
@Argument(fullName="variant_index_type", shortName="", doc="which type of IndexCreator to use for VCF/BCF indices", required=false, exclusiveOf="", validation="")
var variant_index_type: org.broadinstitute.gatk.utils.variant.GATKVCFIndexType = _

/** the parameter (bin width or features per bin) to pass to the VCF/BCF IndexCreator */
@Argument(fullName="variant_index_parameter", shortName="", doc="the parameter (bin width or features per bin) to pass to the VCF/BCF IndexCreator", required=false, exclusiveOf="", validation="")
var variant_index_parameter: Option[Int] = None

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

override def commandLine = super.commandLine + required("-R", reference, spaceSeparated=true, escape=true, format="%s") + repeat("-V", variant, spaceSeparated=true, escape=true, format="%s") + required("-out", outputFile, spaceSeparated=true, escape=true, format="%s") + conditional(assumeSorted, "-assumeSorted", escape=true, format="%s") + optional("--variant_index_type", variant_index_type, spaceSeparated=true, escape=true, format="%s") + optional("--variant_index_parameter", variant_index_parameter, spaceSeparated=true, escape=true, format="%s") + optional("-l", logging_level, spaceSeparated=true, escape=true, format="%s") + optional("-log", log_to_file, spaceSeparated=true, escape=true, format="%s") + conditional(help, "-h", escape=true, format="%s") + conditional(version, "-version", escape=true, format="%s")
}
