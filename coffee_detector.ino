#include <FFT.h>
#include <LiquidCrystal.h>

#define LOG_OUT 1
#define FFT_N 256

#define RS_PIN 8
#define ENABLE_PIN 9
#define D4_PIN 4
#define D5_PIN 5
#define D6_PIN 6
#define D7_PIN 7

#define LCD_WIDTH 16
#define LCD_HEIGHT 2

LiquidCrystal lcd(RS_PIN, ENABLE_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN);

unsigned long coffe_done_time = 0;

void setup() {
    lcd.begin(LCD_WIDTH, LCD_HEIGHT);
    lcd.setCursor(0,0);
    lcd.print("WAITING 4 COFFEE");

    ADCSRA = 0xe5; // set the adc to free running mode
    ADMUX = 0x41; // use adc1
    DIDR0 = 0x02; // turn off the digital input for adc1
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

        if (count > 10) {
            coffe_done_time = millis();
            lcd.setCursor(0,0);
            lcd.print(" Coffee is done ");
        }

        while (coffe_done_time) {
            unsigned long since_msec = millis() - coffe_done_time;
            unsigned long minutes = (since_msec / 1000)/60;
            unsigned long seconds = (since_msec / 1000)%60;

            if (minutes > 30) {
                coffe_done_time = 0;
                lcd.setCursor(0,0);
                lcd.print("WAITING 4 COFFEE");
            } else {
                lcd.setCursor(0,1);
                lcd.print("Coffee age:");
                if (minutes < 10) {
                    lcd.print("0");
                }
                lcd.print(minutes, DEC);
                lcd.print(":");
                if (seconds < 10) {
                    lcd.print("0");
                }
                lcd.print(seconds, DEC);
            }

        }
    }
}
