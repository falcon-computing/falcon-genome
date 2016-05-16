import org.broadinstitute.gatk.queue.QScript
import org.broadinstitute.gatk.queue.extensions.gatk._

class BaseRecalQueue extends QScript {
  @Input(shortName="R")
  var referenceFile: File = _

  @Input(shortName="I")
  var bamFile: List[File] = Nil

  @Argument(shortName="interval_padding", required=false)
  var intervalPadding: Int = _

  @Input(shortName="knownSites")
  var knownSites: List[File] = Nil

  @Input(shortName="BQSR", required=false)
  var bqsrFile: File = _

  @Output(shortName="o")
  var outFile: File = _

  @Argument(shortName="scatterCount")
  var scatterCount: Int = _

  def script() {
    val br = new BaseRecalibrator

    br.R = referenceFile
    br.I = bamFile
    br.interval_padding = intervalPadding
    br.BQSR = bqsrFile
    br.knownSites = knownSites
    br.o = outFile

    br.nct = 4
    br.scatterCount = scatterCount
    br.memoryLimit = 4

    add(br)
  }
}
