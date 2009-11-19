package org.broadinstitute.sting.utils.genotype.vcf;

import org.broadinstitute.sting.BaseTest;
import org.broadinstitute.sting.gatk.refdata.RodVCF;
import org.junit.Assert;
import org.junit.Test;

import java.io.*;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

/** test the VCFReader class test */
public class VCFReaderTest extends BaseTest {

    private static final File vcfFile = new File("/humgen/gsa-scr1/GATK_Data/Validation_Data/vcfexample.vcf");
    private static final File multiSampleVCF = new File("/humgen/gsa-scr1/GATK_Data/Validation_Data/MultiSample.vcf");
    private static final String VCF_MIXUP_FILE = "/humgen/gsa-scr1/GATK_Data/Validation_Data/mixedup.vcf";

    @Test
    public void testVCFInput() {
        VCFReader reader = new VCFReader(vcfFile);
        int counter = 0;
        while (reader.hasNext()) {
            counter++;
            reader.next();
        }
        Assert.assertEquals(6, counter);
    }

    @Test
    public void testMultiSampleVCFInput() {
        VCFReader reader = new VCFReader(multiSampleVCF);
        int counter = 0;
        while (reader.hasNext()) {
            counter++;
            reader.next();
        }
        Assert.assertEquals(99, counter);
    }

    @Test
    public void testNoCallSites() {
        VCFReader reader = new VCFReader(multiSampleVCF);
        if (!reader.hasNext()) Assert.fail("The reader should have a record");
        VCFRecord rec = reader.next();
        final int numberOfNoCallsTruth = 9;
        int noCalls = 0;
        for (VCFGenotypeRecord record : rec.getVCFGenotypeRecords()) {
            List<VCFGenotypeEncoding> encodings = record.getAlleles();
            if (encodings.get(0).getType() == VCFGenotypeEncoding.TYPE.UNCALLED &&
                    encodings.get(1).getType() == VCFGenotypeEncoding.TYPE.UNCALLED)
                noCalls++;
        }
        Assert.assertEquals(numberOfNoCallsTruth, noCalls);
    }


    @Test
    public void testKnownCallSites() {
        VCFReader reader = new VCFReader(multiSampleVCF);
        if (!reader.hasNext()) Assert.fail("The reader should have a record");
        VCFRecord rec = reader.next();
        boolean seenNA11992 = false;
        boolean seenNA12287 = false;
        for (VCFGenotypeRecord record : rec.getVCFGenotypeRecords()) {
            if (record.getSampleName().equals("NA11992")) {
                List<VCFGenotypeEncoding> encodings = record.getAlleles();
                if (!encodings.get(0).getBases().equals("A") ||
                        !encodings.get(1).getBases().equals("A")) {
                    Assert.fail("Sample NA11992 at site 1:10000005 should be AA");
                }
                seenNA11992 = true;
            }
            if (record.getSampleName().equals("NA12287")) {
                List<VCFGenotypeEncoding> encodings = record.getAlleles();
                if (!encodings.get(0).getBases().equals("A") ||
                        !encodings.get(1).getBases().equals("T")) {
                    Assert.fail("Sample NA11992 at site 1:10000005 should be AA");
                }
                seenNA12287 = true;
            }
        }
        Assert.assertTrue(seenNA11992 && seenNA12287);
    }

    /** test the basic parsing */
    @Test
    public void testBasicParsing() {
        String formatString = "GT:B:C:D";
        String genotypeString = "0|1:2:3:4";
        String altAlleles[] = {"A", "G", "T"};
        char referenceBase = 'C';
        VCFGenotypeRecord rec = VCFReader.getVCFGenotype("test", formatString, genotypeString, altAlleles, referenceBase);
        Assert.assertEquals(VCFGenotypeRecord.PHASE.PHASED, rec.getPhaseType());
        Assert.assertEquals("C", rec.getAlleles().get(0).toString());
        Assert.assertEquals("A", rec.getAlleles().get(1).toString());
        Map<String, String> values = rec.getFields();
        Assert.assertEquals(3, values.size());
        Assert.assertTrue(values.get("B").equals("2"));
        Assert.assertTrue(values.get("C").equals("3"));
        Assert.assertTrue(values.get("D").equals("4"));
    }


    /** test the parsing of a genotype field with missing parameters */
    @Test
    public void testMissingFieldParsing() {
        String formatString = "GT:B:C:D";
        String genotypeString = "0|1:::4";
        String altAlleles[] = {"A", "G", "T"};
        char referenceBase = 'C';
        VCFGenotypeRecord rec = VCFReader.getVCFGenotype("test", formatString, genotypeString, altAlleles, referenceBase);
        Assert.assertEquals(VCFGenotypeRecord.PHASE.PHASED, rec.getPhaseType());
        Assert.assertEquals("C", rec.getAlleles().get(0).toString());
        Assert.assertEquals("A", rec.getAlleles().get(1).toString());
        Map<String, String> values = rec.getFields();
        Assert.assertEquals(3, values.size());
        Assert.assertTrue(values.get("B").equals(""));
        Assert.assertTrue(values.get("C").equals(""));
        Assert.assertTrue(values.get("D").equals("4"));
    }

    /** test the parsing of a genotype field with different missing parameters */
    @Test
    public void testMissingAllFields() {
        String formatString = "GT:B:C:D";
        String genotypeString = "0|1:::";
        String altAlleles[] = {"A", "G", "T"};
        char referenceBase = 'C';
        VCFGenotypeRecord rec = VCFReader.getVCFGenotype("test", formatString, genotypeString, altAlleles, referenceBase);
        Assert.assertEquals(VCFGenotypeRecord.PHASE.PHASED, rec.getPhaseType());
        Assert.assertEquals("C", rec.getAlleles().get(0).toString());
        Assert.assertEquals("A", rec.getAlleles().get(1).toString());
        Map<String, String> values = rec.getFields();
        Assert.assertEquals(3, values.size());
        Assert.assertTrue(values.get("B").equals(""));
        Assert.assertTrue(values.get("C").equals(""));
        Assert.assertTrue(values.get("D").equals(""));
    }

    @Test
    public void testKiransOutOfOrderExample() {
        ArrayList<String> header = new ArrayList<String>();
        ArrayList<String> variant = new ArrayList<String>();

        try {
            FileReader reader = new FileReader(VCF_MIXUP_FILE);
            BufferedReader breader = new BufferedReader(reader);
            String line;
            while ((line = breader.readLine()) != null) {
                String[] pieces = line.split("\\s+");

                if (line.contains("##")) {
                    continue;
                } else if (line.contains("#CHROM")) {
                    for (int i = 0; i < pieces.length; i++) {
                        header.add(i, pieces[i]);
                    }
                } else {
                    for (int i = 0; i < pieces.length; i++) {
                        variant.add(i, pieces[i]);
                    }
                }
            }
        } catch (FileNotFoundException e) {
            Assert.fail("Unable to open the mixed up VCF file.");
        } catch (IOException e) {
            Assert.fail("Unable to read from the mixed up VCF file.");
        }

        // now load up a ROD
        Iterator<RodVCF> rod = RodVCF.createIterator("name",new File(VCF_MIXUP_FILE));
        RodVCF newRod = rod.next();
        List<VCFGenotypeRecord> records = newRod.mCurrentRecord.getVCFGenotypeRecords();
        for (VCFGenotypeRecord record: records) {
            if (!header.contains(record.getSampleName())) {
                Assert.fail("Parsed header doesn't contain field " + record.getSampleName());
            }
            if (!variant.get(header.indexOf(record.getSampleName())).equals(record.toStringEncoding(newRod.mCurrentRecord.getAlternateAlleles()))) {
                Assert.fail("Parsed value for " + record.getSampleName() + " doesn't contain the same value ( " + record.toStringEncoding(newRod.mCurrentRecord.getAlternateAlleles())
                    + " != " + variant.get(header.indexOf(record.getSampleName())));
            }
        }
    }
}
