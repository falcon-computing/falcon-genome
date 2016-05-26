package org.broadinstitute.gatk.queue.extensions.gatk

trait CountingRead extends org.broadinstitute.gatk.queue.function.CommandLineFunction {

abstract override def commandLine = super.commandLine + required("--read_filter", "CountingRead")
}
