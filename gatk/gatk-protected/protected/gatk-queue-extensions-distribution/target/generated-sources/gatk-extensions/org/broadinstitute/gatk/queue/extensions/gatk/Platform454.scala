package org.broadinstitute.gatk.queue.extensions.gatk

trait Platform454 extends org.broadinstitute.gatk.queue.function.CommandLineFunction {

abstract override def commandLine = super.commandLine + required("--read_filter", "Platform454")
}
