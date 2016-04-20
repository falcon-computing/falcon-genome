/*
 * The MIT License
 *
 * Copyright (c) 2009 The Broad Institute
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
package htsjdk.samtools.util;

import htsjdk.samtools.util.zip.DeflaterFactory;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.zip.CRC32;
import java.util.zip.Deflater;


/**
 * Writer for a file that is a series of gzip blocks (BGZF format).  The caller just treats it as an
 * OutputStream, and under the covers a gzip block is written when the amount of uncompressed as-yet-unwritten
 * bytes reaches a threshold.
 *
 * The advantage of BGZF over conventional gzip is that BGZF allows for seeking without having to scan through
 * the entire file up to the position being sought.
 *
 * Note that the flush() method should not be called by client
 * unless you know what you're doing, because it forces a gzip block to be written even if the
 * number of buffered bytes has not reached threshold.  close(), on the other hand, must be called
 * when done writing in order to force the last gzip block to be written.
 *
 * c.f. http://samtools.sourceforge.net/SAM1.pdf for details of BGZF file format.
 */
public class BlockCompressedOutputStream
        extends OutputStream
        implements LocationAware
{
    private static int defaultCompressionLevel = BlockCompressedStreamConstants.DEFAULT_COMPRESSION_LEVEL;

    /**
     * Sets the GZip compression level for subsequent BlockCompressedOutputStream object creation
     * that do not specify the compression level.
     * @param compressionLevel 1 <= compressionLevel <= 9
     */
    public static void setDefaultCompressionLevel(final int compressionLevel) {
        if (compressionLevel < Deflater.NO_COMPRESSION || compressionLevel > Deflater.BEST_COMPRESSION) {
            throw new IllegalArgumentException("Invalid compression level: " + compressionLevel);
        }
        defaultCompressionLevel = compressionLevel;
    }

    public static int getDefaultCompressionLevel() {
        return defaultCompressionLevel;
    }

    private final BinaryCodec codec;
    private static final int NOF_BLOCK = 16;
    private static final int UNCOMPRESSED_BLOCK_SIZE =
        BlockCompressedStreamConstants.DEFAULT_UNCOMPRESSED_BLOCK_SIZE;
    private static final int COMPRESSED_BLOCK_SIZE =
        BlockCompressedStreamConstants.MAX_COMPRESSED_BLOCK_SIZE -
        BlockCompressedStreamConstants.BLOCK_HEADER_LENGTH;
    private final byte[] uncompressedBuffer = new byte[UNCOMPRESSED_BLOCK_SIZE * NOF_BLOCK];
    private int numUncompressedBytes = 0;
    private final byte[] compressedBuffer = new byte[COMPRESSED_BLOCK_SIZE * NOF_BLOCK];
    private final Deflater[] deflaters = new Deflater[NOF_BLOCK];
    private final CRC32[] crc32s = new CRC32[NOF_BLOCK];
    ExecutorService service = Executors.newFixedThreadPool(8);
    private ArrayList<Future<Integer> > deflateFutures =
        new ArrayList<Future<Integer> >();

    // A second deflater is created for the very unlikely case where the regular deflation actually makes
    // things bigger, and the compressed block is too big.  It should be possible to downshift the
    // primary deflater to NO_COMPRESSION level, recompress, and then restore it to its original setting,
    // but in practice that doesn't work.
    // The motivation for deflating at NO_COMPRESSION level is that it will predictably produce compressed
    // output that is 10 bytes larger than the input, and the threshold at which a block is generated is such that
    // the size of tbe final gzip block will always be <= 64K.  This is preferred over the previous method,
    // which would attempt to compress up to 64K bytes, and if the resulting compressed block was too large,
    // try compressing fewer input bytes (aka "downshifting').  The problem with downshifting is that
    // getFilePointer might return an inaccurate value.
    // I assume (AW 29-Oct-2013) that there is no value in using hardware-assisted deflater for no-compression mode,
    // so just use JDK standard.
    private final Deflater noCompressionDeflater = new Deflater(Deflater.NO_COMPRESSION, true);
    //private final CRC32 crc32 = new CRC32();
    private File file = null;
    private long mBlockAddress = 0;


    // Really a local variable, but allocate once to reduce GC burden.
    private final byte[] singleByteArray = new byte[1];

    /**
     * Uses default compression level, which is 5 unless changed by setCompressionLevel
     */
    public BlockCompressedOutputStream(final String filename) {
        this(filename, defaultCompressionLevel);
    }

    /**
     * Uses default compression level, which is 5 unless changed by setCompressionLevel
     */
    public BlockCompressedOutputStream(final File file) {
        this(file, defaultCompressionLevel);
    }

    /**
     * Prepare to compress at the given compression level
     * @param compressionLevel 1 <= compressionLevel <= 9
     */
    public BlockCompressedOutputStream(final String filename, final int compressionLevel) {
        this(new File(filename), compressionLevel);
    }

    /**
     * Prepare to compress at the given compression level
     * @param compressionLevel 1 <= compressionLevel <= 9
     */
    public BlockCompressedOutputStream(final File file, final int compressionLevel) {
        this.file = file;
        codec = new BinaryCodec(file, true);
        for (int i = 0; i < NOF_BLOCK; ++i) {
          crc32s[i] = new CRC32();
          deflaters[i] = DeflaterFactory.makeDeflater(compressionLevel, true);
        }
    }

    /**
     * Constructors that take output streams
     * file may be null
     */
    public BlockCompressedOutputStream(final OutputStream os, final File file) {
        this(os, file, defaultCompressionLevel);
    }

    public BlockCompressedOutputStream(final OutputStream os, final File file, final int compressionLevel) {
        this.file = file;
        codec = new BinaryCodec(os);
        if (file != null) {
            codec.setOutputFileName(file.getAbsolutePath());
        }
        for (int i = 0; i < NOF_BLOCK; ++i) {
          crc32s[i] = new CRC32();
          deflaters[i] = DeflaterFactory.makeDeflater(compressionLevel, true);
        }
    }

    /**
     *
     * @param location May be null.  Used for error messages, and for checking file termination.
     * @param output May or not already be a BlockCompressedOutputStream.
     * @return A BlockCompressedOutputStream, either by wrapping the given OutputStream, or by casting if it already
     *         is a BCOS.
     */
    public static BlockCompressedOutputStream maybeBgzfWrapOutputStream(final File location, OutputStream output) {
      if (!(output instanceof BlockCompressedOutputStream)) {
        return new BlockCompressedOutputStream(output, location);
      } else {
        return (BlockCompressedOutputStream)output;
        }
    }

    /**
     * Writes b.length bytes from the specified byte array to this output stream. The general contract for write(b)
     * is that it should have exactly the same effect as the call write(b, 0, b.length).
     * @param bytes the data
     */
    @Override
    public void write(final byte[] bytes) throws IOException {
        write(bytes, 0, bytes.length);
    }

    /**
     * Writes len bytes from the specified byte array starting at offset off to this output stream. The general
     * contract for write(b, off, len) is that some of the bytes in the array b are written to the output stream in order;
     * element b[off] is the first byte written and b[off+len-1] is the last byte written by this operation.
     *
     * @param bytes the data
     * @param startIndex the start offset in the data
     * @param numBytes the number of bytes to write
     */
    @Override
    public void write(final byte[] bytes, int startIndex, int numBytes) throws IOException {
        assert(numUncompressedBytes < uncompressedBuffer.length);
        while (numBytes > 0) {
            final int bytesToWrite = Math.min(uncompressedBuffer.length - numUncompressedBytes, numBytes);
            System.arraycopy(bytes, startIndex, uncompressedBuffer, numUncompressedBytes, bytesToWrite);
            numUncompressedBytes += bytesToWrite;
            startIndex += bytesToWrite;
            numBytes -= bytesToWrite;
            assert(numBytes >= 0);
            if (numUncompressedBytes == uncompressedBuffer.length) {
                deflateBlock();
            }
        }
    }

    /**
     * WARNING: flush() affects the output format, because it causes the current contents of uncompressedBuffer
     * to be compressed and written, even if it isn't full.  Unless you know what you're doing, don't call flush().
     * Instead, call close(), which will flush any unwritten data before closing the underlying stream.
     *
     */
    @Override
    public void flush() throws IOException {
        while (numUncompressedBytes > 0) {
            deflateBlock();
        }
        codec.getOutputStream().flush();
    }

    /**
     * close() must be called in order to flush any remaining buffered bytes.  An unclosed file will likely be
     * defective.
     *
     */
    @Override
    public void close() throws IOException {
        flush();
        // For debugging...
        // if (numberOfThrottleBacks > 0) {
        //     System.err.println("In BlockCompressedOutputStream, had to throttle back " + numberOfThrottleBacks +
        //                        " times for file " + codec.getOutputFileName());
        // }
        codec.writeBytes(BlockCompressedStreamConstants.EMPTY_GZIP_BLOCK);
        codec.close();
        // Can't re-open something that is not a regular file, e.g. a named pipe or an output stream
        if (this.file == null || !this.file.isFile()) return;
        if (BlockCompressedInputStream.checkTermination(this.file) !=
                BlockCompressedInputStream.FileTermination.HAS_TERMINATOR_BLOCK) {
            throw new IOException("Terminator block not found after closing BGZF file " + this.file);
        }
        service.shutdown();
    }

    /**
     * Writes the specified byte to this output stream. The general contract for write is that one byte is written
     * to the output stream. The byte to be written is the eight low-order bits of the argument b.
     * The 24 high-order bits of b are ignored.
     * @param bite
     * @throws IOException
     */
    public void write(final int bite) throws IOException {
        singleByteArray[0] = (byte)bite;
        write(singleByteArray);
    }

    /** Encode virtual file pointer
     * Upper 48 bits is the byte offset into the compressed stream of a block.
     * Lower 16 bits is the byte offset into the uncompressed stream inside the block.
     */
    public long getFilePointer(){
      // TODO(mhhuang): this function no longer works in multi-threaded implementation.
      assert(false);
      return 0;
      //return BlockCompressedFilePointerUtil.makeFilePointer(mBlockAddress, numUncompressedBytes);
    }

    @Override
    public long getPosition() {
        return getFilePointer();
    }

    /**
     * Attempt to write the data in uncompressedBuffer to the underlying file in a gzip block.
     * If the entire uncompressedBuffer does not fit in the maximum allowed size, reduce the amount
     * of data to be compressed, and slide the excess down in uncompressedBuffer so it can be picked
     * up in the next deflate event.
     * @return size of gzip block that was written.
     */

    class deflateThread implements Callable<Integer> {
      private final int ucBufferOffset;
      private final int bytesToCompress;
      private final int cBufferOffset;
      private CRC32 crc32;
      private final Deflater deflater;

      deflateThread(final int ucBufferOffset, final int bytesToCompress,
          final int cBufferOffset, final Deflater deflater, final CRC32 crc32) {
        this.ucBufferOffset = ucBufferOffset;
        this.bytesToCompress = bytesToCompress;
        this.cBufferOffset = cBufferOffset;
        this.deflater = deflater;
        this.crc32 = crc32;
      }

      @Override
      public Integer call() {
        // Compress the input
        deflater.reset();
        deflater.setInput(uncompressedBuffer, ucBufferOffset, bytesToCompress);
        deflater.finish();
        int compressedSize = deflater.deflate(compressedBuffer, cBufferOffset, COMPRESSED_BLOCK_SIZE);

        // This shouldn't be invoked if we are using compression level 0 at the first hand
        // If it didn't all fit in compressedBuffer.length, set compression level to NO_COMPRESSION
        // and try again.  This should always fit.
        if (!deflater.finished()) {
            noCompressionDeflater.reset();
            noCompressionDeflater.setInput(uncompressedBuffer, 0, bytesToCompress);
            noCompressionDeflater.finish();
            compressedSize = noCompressionDeflater.deflate(compressedBuffer, cBufferOffset, COMPRESSED_BLOCK_SIZE);
            if (!noCompressionDeflater.finished()) {
                throw new IllegalStateException("unpossible");
            }
        }
        // Data compressed small enough, so write it out.
        crc32.reset();
        crc32.update(uncompressedBuffer, ucBufferOffset, bytesToCompress);

        return Integer.valueOf(compressedSize);
      }
    }


    /* Multi-threaded implementation */
    private int deflateBlock() {
        if (numUncompressedBytes == 0) {
            return 0;
        }

        // Set number of threads
        deflateFutures.clear();
        
        int remainingBytes = numUncompressedBytes;
        int ucBufferOffset = 0;
        int cBufferOffset = 0;
        int counter = 0;
        while (remainingBytes > 0) {
          final int bytesToCompress = Math.min(remainingBytes, UNCOMPRESSED_BLOCK_SIZE);
          deflateFutures.add(service.submit(
              new deflateThread(
                  ucBufferOffset, bytesToCompress,
                  cBufferOffset, deflaters[counter], crc32s[counter])));

          ucBufferOffset += bytesToCompress;
          cBufferOffset += COMPRESSED_BLOCK_SIZE;
          counter++;

          remainingBytes -= bytesToCompress;
        }

        int totalBlockSize = 0;
        
        // sequential part
        remainingBytes = numUncompressedBytes;
        counter = 0;
        cBufferOffset = 0;
        while (remainingBytes > 0) {
          final int bytesToCompress = Math.min(remainingBytes, UNCOMPRESSED_BLOCK_SIZE);
          int compressedSize = 0;
          try {
            compressedSize = deflateFutures.get(counter).get();
          } catch (InterruptedException ie) {
            System.err.println("InterruptedException in deflating blocks");
            assert(false);
          } catch (ExecutionException ee) {
            System.err.println("ExecutionException in deflating blocks");
            assert(false);
          }

          totalBlockSize += writeGzipBlock(
              cBufferOffset, compressedSize, bytesToCompress, crc32s[counter].getValue());

          cBufferOffset += COMPRESSED_BLOCK_SIZE;
          remainingBytes -= bytesToCompress;
          counter ++;
        }
        
        numUncompressedBytes = 0;
        mBlockAddress += totalBlockSize;
        return totalBlockSize;
    }

    /**
     * Writes the entire gzip block, assuming the compressed data is stored in compressedBuffer
     * @return  size of gzip block that was written.
     */
    private int writeGzipBlock(final int compressedBufferOffset, final int compressedSize,
        final int uncompressedSize, final long crc) {
        // Init gzip header
        codec.writeByte(BlockCompressedStreamConstants.GZIP_ID1);
        codec.writeByte(BlockCompressedStreamConstants.GZIP_ID2);
        codec.writeByte(BlockCompressedStreamConstants.GZIP_CM_DEFLATE);
        codec.writeByte(BlockCompressedStreamConstants.GZIP_FLG);
        codec.writeInt(0); // Modification time
        codec.writeByte(BlockCompressedStreamConstants.GZIP_XFL);
        codec.writeByte(BlockCompressedStreamConstants.GZIP_OS_UNKNOWN);
        codec.writeShort(BlockCompressedStreamConstants.GZIP_XLEN);
        codec.writeByte(BlockCompressedStreamConstants.BGZF_ID1);
        codec.writeByte(BlockCompressedStreamConstants.BGZF_ID2);
        codec.writeShort(BlockCompressedStreamConstants.BGZF_LEN);
        final int totalBlockSize = compressedSize + BlockCompressedStreamConstants.BLOCK_HEADER_LENGTH +
                BlockCompressedStreamConstants.BLOCK_FOOTER_LENGTH;

        // I don't know why we store block size - 1, but that is what the spec says
        codec.writeShort((short)(totalBlockSize - 1));
        codec.writeBytes(compressedBuffer, compressedBufferOffset, compressedSize);
        codec.writeInt((int)crc);
        codec.writeInt(uncompressedSize);
        return totalBlockSize;
    }
}
