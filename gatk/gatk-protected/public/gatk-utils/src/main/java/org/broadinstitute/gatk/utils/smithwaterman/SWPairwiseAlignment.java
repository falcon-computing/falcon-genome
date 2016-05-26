/*
* Copyright 2012-2015 Broad Institute, Inc.
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
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
* THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package org.broadinstitute.gatk.utils.smithwaterman;

import com.google.common.io.FileBackedOutputStream;
import htsjdk.samtools.Cigar;
import htsjdk.samtools.CigarElement;
import htsjdk.samtools.CigarOperator;
import org.apache.commons.io.IOExceptionWithCause;
import org.broadinstitute.gatk.utils.exceptions.GATKException;
import org.broadinstitute.gatk.utils.sam.AlignmentUtils;
import org.apache.log4j.Logger;

import java.io.*;
import java.io.FileOutputStream;
//import java.io.File;
//import java.io.FileWriter;
import java.io.IOException;
//import java.io.FileNotFoundException;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

/**
 * Pairwise discrete smith-waterman alignment
 *
 * ************************************************************************
 * ****                    IMPORTANT NOTE:                             ****
 * ****  This class assumes that all bytes come from UPPERCASED chars! ****
 * ************************************************************************
 *
 * User: asivache
 * Date: Mar 23, 2009
 * Time: 1:54:54 PM
 */
public class SWPairwiseAlignment implements SmithWaterman{
    private final static Logger logger = Logger.getLogger(SWPairwiseAlignment.class);

    protected static long total_time = 0;
    protected static long length_ref_max = 0;
    protected static long length_alt_max = 0;
    protected static long count_sw_max = 1000000;
    protected static long count_sw = 0;
    protected static long count_debug = 39797;
    protected SWPairwiseAlignmentResult alignmentResult;

    protected final Parameters parameters;

    /**
     * The state of a trace step through the matrix, 0,1,2,3
     */
    protected enum State {
        MATCH,
        INSERTION,
        DELETION,
        CLIP
    }

    /**
     * What strategy should we use when the best path does not start/end at the corners of the matrix?
     */
    public enum OVERHANG_STRATEGY {
        /*
         * Add softclips for the overhangs
         */
        SOFTCLIP,

        /*
         * Treat the overhangs as proper insertions/deletions
         */
        INDEL,

        /*
         * Treat the overhangs as proper insertions/deletions for leading (but not trailing) overhangs.
         * This is useful e.g. when we want to merge dangling tails in an assembly graph: because we don't
         * expect the dangling tail to reach the end of the reference path we are okay ignoring trailing
         * deletions - but leading indels are still very much relevant.
         */
        LEADING_INDEL,

        /*
         * Just ignore the overhangs
         */
        IGNORE
    }

    protected static boolean cutoff = false;

    protected OVERHANG_STRATEGY overhang_strategy = OVERHANG_STRATEGY.SOFTCLIP;

    /**
     * The SW scoring matrix, stored for debugging purposes if keepScoringMatrix is true
     */
    protected int[][] SW = null;
    private static int max_length_ref = 0;
    private static int max_length_alt = 0;

    /**
     * Only for testing purposes in the SWPairwiseAlignmentMain function
     * set to true to keep SW scoring matrix after align call
     */
    protected static boolean keepScoringMatrix = false;

    /**
     * Create a new SW pairwise aligner.
     *
     * @deprecated in favor of constructors using the Parameter or ParameterSet class
     */
    @Deprecated
    public SWPairwiseAlignment(byte[] seq1, byte[] seq2, int match, int mismatch, int open, int extend ) {
        this(seq1, seq2, new Parameters(match, mismatch, open, extend));
    }

    /**
     * Create a new SW pairwise aligner
     *
     * After creating the object the two sequences are aligned with an internal call to align(seq1, seq2)
     *
     * @param seq1 the first sequence we want to align
     * @param seq2 the second sequence we want to align
     * @param parameters the SW parameters to use
     */
    public SWPairwiseAlignment(byte[] seq1, byte[] seq2, Parameters parameters) {
        this(parameters);
        align(seq1,seq2);
    }

    /**
     * Create a new SW pairwise aligner
     *
     * After creating the object the two sequences are aligned with an internal call to align(seq1, seq2)
     *
     * @param seq1 the first sequence we want to align
     * @param seq2 the second sequence we want to align
     * @param parameters the SW parameters to use
     * @param strategy   the overhang strategy to use
     */
    public SWPairwiseAlignment(final byte[] seq1, final byte[] seq2, final SWParameterSet parameters, final OVERHANG_STRATEGY strategy) {
        this(parameters.parameters);
        overhang_strategy = strategy;
        align(seq1, seq2);
    }

    /**
     * Create a new SW pairwise aligner, without actually doing any alignment yet
     *
     * @param parameters the SW parameters to use
     */
    protected SWPairwiseAlignment(final Parameters parameters) {
        this.parameters = parameters;
    }

    /**
     * Create a new SW pairwise aligner
     *
     * After creating the object the two sequences are aligned with an internal call to align(seq1, seq2)
     *
     * @param seq1 the first sequence we want to align
     * @param seq2 the second sequence we want to align
     * @param namedParameters the named parameter set to get our parameters from
     */
    public SWPairwiseAlignment(byte[] seq1, byte[] seq2, SWParameterSet namedParameters) {
        this(seq1, seq2, namedParameters.parameters);
    }

    public SWPairwiseAlignment(byte[] seq1, byte[] seq2) {
        this(seq1,seq2,SWParameterSet.ORIGINAL_DEFAULT);
    }

    @Override
    public Cigar getCigar() { return alignmentResult.cigar ; }

    @Override
    public int getAlignmentStart2wrt1() { return alignmentResult.alignment_offset; }

//    public class SW_JNI {
//        public native int smithwaterman(int p_w_match, int p_w_mismatch, int p_w_open, int p_w_extend,
//                                        byte[] reference, byte[] alternate, int ref_length, int alt_length, int strategy,
//                                        byte[] state_array, byte[] length_array, int output_alighment_offset, int output_num );
//        private int SW_JNI() {
//            return 1;
//        }
//    }
    /**
     * Aligns the alternate sequence to the reference sequence
     *
     * @param reference  ref sequence
     * @param alternate  alt sequence
     */
    protected void align(final byte[] reference, final byte[] alternate) {
        if ( reference == null || reference.length == 0 || alternate == null || alternate.length == 0 )
            throw new IllegalArgumentException("Non-null, non-empty sequences are required for the Smith-Waterman calculation");

        //---------------------------------------------------------------------------------------------JNI
        byte[] state_array = new byte[1024];
        int[] length_array = new int[1024];
        int[] output_alignment_offset = new int[1];
        int[] output_num = new int[1];
        int strategy = 0;
        switch( overhang_strategy ) {
            case SOFTCLIP:      strategy = 0; break;
            case INDEL:         strategy = 1; break;
            case LEADING_INDEL: strategy = 2; break;
            case IGNORE:        strategy = 3; break;
        }

      //System.load("/curr/hanhu/GENO/sw_JNI/libSWJNI.so");
        System.loadLibrary("SWJNI");
        SWJNI new_jni = new SWJNI();
        new_jni.smithwaterman(parameters.w_match, parameters.w_mismatch, parameters.w_open, parameters.w_extend,
                reference, alternate, reference.length, alternate.length, strategy,
                state_array, length_array, output_alignment_offset, output_num);
        //System.out.println("SmithWaterman output_alignment_offset = " + output_alignment_offset[0] + " ,output_num =" + output_num[0]);
        //for(int i=0; i<output_num[0]; i++) {
        //    System.out.println("SmithWaterman state = " + state_array[i] + ", length =" + length_array[i]);
        //}

        alignmentResult = calculateCigar_new(state_array, length_array, output_alignment_offset[0], output_num[0]);
        //---------------------------------------------------------------------------------------------JNI

        final int n = reference.length+1;
        final int m = alternate.length+1;
        length_ref_max += n;
        length_alt_max += m;
        //logger.info("SmithWaterman reference.length.max = " + length_ref_max + ", alternate.length.max =" + length_alt_max + "\n");

        if(count_sw<count_sw_max) {
            try {
                Writer writer_ref = new FileWriter("ref.dat", true);
                Writer writer_alt = new FileWriter("alt.dat", true);
                Writer writer_ref_len = new FileWriter("ref_length.dat", true);
                Writer writer_alt_len = new FileWriter("alt_length.dat", true);
                Writer writer_strategy = new FileWriter("strategy.dat", true);
                Writer writer_strategy_c = new FileWriter("strategy_c.dat", true);
                int strategy_c=0;

                writer_ref_len.write(String.valueOf(n-1) + "\n");
                writer_alt_len.write(String.valueOf(m-1) + "\n");
                for (int i = 0; i < reference.length; i++) {
                    writer_ref.write(String.valueOf(reference[i]) + "\n");
                }
                for (int i = 0; i < alternate.length; i++) {
                    writer_alt.write(String.valueOf(alternate[i]) + "\n");
                }
                if(overhang_strategy == OVERHANG_STRATEGY.SOFTCLIP) {
                    strategy_c=0;
                } else if(overhang_strategy == OVERHANG_STRATEGY.INDEL) {
                    strategy_c=1;
                } else if(overhang_strategy == OVERHANG_STRATEGY.LEADING_INDEL) {
                    strategy_c=2;
                } else {
                    strategy_c=3;
                }
                writer_strategy.write(String.valueOf(overhang_strategy) + "\n");
                writer_strategy_c.write(String.valueOf(strategy_c) + "\n");

                writer_ref.close();
                writer_alt.close();
                writer_ref_len.close();
                writer_alt_len.close();
                writer_strategy.close();
                writer_strategy_c.close();


                Writer writer_w_open = new FileWriter("w_open.dat", true);
                writer_w_open.write(String.valueOf(parameters.w_open) + "\n");
                writer_w_open.close();
                Writer writer_w_extend = new FileWriter("w_extend.dat", true);
                writer_w_extend.write(String.valueOf(parameters.w_extend) + "\n");
                writer_w_extend.close();
                Writer writer_w_match = new FileWriter("w_match.dat", true);
                writer_w_match.write(String.valueOf(parameters.w_match) + "\n");
                writer_w_match.close();
                Writer writer_w_mismatch = new FileWriter("w_mismatch.dat", true);
                writer_w_mismatch.write(String.valueOf(parameters.w_mismatch) + "\n");
                writer_w_mismatch.close();
                //System.out.println("SmithWaterman parameter.open = " + parameters.w_open + " ,parameter.extend =" + parameters.w_extend + " ,parameter.match =" + parameters.w_match + " ,parameter.mismatch =" + parameters.w_mismatch);
            } catch (IOException e) {
                e.printStackTrace();
            }
            ;
            //long start_time = System.nanoTime();
            //int length1;
            //if(reference.length%2 == 1) {
            //    length1 = reference.length+1;
            //} else {
            //    length1 = reference.length;
            //}
            //int length2;
            //if(alternate.length%2 == 1) {
            //    length2 = alternate.length+1;
            //} else {
            //    length2 = alternate.length;
            //}
            //byte data_ref[] = new byte[length1/2+length2/2];
            //byte ref_new[] = new byte[reference.length];
            //byte alt_new[] = new byte[alternate.length];
            //for(int i=0; i<reference.length; i++) {
            //    if(reference[i] == 65) {
            //        ref_new[i] = 1;
            //    } else if(reference[i] == 67) {
            //        ref_new[i] = 2;
            //    } else if(reference[i] == 71) {
            //        ref_new[i] = 3;
            //    } else if(reference[i] == 78) {
            //        ref_new[i] = 4;
            //    } else if(reference[i] == 84) {
            //        ref_new[i] = 5;
            //    }
            //}
            //for(int i=0; i<alternate.length; i++) {
            //    if(alternate[i] == 65) {
            //        alt_new[i] = 1;
            //    } else if(alternate[i] == 67) {
            //        alt_new[i] = 2;
            //    } else if(alternate[i] == 71) {
            //        alt_new[i] = 3;
            //    } else if(alternate[i] == 78) {
            //        alt_new[i] = 4;
            //    } else if(alternate[i] == 84) {
            //        alt_new[i] = 5;
            //    }
            //}
            //for(int i=0; i<length1/2; i++) {
            //    byte data0 = ref_new[2*i];
            //    byte data1 = ref_new[2*i+1];
            //    if(length1 > reference.length && (2*i+1) == reference.length) {
            //        data1 = 0;
            //    }
            //    data_ref[i] = data0 + data1<<4;
            //}
            //for(int i=0; i<length2/2; i++) {
            //    byte data0 = alt_new[2*i];
            //    byte data1 = alt_new[2*i+1];
            //    if(length2 > alternate.length && (2*i+1) == alternate.length) {
            //        data1 = 0;
            //    }
            //    data_ref[length1/2+i] = data0 + data1<<4;
            //}
            //long end_time = System.nanoTime();
            //total_time += end_time - start_time;
            //System.out.println("total time = " + total_time);
        }
        if(count_sw==count_debug) {
            try {
                Writer writer_ref_debug = new FileWriter("ref_debug.dat", true);
                Writer writer_alt_debug = new FileWriter("alt_debug.dat", true);
                for (int i = 0; i < reference.length; i++) {
                    writer_ref_debug.write(String.valueOf(reference[i]) + "\n");
                }
                for (int i = 0; i < alternate.length; i++) {
                    writer_alt_debug.write(String.valueOf(alternate[i]) + "\n");
                }

                writer_ref_debug.close();
                writer_alt_debug.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            ;
        }
//        System.out.println("reference = " + Arrays.toString(reference));
//        System.out.println("alternate = " + Arrays.toString(alternate));
//        System.out.println("SmithWaterman reference.length = " + n + ", alternate.length =" + m + "");
//        System.out.println("SmithWaterman parameter.open = " + parameters.w_open + " ,parameter.extend =" + parameters.w_extend + " ,parameter.match =" + parameters.w_match + " ,parameter.mismatch =" + parameters.w_mismatch);
//        System.out.println("SmithWaterman overhang_strategy = " + overhang_strategy);
//        long start_time1 = System.nanoTime();
        int[][] sw = new int[n][m];
        if ( keepScoringMatrix ) SW = sw;
        int[][] btrack=new int[n][m];

//        long end_time1 = System.nanoTime();
//        double difference1 = (end_time1 - start_time1)/1e6;
//        long start_time2 = System.nanoTime();
//        calculateMatrix(reference, alternate, sw, btrack);

//        long end_time2 = System.nanoTime();
//        double difference2 = (end_time2 - start_time2)/1e6;
//        long start_time3 = System.nanoTime();
//        alignmentResult = calculateCigar(sw, btrack, overhang_strategy); // length of the segment (continuous matches, insertions or deletions)

//        long end_time3 = System.nanoTime();
//        double difference3 = (end_time3 - start_time3)/1e6;
//        System.out.println("SmithWaterman difference1 = " + difference1 + ", difference2 = " + difference2 + ", difference3 = " + difference3);
//        double all = difference1 + difference2 + difference3;
//        System.out.println("SmithWaterman ratio1 = " + difference1/all + ", ration2 = " + difference2/all + ", ratio3 = " + difference3/all);
//        if(max_length_ref<n) max_length_ref=n;
//        if(max_length_alt<m) max_length_alt=m;
//        System.out.println("SmithWaterman reference.length max = " + max_length_ref + ", alternate.length max =" + max_length_alt + "");
        if(count_sw == count_debug) {
            System.out.println("SmithWaterman strategy = " + overhang_strategy);
            System.out.println("SmithWaterman reference.length = " + (n-1) + ", alternate.length =" + (m-1));
            try {
                Writer writer_sw = new FileWriter("sw.dat", true);
                Writer writer_btrack = new FileWriter("btrack.dat", true);
                int strategy_c=0;

                for (int i = 0; i < reference.length+1; i++) {
                    for (int j = 0; j < alternate.length+1; j++) {
                        writer_sw.write(String.valueOf(sw[i][j]) + "\n");
                        writer_btrack.write(String.valueOf(btrack[i][j]) + "\n");
                    }
                }

                writer_sw.close();
                writer_btrack.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            ;
        }

        count_sw++;
//        System.out.println("SmithWaterman count = " + count_sw);
    }

    /**
     * Calculates the SW matrices for the given sequences
     *
     * @param reference  ref sequence
     * @param alternate  alt sequence
     * @param sw         the Smith-Waterman matrix to populate
     * @param btrack     the back track matrix to populate
     */
    protected void calculateMatrix(final byte[] reference, final byte[] alternate, final int[][] sw, final int[][] btrack) {
        calculateMatrix(reference, alternate, sw, btrack, overhang_strategy);
    }

    /**
     * Calculates the SW matrices for the given sequences
     *
     * @param reference  ref sequence
     * @param alternate  alt sequence
     * @param sw         the Smith-Waterman matrix to populate
     * @param btrack     the back track matrix to populate
     * @param overhang_strategy    the strategy to use for dealing with overhangs
     */
    protected void calculateMatrix(final byte[] reference, final byte[] alternate, final int[][] sw, final int[][] btrack, final OVERHANG_STRATEGY overhang_strategy) {
        if ( reference.length == 0 || alternate.length == 0 )
            throw new IllegalArgumentException("Non-null, non-empty sequences are required for the Smith-Waterman calculation");
        try {
            Writer writer_debug_sw = new FileWriter("debug_sw.dat", true);

            final int ncol = sw[0].length;//alternate.length+1; formerly m
            final int nrow = sw.length;// reference.length+1; formerly n

            final int MATRIX_MIN_CUTOFF;   // never let matrix elements drop below this cutoff
            if (cutoff) MATRIX_MIN_CUTOFF = 0;
            else MATRIX_MIN_CUTOFF = (int) -1e8;
            //System.out.println("MATRIX_MIN_CUTOFF = " + MATRIX_MIN_CUTOFF);

            //int lowInitValue=Integer.MIN_VALUE/2;
            int lowInitValue = -(int) 1e31 / 2 - 1;
//        System.out.println("lowInitValue = " + lowInitValue);
//        System.out.println("lowInitValue ref = " + Integer.MIN_VALUE/2);
            final int[] best_gap_v = new int[ncol + 1];
            //Arrays.fill(best_gap_v, lowInitValue);
            for (int i = 0; i < ncol + 1; i++) {
                best_gap_v[i] = lowInitValue;
            }
            final int[] gap_size_v = new int[ncol + 1];
            final int[] best_gap_h = new int[nrow + 1];
            //Arrays.fill(best_gap_h,lowInitValue);
            for (int i = 0; i < nrow + 1; i++) {
                best_gap_h[i] = lowInitValue;
            }

            final int[] gap_size_h = new int[nrow + 1];

            // we need to initialize the SW matrix with gap penalties if we want to keep track of indels at the edges of alignments
            if (overhang_strategy == OVERHANG_STRATEGY.INDEL || overhang_strategy == OVERHANG_STRATEGY.LEADING_INDEL) {
                // initialize the first row
                int[] topRow = sw[0];
                topRow[1] = parameters.w_open;
                int currentValue = parameters.w_open;
                for (int i = 2; i < topRow.length; i++) {
                    currentValue += parameters.w_extend;
                    topRow[i] = currentValue;
                    //if(count_sw == count_debug)
                    //    System.out.println("sw[0][" + i + "] = " + sw[0][i]);
                }
                // initialize the first column
                sw[1][0] = parameters.w_open;
                currentValue = parameters.w_open;
                for (int i = 2; i < sw.length; i++) {
                    currentValue += parameters.w_extend;
                    sw[i][0] = currentValue;
                }
            }
            if(count_sw == count_debug) {
                try {
                    Writer writer_sw_init = new FileWriter("sw_init.dat", true);
                    Writer writer_btrack_init = new FileWriter("btrack_init.dat", true);

                    for (int i = 0; i < reference.length+1; i++) {
                        for (int j = 0; j < alternate.length+1; j++) {
                            writer_sw_init.write(String.valueOf(sw[i][j]) + "\n");
                            writer_btrack_init.write(String.valueOf(btrack[i][j]) + "\n");
                        }
                    }

                    writer_sw_init.close();
                    writer_btrack_init.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                ;
            }
            // build smith-waterman matrix and keep backtrack info:
            int[] curRow = sw[0];
            if(count_sw == count_debug) writer_debug_sw.write("Data=" + String.valueOf(curRow[1]) + "\n");
            for (int i = 1; i < sw.length; i++) {
                final byte a_base = reference[i - 1]; // letter in a at the current pos
                final int[] lastRow = curRow;
                curRow = sw[i];
                final int[] curBackTrackRow = btrack[i];
                for (int j = 1; j < curRow.length; j++) {
                    if(count_sw == count_debug) writer_debug_sw.write("Data=" + String.valueOf(lastRow[j]) + "\n");
                    //if(count_sw == count_debug) writer_debug_sw.write("00 " + String.valueOf(best_gap_v[j]) + ", " + String.valueOf(best_gap_h[i]) + "\n");
                    final byte b_base = alternate[j - 1]; // letter in b at the current pos
                    // in other words, step_diag = sw[i-1][j-1] + wd(a_base,b_base);
                    final int step_diag = lastRow[j - 1] + wd(a_base, b_base);
                    //if(count_sw == count_debug) System.out.println("SmithWaterman parameter.open = " + parameters.w_open + " ,parameter.extend =" + parameters.w_extend + " ,parameter.match =" + parameters.w_match + " ,parameter.mismatch =" + parameters.w_mismatch);
                    //if(count_sw == count_debug) writer_debug_sw.write("11 " + String.valueOf(lastRow[j - 1]) + ", " + wd(a_base, b_base) + "\n");
                    //if(count_sw == count_debug) writer_debug_sw.write("12 " + String.valueOf(step_diag) + "\n");

                    // optimized "traversal" of all the matrix cells above the current one (i.e. traversing
                    // all 'step down' events that would end in the current cell. The optimized code
                    // does exactly the same thing as the commented out loop below. IMPORTANT:
                    // the optimization works ONLY for linear w(k)=wopen+(k-1)*wextend!!!!

                    // if a gap (length 1) was just opened above, this is the cost of arriving to the current cell:
                    int prev_gap = lastRow[j] + parameters.w_open;
                    best_gap_v[j] += parameters.w_extend; // for the gaps that were already opened earlier, extending them by 1 costs w_extend
                    if (prev_gap > best_gap_v[j]) {
                        // opening a gap just before the current cell results in better score than extending by one
                        // the best previously opened gap. This will hold for ALL cells below: since any gap
                        // once opened always costs w_extend to extend by another base, we will always get a better score
                        // by arriving to any cell below from the gap we just opened (prev_gap) rather than from the previous best gap
                        best_gap_v[j] = prev_gap;
                        gap_size_v[j] = 1; // remember that the best step-down gap from above has length 1 (we just opened it)
                    } else {
                        // previous best gap is still the best, even after extension by another base, so we just record that extension:
                        gap_size_v[j]++;
                    }

                    final int step_down = best_gap_v[j];
                    final int kd = gap_size_v[j];
                    if(count_sw == count_debug) writer_debug_sw.write("Data=" + String.valueOf(lastRow[j]) + "," + String.valueOf(prev_gap) + "," + String.valueOf(best_gap_v[j]) + "," + String.valueOf(step_down) + "\n");
                    //if(count_sw == count_debug) writer_debug_sw.write("21 " + String.valueOf(lastRow[j]) + "\n");
                    //if(count_sw == count_debug) writer_debug_sw.write("22 " + String.valueOf(step_down) + "\n");

                    // optimized "traversal" of all the matrix cells to the left of the current one (i.e. traversing
                    // all 'step right' events that would end in the current cell. The optimized code
                    // does exactly the same thing as the commented out loop below. IMPORTANT:
                    // the optimization works ONLY for linear w(k)=wopen+(k-1)*wextend!!!!

                    prev_gap = curRow[j - 1] + parameters.w_open; // what would it cost us to open length 1 gap just to the left from current cell
                    best_gap_h[i] += parameters.w_extend; // previous best gap would cost us that much if extended by another base
                    //if(count_sw == count_debug) writer_debug_sw.write("30 " + String.valueOf(curRow[j-1]) + "\n");
                    if (prev_gap > best_gap_h[i]) {
                        // newly opened gap is better (score-wise) than any previous gap with the same row index i; since
                        // gap penalty is linear with k, this new gap location is going to remain better than any previous ones
                        best_gap_h[i] = prev_gap;
                        gap_size_h[i] = 1;
                    } else {
                        gap_size_h[i]++;
                    }

                    final int step_right = best_gap_h[i];
                    final int ki = gap_size_h[i];
                    //if(count_sw == count_debug) writer_debug_sw.write("31 " + String.valueOf(parameters.w_extend) + "\n");
                    //if(count_sw == count_debug) writer_debug_sw.write("32 " + String.valueOf(step_right) + "\n");

                    //priority here will be step diagonal, step right, step down
                    final boolean diagHighestOrEqual = (step_diag >= step_down)
                            && (step_diag >= step_right);

                    if (diagHighestOrEqual) {
                        //curRow[j]=Math.max(MATRIX_MIN_CUTOFF,step_diag);
                        if (MATRIX_MIN_CUTOFF > step_diag) {
                            curRow[j] = MATRIX_MIN_CUTOFF;
                        } else {
                            curRow[j] = step_diag;
                        }
                        curBackTrackRow[j] = 0;
                    } else if (step_right >= step_down) { //moving right is the highest
                        //curRow[j]=Math.max(MATRIX_MIN_CUTOFF,step_right);
                        if (MATRIX_MIN_CUTOFF > step_right) {
                            curRow[j] = MATRIX_MIN_CUTOFF;
                        } else {
                            curRow[j] = step_right;
                        }
                        curBackTrackRow[j] = -ki; // negative = horizontal
                    } else {
                        //curRow[j]=Math.max(MATRIX_MIN_CUTOFF,step_down);
                        if (MATRIX_MIN_CUTOFF > step_down) {
                            curRow[j] = MATRIX_MIN_CUTOFF;
                        } else {
                            curRow[j] = step_down;
                        }
                        curBackTrackRow[j] = kd; // positive=vertical
                    }
                    if(count_sw == count_debug) writer_debug_sw.write("Data=" + String.valueOf(curRow[j]) + "," + String.valueOf(step_diag) + "," + String.valueOf(step_right) + "," + String.valueOf(step_down) + "," + String.valueOf(MATRIX_MIN_CUTOFF) + "\n");
                    if(count_sw == count_debug) writer_debug_sw.write("Data=" + parameters.w_open + "," + parameters.w_extend + "," + parameters.w_match + "," + parameters.w_mismatch + "\n");
                }
            }
            writer_debug_sw.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /*
     * Class to store the result of calculating the CIGAR from the back track matrix
     */
    protected final class SWPairwiseAlignmentResult {
        public final Cigar cigar;
        public final int alignment_offset;
        public SWPairwiseAlignmentResult(final Cigar cigar, final int alignment_offset) {
            this.cigar = cigar;
            this.alignment_offset = alignment_offset;
        }
    }

    /**
     * Calculates the CIGAR for the alignment from the back track matrix
     *
     * @param sw                   the Smith-Waterman matrix to use
     * @param btrack               the back track matrix to use
     * @param overhang_strategy    the strategy to use for dealing with overhangs
     * @return non-null SWPairwiseAlignmentResult object
     */
    protected SWPairwiseAlignmentResult calculateCigar(final int[][] sw, final int[][] btrack, final OVERHANG_STRATEGY overhang_strategy){
        final List<CigarElement> lce = new ArrayList<CigarElement>(5);
        State state = State.MATCH;
        int alignment_offset = 0;
        // p holds the position we start backtracking from; we will be assembling a cigar in the backwards order
        int p1 = 0, p2 = 0;

        int refLength = sw.length-1;
        int altLength = sw[0].length-1;
        int[] state_array = new int[1000];  //size ?
        for(int i=0; i<1000; i++) {
            state_array[i] = 0;
        }
        int state_c = 0;
        int[] length_array = new int[1000];  //size ?
        int count_cigar = 0;

        //int maxscore = Integer.MIN_VALUE; // sw scores are allowed to be negative
        int maxscore =-(int)1e31 - 1;
        int segment_length = 0; // length of the segment (continuous matches, insertions or deletions)
        //System.out.println("maxscore = " + maxscore);
        //System.out.println("maxscore ref = " + Integer.MIN_VALUE);

        // Han : Test Code
        int number = 0;
        try {
            Writer writer_state = new FileWriter("state.dat", true);
            Writer writer_length = new FileWriter("length.dat", true);
            Writer writer_number = new FileWriter("number.dat", true);
            Writer writer_offset = new FileWriter("offset.dat", true);
            Writer writer_state_c = new FileWriter("state_c.dat", true);
            Writer writer_length_c = new FileWriter("length_c.dat", true);
            Writer debug = new FileWriter("debug.dat", true);

            if ( overhang_strategy == OVERHANG_STRATEGY.INDEL ) {
                p1 = refLength;
                p2 = altLength;
                //if(count_sw == count_debug)
                //    System.out.println("0 p1 = " + p1 + ", p2 = " + p2);
            } else {
                // look for the largest score on the rightmost column. we use >= combined with the traversal direction
                // to ensure that if two scores are equal, the one closer to diagonal gets picked
                //Note: this is not technically smith-waterman, as by only looking for max values on the right we are
                //excluding high scoring local alignments
                p2=altLength;
                //if(count_sw == count_debug)
                //    System.out.println("0 p1 = " + p1 + ", p2 = " + p2);

                for(int i=1;i<sw.length;i++)  {
                    final int curScore = sw[i][altLength];
                    if (curScore >= maxscore ) {
                        p1 = i;
                        maxscore = curScore;
                    }
                    //if(count_sw == count_debug)
                    //    System.out.println("xxx1 = " + curScore + ", xxx2 = " + maxscore + ", xxx3 = " + i);
                }
                //if(count_sw == count_debug)
                //    System.out.println("1 p1 = " + p1 + ", p2 = " + p2);
                // now look for a larger score on the bottom-most row
                if ( overhang_strategy != OVERHANG_STRATEGY.LEADING_INDEL ) {
                    final int[] bottomRow=sw[refLength];
                    for ( int j = 1 ; j < bottomRow.length; j++) {
                        int curScore=bottomRow[j];
                        // data_offset is the offset of [n][j]
                        int abs1, abs2;
                        if(refLength > j) {
                            abs1 = refLength - j;
                        } else {
                            abs1 = j - refLength;
                        }
                        if(p1 > p2) {
                            abs2 = p1 - p2;
                        } else {
                            abs2 = p2 - p1;
                        }
                        if ( curScore > maxscore ||
                                (curScore == maxscore && abs1 < abs2) ) {
                                //(curScore == maxscore && Math.abs(refLength-j) < Math.abs(p1 - p2) ) ) {
                            p1 = refLength;
                            p2 = j ;
                            maxscore = curScore;
                            segment_length = altLength - j ; // end of sequence 2 is overhanging; we will just record it as 'M' segment
                        }
                    }
                }
            }
            //if(count_sw == count_debug)
            //    System.out.println("2 p1 = " + p1 + ", p2 = " + p2);

            //if(count_sw == count_debug) {System.out.println("xxx : segment_length = " + segment_length + ", state = " + overhang_strategy);}
            if ( segment_length > 0 && overhang_strategy == OVERHANG_STRATEGY.SOFTCLIP ) {
                state_array[count_cigar] = 3;
                length_array[count_cigar] = segment_length;
                if(count_sw<count_sw_max) {
                    writer_state.write(String.valueOf(State.CLIP) + "\n");
                    writer_length.write(String.valueOf(segment_length) + "\n");
                    writer_state_c.write(String.valueOf(3) + "\n");
                    writer_length_c.write(String.valueOf(length_array[count_cigar]) + "\n");
                    debug.write(String.valueOf(0) + " " + String.valueOf(p1) + " " + String.valueOf(p2) + " " + String.valueOf(length_array[count_cigar]) + " " + String.valueOf(state_array[count_cigar]) + "\n");
                    number++;
                }
                //if(count_sw == count_debug) {System.out.println("1 : cigar = " + count_cigar + ", state = " + state_array[count_cigar] + ", length = " + length_array[count_cigar]);}
                count_cigar++;
                //lce.add(makeElement(State.CLIP, segment_length));
                segment_length = 0;
            }

            // we will be placing all insertions and deletions into sequence b, so the states are named w/regard
            // to that sequence
            do {
                //if(count_sw == count_debug)
                //    System.out.println("p1 = " + p1 + ", p2 = " + p2 + ", btrack = " + btrack[p1][p2] + ", cigar = " + count_cigar);
                int btr = btrack[p1][p2];
                State new_state;
                int new_state_c;
                int step_length = 1;
                if ( btr > 0 ) {
                    new_state = State.DELETION;
                    new_state_c = 2;
                    step_length = btr;
                } else if ( btr < 0 ) {
                    new_state = State.INSERTION;
                    new_state_c = 1;
                    step_length = (-btr);
                } else {
                    new_state = State.MATCH; // and step_length =1, already set above
                    new_state_c = 0;
                }

                // move to next best location in the sw matrix:
                //switch( new_state ) {
                //    case MATCH:  p1--; p2--; break; // move back along the diag in the sw matrix
                //    case INSERTION: p2 -= step_length; break; // move left
                //    case DELETION:  p1 -= step_length; break; // move up
                //}
                switch( new_state_c ) {
                    case 0: p1--; p2--; break; // move back along the diag in the sw matrix
                    case 1: p2 -= step_length; break; // move left
                    case 2: p1 -= step_length; break; // move up
                }

                // now let's see if the state actually changed:
                //if ( new_state == state ) segment_length+=step_length;
                if ( new_state_c == state_c ) segment_length+=step_length;
                else {
                    // state changed, lets emit previous segment, whatever it was (Insertion Deletion, or (Mis)Match).
                    state_array[count_cigar] = state_c;
                    length_array[count_cigar] = segment_length;
                    if(count_sw<count_sw_max) {
                        writer_state.write(String.valueOf(state) + "\n");
                        writer_length.write(String.valueOf(segment_length) + "\n");
                        //writer_length.write(segment_length);
                        writer_state_c.write(String.valueOf(state_array[count_cigar]) + "\n");
                        writer_length_c.write(String.valueOf(length_array[count_cigar]) + "\n");
                        debug.write(String.valueOf(1) + " " + String.valueOf(p1) + " " + String.valueOf(p2) + " " + String.valueOf(length_array[count_cigar]) + " " + String.valueOf(state_array[count_cigar]) + "\n");
                        number++;
                    }
                    //if(count_sw == count_debug) {System.out.println("2 : cigar = " + count_cigar + ", state = " + state_array[count_cigar] + ", length = " + length_array[count_cigar]);}
                    count_cigar++;
                    //lce.add(makeElement(state, segment_length));
                    segment_length = step_length;
                    state = new_state;
                    segment_length = step_length;
                    state_c = new_state_c;
                }
                // next condition is equivalent to  while ( sw[p1][p2] != 0 ) (with modified p1 and/or p2:
            } while ( p1 > 0 && p2 > 0 );

            // post-process the last segment we are still keeping;
            // NOTE: if reads "overhangs" the ref on the left (i.e. if p2>0) we are counting
            // those extra bases sticking out of the ref into the first cigar element if DO_SOFTCLIP is false;
            // otherwise they will be softclipped. For instance,
            // if read length is 5 and alignment starts at offset -2 (i.e. read starts before the ref, and only
            // last 3 bases of the read overlap with/align to the ref), the cigar will be still 5M if
            // DO_SOFTCLIP is false or 2S3M if DO_SOFTCLIP is true.
            // The consumers need to check for the alignment offset and deal with it properly.
            if ( overhang_strategy == OVERHANG_STRATEGY.SOFTCLIP ) {
                state_array[count_cigar] = state_c;
                length_array[count_cigar] = segment_length;
                if(count_sw<count_sw_max) {
                    writer_state.write(String.valueOf(state) + "\n");
                    writer_length.write(String.valueOf(segment_length) + "\n");
                    writer_state_c.write(String.valueOf(state_array[count_cigar]) + "\n");
                    writer_length_c.write(String.valueOf(length_array[count_cigar]) + "\n");
                    debug.write(String.valueOf(2) + " " + String.valueOf(p1) + " " + String.valueOf(p2) + " " + String.valueOf(length_array[count_cigar]) + " " + String.valueOf(state_array[count_cigar]) + "\n");
                    number++;
                }
                //if(count_sw == count_debug) {System.out.println("3 : cigar = " + count_cigar + ", state = " + state_array[count_cigar] + ", length = " + length_array[count_cigar]);}
                count_cigar++;
                //lce.add(makeElement(state, segment_length));
                if ( p2 > 0 ) {
                    state_array[count_cigar] = 3;
                    length_array[count_cigar] = p2;
                    if(count_sw<count_sw_max) {
                        writer_state.write(String.valueOf(State.CLIP) + "\n");
                        writer_length.write(String.valueOf(p2) + "\n");
                        writer_state_c.write(String.valueOf(state_array[count_cigar]) + "\n");
                        writer_length_c.write(String.valueOf(length_array[count_cigar]) + "\n");
                        debug.write(String.valueOf(3) + " " + String.valueOf(p1) + " " + String.valueOf(p2) + " " + String.valueOf(length_array[count_cigar]) + " " + String.valueOf(state_array[count_cigar]) + "\n");
                        number++;
                    }
                    //if(count_sw == count_debug) {System.out.println("4 : cigar = " + count_cigar + ", state = " + state_array[count_cigar] + ", length = " + length_array[count_cigar]);}
                    count_cigar++;
                    //lce.add(makeElement(State.CLIP, p2));
                }
                alignment_offset = p1;
            } else if ( overhang_strategy == OVERHANG_STRATEGY.IGNORE ) {
                state_array[count_cigar] = state_c;
                length_array[count_cigar] = segment_length + p2;
                if(count_sw<count_sw_max) {
                    writer_state.write(String.valueOf(state) + "\n");
                    writer_length.write(String.valueOf(segment_length + p2) + "\n");
                    writer_state_c.write(String.valueOf(state_array[count_cigar]) + "\n");
                    writer_length_c.write(String.valueOf(length_array[count_cigar]) + "\n");
                    debug.write(String.valueOf(4) + " " + String.valueOf(p1) + " " + String.valueOf(p2) + " " + String.valueOf(length_array[count_cigar]) + " " + String.valueOf(state_array[count_cigar]) + "\n");
                    number++;
                }
                //if(count_sw == count_debug) {System.out.println("5 : cigar = " + count_cigar + ", state = " + state_array[count_cigar] + ", length = " + length_array[count_cigar]);}
                count_cigar++;
                //lce.add(makeElement(state, segment_length + p2));
                alignment_offset = p1 - p2;
            } else {  // overhang_strategy == OVERHANG_STRATEGY.INDEL || overhang_strategy == OVERHANG_STRATEGY.LEADING_INDEL

                // take care of the actual alignment
                state_array[count_cigar] = state_c;
                length_array[count_cigar] = segment_length;
                if(count_sw<count_sw_max) {
                    writer_state.write(String.valueOf(state) + "\n");
                    writer_length.write(String.valueOf(segment_length) + "\n");
                    writer_state_c.write(String.valueOf(state_array[count_cigar]) + "\n");
                    writer_length_c.write(String.valueOf(length_array[count_cigar]) + "\n");
                    debug.write(String.valueOf(5) + " " + String.valueOf(p1) + " " + String.valueOf(p2) + " " + String.valueOf(length_array[count_cigar]) + " " + String.valueOf(state_array[count_cigar]) + "\n");
                    number++;
                }
                //if(count_sw == count_debug) {System.out.println("6 : cigar = " + count_cigar + ", state = " + state_array[count_cigar] + ", length = " + length_array[count_cigar]);}
                count_cigar++;
                //lce.add(makeElement(state, segment_length));

                // take care of overhangs at the beginning of the alignment
                if ( p1 > 0 ) {
                    state_array[count_cigar] = 2;
                    length_array[count_cigar] = p1;
                    if(count_sw<count_sw_max) {
                        writer_state.write(String.valueOf(State.DELETION) + "\n");
                        writer_length.write(String.valueOf(p1) + "\n");
                        writer_state_c.write(String.valueOf(state_array[count_cigar]) + "\n");
                        writer_length_c.write(String.valueOf(length_array[count_cigar]) + "\n");
                        debug.write(String.valueOf(6) + " " + String.valueOf(p1) + " " + String.valueOf(p2) + " " + String.valueOf(length_array[count_cigar]) + " " + String.valueOf(state_array[count_cigar]) + "\n");
                        number++;
                    }
                    //if(count_sw == count_debug) {System.out.println("7 : cigar = " + count_cigar + ", state = " + state_array[count_cigar] + ", length = " + length_array[count_cigar]);}
                    count_cigar++;
                    //lce.add(makeElement(State.DELETION, p1));
                } else if ( p2 > 0 ) {
                    state_array[count_cigar] = 1;
                    length_array[count_cigar] = p2;
                    if(count_sw<count_sw_max) {
                        writer_state.write(String.valueOf(State.INSERTION) + "\n");
                        writer_length.write(String.valueOf(p2) + "\n");
                        writer_state_c.write(String.valueOf(state_array[count_cigar]) + "\n");
                        writer_length_c.write(String.valueOf(length_array[count_cigar]) + "\n");
                        debug.write(String.valueOf(7) + " " + String.valueOf(p1) + " " + String.valueOf(p2) + " " + String.valueOf(length_array[count_cigar]) + " " + String.valueOf(state_array[count_cigar]) + "\n");
                        number++;
                    }
                    //if(count_sw == count_debug) {System.out.println("8 : cigar = " + count_cigar + ", state = " + state_array[count_cigar] + ", length = " + length_array[count_cigar]);}
                    count_cigar++;
                    //lce.add(makeElement(State.INSERTION, p2));
                }

                alignment_offset = 0;
            }

            if(count_sw<count_sw_max) {
                writer_number.write(String.valueOf(number) + "\n");
                writer_offset.write(String.valueOf(alignment_offset) + "\n");
            }
            writer_state.close();
            writer_length.close();
            writer_state_c.close();
            writer_length_c.close();
            writer_number.close();
            writer_offset.close();
            debug.close();
        } catch (IOException e) {
            e.printStackTrace();
        };

// Han test
//        for(int i=0; i<count_cigar; i++) {
//            System.out.println("REF SmithWaterman state = " + state_array[i] + ", length =" + length_array[i]);
//        }
        State state_final = State.MATCH;
        for(int i=0; i<count_cigar; i++) {
            switch (state_array[i]) {
                case 0: state_final = State.MATCH;      break;
                case 1: state_final = State.INSERTION;  break;
                case 2: state_final = State.DELETION;   break;
                case 3: state_final = State.CLIP;       break;
            }
            lce.add(makeElement(state_final, length_array[i]));
        }

        Collections.reverse(lce);
        return new SWPairwiseAlignmentResult(AlignmentUtils.consolidateCigar(new Cigar(lce)), alignment_offset);
    }

    protected SWPairwiseAlignmentResult calculateCigar_new(final byte[] state_array, final int[] length_array, int output_alignment_offset, int output_num) {
        final List<CigarElement> lce = new ArrayList<CigarElement>(5);
        try {
            Writer writer_number_jni = new FileWriter("number_jni.dat", true);
            Writer writer_state_jni  = new FileWriter("state_jni.dat", true);
            Writer writer_length_jni = new FileWriter("length_jni.dat", true);
            Writer writer_offset_jni = new FileWriter("offset_jni.dat", true);

            if(count_sw<count_sw_max) {
                writer_number_jni.write(String.valueOf(output_num) + "\n");
                writer_offset_jni.write(String.valueOf(output_alignment_offset) + "\n");
                for (int i = 0; i < output_num; i++) {
                    writer_state_jni.write(String.valueOf(state_array[i]) + "\n");
                    writer_length_jni.write(String.valueOf(length_array[i]) + "\n");
                }
            }

            writer_state_jni.close();
            writer_length_jni.close();
            writer_number_jni.close();
            writer_offset_jni.close();
        } catch (IOException e) {
            e.printStackTrace();
        };

        State state_final = State.MATCH;
        for (int i = 0; i < output_num; i++) {
            switch (state_array[i]) {
                case 0: state_final = State.MATCH; break;
                case 1: state_final = State.INSERTION; break;
                case 2: state_final = State.DELETION; break;
                case 3: state_final = State.CLIP; break;
            }
            lce.add(makeElement(state_final, length_array[i]));
        }

        Collections.reverse(lce);
        return new SWPairwiseAlignmentResult(AlignmentUtils.consolidateCigar(new Cigar(lce)), output_alignment_offset);
    }

    protected CigarElement makeElement(final State state, final int length) {
        CigarOperator op = null;
        switch (state) {
            case MATCH: op = CigarOperator.M; break;
            case INSERTION: op = CigarOperator.I; break;
            case DELETION: op = CigarOperator.D; break;
            case CLIP: op = CigarOperator.S; break;
        }
        return new CigarElement(length, op);
    }


    private int wd(final byte x, final byte y) {
        return (x == y ? parameters.w_match : parameters.w_mismatch);
    }

    public void printAlignment(byte[] ref, byte[] read) {
        printAlignment(ref,read,100);
    }
    
    public void printAlignment(byte[] ref, byte[] read, int width) {
        StringBuilder bread = new StringBuilder();
        StringBuilder bref = new StringBuilder();
        StringBuilder match = new StringBuilder();

        int i = 0;
        int j = 0;

        final int offset = getAlignmentStart2wrt1();

        Cigar cigar = getCigar();

        if ( overhang_strategy != OVERHANG_STRATEGY.SOFTCLIP ) {

            // we need to go through all the hassle below only if we do not do softclipping;
            // otherwise offset is never negative
            if ( offset < 0 ) {
                for (  ; j < (-offset) ; j++ ) {
                    bread.append((char)read[j]);
                    bref.append(' ');
                    match.append(' ');
                }
                // at negative offsets, our cigar's first element carries overhanging bases
                // that we have just printed above. Tweak the first element to
                // exclude those bases. Here we create a new list of cigar elements, so the original
                // list/original cigar are unchanged (they are unmodifiable anyway!)

                List<CigarElement> tweaked = new ArrayList<CigarElement>();
                tweaked.addAll(cigar.getCigarElements());
                tweaked.set(0,new CigarElement(cigar.getCigarElement(0).getLength()+offset,
                        cigar.getCigarElement(0).getOperator()));
                cigar = new Cigar(tweaked);
            }
        }

        if ( offset > 0 ) { // note: the way this implementation works, cigar will ever start from S *only* if read starts before the ref, i.e. offset = 0
            for (  ; i < getAlignmentStart2wrt1() ; i++ ) {
                bref.append((char)ref[i]);
                bread.append(' ');
                match.append(' ');
            }
        }
        
        for ( CigarElement e : cigar.getCigarElements() ) {
            switch (e.getOperator()) {
                case M :
                    for ( int z = 0 ; z < e.getLength() ; z++, i++, j++  ) {
                        bref.append((i<ref.length)?(char)ref[i]:' ');
                        bread.append((j < read.length)?(char)read[j]:' ');
                        match.append( ( i<ref.length && j < read.length ) ? (ref[i] == read[j] ? '.':'*' ) : ' ' );
                    }
                    break;
                case I :
                    for ( int z = 0 ; z < e.getLength(); z++, j++ ) {
                        bref.append('-');
                        bread.append((char)read[j]);
                        match.append('I');
                    }
                    break;
                case S :
                    for ( int z = 0 ; z < e.getLength(); z++, j++ ) {
                        bref.append(' ');
                        bread.append((char)read[j]);
                        match.append('S');
                    }
                    break;
                case D:
                    for ( int z = 0 ; z < e.getLength(); z++ , i++ ) {
                        bref.append((char)ref[i]);
                        bread.append('-');
                        match.append('D');
                    }
                    break;
                default:
                    throw new GATKException("Unexpected Cigar element:" + e.getOperator());
            }
        }
        for ( ; i < ref.length; i++ ) bref.append((char)ref[i]);
        for ( ; j < read.length; j++ ) bread.append((char)read[j]);

        int pos = 0 ;
        int maxlength = Math.max(match.length(),Math.max(bread.length(),bref.length()));
        while ( pos < maxlength ) {
            print_cautiously(match,pos,width);
            print_cautiously(bread,pos,width);
            print_cautiously(bref,pos,width);
            System.out.println();
            pos += width;
        }
    }

    /** String builder's substring is extremely stupid: instead of trimming and/or returning an empty
     * string when one end/both ends of the interval are out of range, it crashes with an
     * exception. This utility function simply prints the substring if the interval is within the index range
     * or trims accordingly if it is not.
     * @param s
     * @param start
     * @param width
     */
    private static void print_cautiously(StringBuilder s, int start, int width) {
        if ( start >= s.length() ) {
            System.out.println();
            return;
        }
        int end = Math.min(start+width,s.length());
        System.out.println(s.substring(start,end));
    }
}
