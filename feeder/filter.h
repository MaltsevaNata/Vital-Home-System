#define NUM_READ 10               // buffer size


void sort(int *buf) {
	for (int i = 0; i <= (int) ((NUM_READ / 2) + 1); i++) { 
    for (int m = 0; m < NUM_READ - i - 1; m++) {
      if (buf[m] > buf[m + 1]) {
        int buff = buf[m];
        buf[m] = buf[m + 1];
        buf[m + 1] = buff;
      }
    }
  }
}


int findMedianN(int newVal) {
  static int buffer[NUM_READ]; 
  static byte count = 0;    // counter
  buffer[count] = newVal;
  if (++count >= NUM_READ) count = 0;  // roll-back the buffer
  
  int buf[NUM_READ];    // local buffer for median
  for (byte i = 0; i < NUM_READ; i++) buf[i] = buffer[i];  
  sort(buf);
  int ans = 0;
  if (NUM_READ % 2 == 0) {             // if even (NUM_READ - the last index)
    ans = buf[(int) (NUM_READ / 2)];   // central item
  } else {
    ans = (buf[(int) (NUM_READ / 2)] + buf[((int) (NUM_READ / 2)) + 1]) / 2; // average of 2 items
  }
  return ans;
}
