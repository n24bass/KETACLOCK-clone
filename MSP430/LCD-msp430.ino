/* 
 * KETACLOCK clone - MSP430G2553 Energia
 * 1 digit segment LCD
 */

#include <IRremote.h>


int RECV_PIN = 11;

IRrecv irrecv(RECV_PIN);

decode_results results;

void callbackLCD();

#define LCD_FREQ 50  // LCD AC drive frequency

/*
 * 'KETA' selecter pin assign
 * 
 *            DIG2 DIG1 DIG0
 * sec   0   0    0    0
 * sec  10   0    1    0 
 * min   0   0    0    1
 * min  10   0    1    1 
 * hour  0   1    0    0
 * hour 10   1    0    1
 */
#define DIG0 12
#define DIG1 13
#define DIG2 14

/*
 1 - VCC +3.3V+3.3V
 2 - P1.0 LED
 3 - P1.1 RXD
 4 - P1.2 TXD
 5 - P1.3
 6 - P1.4
 7 - P1.5
 8 - P2.0
 9 - P2.1
 10 - P2.2
 - 
 11 - P2.3
 12 - P2.4
 13 - P2.5
 14 - P1.6
 15 - P1.7 ??
 16 - RESET
 17 - TEST
 18 - P2.7 XOUT
 19 - P2.6 XIN
 20 - GND
*/

// 7 SEGMENT LCD pin assign
#define SEGMENTS 8
const int segments[SEGMENTS + 1] =
  {
   19, // DP
    5, // G
    8, // F
   10, // E
   15, // D
   18, // C
    6, // B
    7, // A
    9, // COM
  };

/*
 * segment pattern
 */
const int digits[10] = {
  0xfc, // 0
  0x60, // 1
  0xDa, // 2
  0xf2, // 3
  0x66, // 4
  0xb6, // 5
  0xbe, // 6
  0xe0, // 7
  0xfe, // 8
  0xf6, // 9
  // 0x00, // none
};

volatile unsigned char n = 0; // display value
volatile unsigned char f = 0; // LCD freq counter
volatile unsigned char keta = 0;

// get my digit to display
void getKeta() {
  unsigned char d = (digitalRead(DIG2) ? 0: 4) + (digitalRead(DIG1) ? 0: 2) + (digitalRead(DIG0) ? 0: 1);
  // Serial.print("KETA:"); Serial.println(d, HEX);
  keta = d; // 
}

// callback for LCD AC drive
void callbackLCD(void)
{
  static bool toggle = true;

  int digit = digits[n];
  if (f > (LCD_FREQ / 2)) digit |= 1; // DP

  if (toggle) digit = ~digit;
  for (int i = 0; i < SEGMENTS; i++) {
    digitalWrite(segments[i], (digit & 0x01)? HIGH: LOW);
    digit >>= 1;
  }
  digitalWrite(segments[SEGMENTS], toggle? HIGH: LOW); // COM

  f++;
  if (f > LCD_FREQ) {
    // n = (n + 1) % 10;
    f = 0;
  }

  toggle = !toggle;
}

// initialize
void setup() {
  Serial.begin(9600);

  // LCD segment pin
  for (int i = 0; i <= SEGMENTS; i++) {
    pinMode(segments[i], OUTPUT);
  }

  // digit selecter pin
  pinMode(DIG0, INPUT_PULLUP);
  pinMode(DIG1, INPUT_PULLUP);
  pinMode(DIG2, INPUT_PULLUP);

  getKeta();
  Serial.print("KETA:"); Serial.println(keta, HEX);

  // start IR receiver
  irrecv.blink13(1);
  irrecv.enableIRIn();
}

unsigned char reverse(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void loop() {
  static unsigned long value = 0; // IR value
  static unsigned long t0 = 0;

  // getKeta();
  if (irrecv.decode(&results) && results.decode_type == NEC && keta < 6) {
    // Serial.print("type:"); Serial.println(results.decode_type, HEX);
    // Serial.println(results.value, HEX);
    if (results.value != value) {
      value = results.value;

      unsigned char b;
      switch (keta / 2) {
        case 0: // sec
          b = 8; break;
        case 1: // min
          b = 24; break;
        case 2:
          b = 16; break;
        // no default        
      }
      b = reverse(value >> b);
      n = (keta % 2) ? b / 10: b % 10;
      while (n > 9) n %= 10;
      f = 0;
    }
    irrecv.resume();
  } else {
    unsigned long t = millis();
    if (t == 0 || t - t0 > 20) {
      callbackLCD();
      t0 = t;
    }
    // n = (n + 1) % 10;
  }
  
}
