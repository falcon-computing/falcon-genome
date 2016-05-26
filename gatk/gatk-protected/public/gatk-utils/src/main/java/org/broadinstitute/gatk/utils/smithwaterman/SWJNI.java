package org.broadinstitute.gatk.utils.smithwaterman;

public class SWJNI {
    public native int smithwaterman(int p_w_match, int p_w_mismatch, int p_w_open, int p_w_extend,
                                    byte[] reference, byte[] alternate, int ref_length, int alt_length, int strategy,
                                    byte[] state_array, int[] length_array, int[] output_alignment_offset, int[] output_num );
    //public native int smithwatermantest(int m);
    private int SWJNI() {
        System.out.println("New Smithwaterman JNI");
        return 1;
    }
}
