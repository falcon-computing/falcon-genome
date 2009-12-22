/*
 * Copyright (c) 2009 The Broad Institute
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

package org.broadinstitute.sting.gatk.io.storage;

import java.io.*;
import java.util.List;
import java.util.Set;
import java.util.HashSet;

import org.broadinstitute.sting.gatk.io.stubs.GenotypeWriterStub;
import org.broadinstitute.sting.utils.genotype.*;
import org.broadinstitute.sting.utils.genotype.vcf.*;
import org.broadinstitute.sting.utils.SampleUtils;

/**
 * Provides temporary storage for GenotypeWriters.
 *
 * @author ebanks
 * @version 0.1
 */
public abstract class GenotypeWriterStorage<T extends GenotypeWriter> implements GenotypeWriter, Storage<T> {
    protected final File file;
    protected final GenotypeWriter writer;

    /**
     * Constructs an object which will write directly into the output file provided by the stub.
     * Intentionally delaying the writing of the header -- this should be filled in by the walker.
     * @param stub Stub to use when constructing the output file.
     */
    public GenotypeWriterStorage( GenotypeWriterStub stub ) {
        this.file = stub.getFile();
        writer = GenotypeWriterFactory.create(stub.getFormat(), file);
    }

    /**
     * Constructs an object which will redirect into a different file.
     * @param stub Stub to use when synthesizing file / header info.
     * @param file File into which to direct the output data.
     */
    public GenotypeWriterStorage( GenotypeWriterStub stub, File file ) {
        this.file = file;
        writer = GenotypeWriterFactory.create(stub.getFormat(), file);
        Set<String> samples = SampleUtils.getSAMFileSamples(stub.getSAMFileHeader());
        GenotypeWriterFactory.writeHeader(writer, stub.getSAMFileHeader(), samples, new HashSet<VCFHeaderLine>());
    }

    public void addGenotypeCall(Genotype call) {
        writer.addGenotypeCall(call);
    }

    public void addNoCall(int position) {
        writer.addNoCall(position);
    }

    public void addMultiSampleCall(List<Genotype> genotypes, VariationCall variation) {
        writer.addMultiSampleCall(genotypes, variation);
    }

    public boolean supportsMultiSample() {
        return writer.supportsMultiSample();
    }

    public void close() {
        writer.close();
    }

}