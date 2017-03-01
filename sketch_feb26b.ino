#define LOG_OUT 1
#define FFT_N 256

#include <FFT.h>

void setup() {
  Serial.begin(115200); // use the serial port
  TIMSK0 = 0; // turn off timer0 for lower jitter - delay() and millis() killed
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  Serial.print("INITIALIZED");
}

void loop() {
  while(1) { // reduces jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < (FFT_N * 2) ; i += 2) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }
    // window data, then reorder, then run, then take output
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    sei(); // turn interrupts back on

    static int count = 0;

    // Coffee maker tone deterimed to have peaks around buckets 16 and 32

    if (fft_log_out[16] > 40 && fft_log_out[32] > 40) {
        count++;
    } else {
        count = 0;
    }

    // 256 buckets => 6.3 ms sample time
    // Coffee maker tone is about 2 s
    if (count > 100) {
        Serial.print("TONE!\n");
    }
  }
}

