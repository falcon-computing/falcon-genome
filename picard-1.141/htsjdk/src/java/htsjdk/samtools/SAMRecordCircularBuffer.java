/*
 * A circular buffer that provides blocking read and blocking write methods.
 * It allows null element, which is used for indicating the finishing of the
 * producer (write).
 * 
 * WARNING: this implementation is NOT thread safe.
 * Only used this class for single producer and single consumer case.
 *
 * @author Muhuan Huang
 */

package htsjdk.samtools;

public class SAMRecordCircularBuffer {
  // single producer, single consumer
  private SAMRecord[] buffer;
  private volatile int head, tail;
  private static final int SIZE = 10000;

  public SAMRecordCircularBuffer() {
    buffer = new SAMRecord[SIZE];
    head = 0;
    tail = 0;
  }

  // blocking read
  public SAMRecord take() throws InterruptedException {
    while (head == tail) { // empty
      Thread.sleep(1);
    }
    SAMRecord e = buffer[tail];
    buffer[tail] = null; // indicating end of input
    tail = inc(tail);
    return e;
  }

  // blocking write 
  public void put(SAMRecord m) throws InterruptedException {
    while (inc(head) == tail) { // full
      Thread.sleep(1);
    }
    buffer[head] = m;
    head = inc(head);
  }

  private int inc(int a) {
    int b = a + 1;
    return (b == SIZE) ? 0 : b;
  }
}
