package org.broadinstitute.gatk.queue.extensions.gatk

trait BadCigar extends org.broadinstitute.gatk.queue.function.CommandLineFunction {

abstract override def commandLine = super.commandLine + required("--read_filter", "BadCigar")
}
