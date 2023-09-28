/* 
 * KEATACLOCK clone for Arduinno ATmega168/326
 * 1 digit segment LCD
 */

#define DECODE_NEC	// Includes Apple and Onkyo
#define LCD_FREQ2 50	// LCD AC drive frequency / 2

#include <TimerOne.h>
#include <Arduino.h>

#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc.
#include <IRremote.hpp>

// IR feedback LED pin
#define LED 17

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
#define DIG0 14
// #define DIG1 15
// #define DIG2 16

// 7 SEGMENT LCD pin assign
#define SEGMENTS 8
const int segments[SEGMENTS + 1] =
  {
   13, // DP
    5, // G
    8, // F
   10, // E
   11, // D
   12, // C
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

volatile uint8_t n = 0; // display value
volatile uint8_t f = 0; // LCD freq counter
volatile uint8_t keta = 0; // my digit to display
// volatile bool sIRDataJustReceived = false; // no means

// get my digit to display
void getKeta() {
  int d = 0;
  for (int i = 0; i < 3; i++) {
    if (!digitalRead(DIG0 + i)) {
      d += (1 << i);
    }
  }
  keta = d;
}

// callback for IR receiver
void callbackIr(void) {
  static uint16_t oldAddr = 0;
  static uint8_t oldCommand = 0;
  
  IrReceiver.decode(); // fill IrReceiver.decodedIRData
  // IrReceiver.printIRResultShort(&Serial);

  if (IrReceiver.decodedIRData.protocol == NEC
      && ((oldCommand != IrReceiver.decodedIRData.command)
	  || (oldAddr != IrReceiver.decodedIRData.address))) {
    uint8_t b = 0;
    getKeta();
    if (keta < 6) {
      if (keta < 2) b = IrReceiver.decodedIRData.command;
      else if (keta < 4) b = (IrReceiver.decodedIRData.address & 0xFF);
      else if (keta < 6) b = (IrReceiver.decodedIRData.address >> 8) & 0xFF;
      n = (keta % 2)? b / 10: b % 10;
      f = 0;
      // Serial.print("keta:"); Serial.print(keta);
      // Serial.print(", n:"); Serial.println(n);
      // sIRDataJustReceived = true;
    }
    oldAddr = IrReceiver.decodedIRData.address;
    oldCommand = IrReceiver.decodedIRData.command;
  }

  IrReceiver.resume(); // Enable receiving of the next value
}

// callback for LCD AC drive
void callbackLCD(void)
{
  static bool toggle = true;

  int digit = digits[n];
  if (f > (LCD_FREQ2 / 2)) digit |= 1; // DP

  if (toggle) digit = ~digit;
  for (int i = 0; i < SEGMENTS; i++) {
    digitalWrite(segments[i], (digit & 0x01)? HIGH: LOW);
    digit >>= 1;
  }
  digitalWrite(segments[SEGMENTS], toggle? HIGH: LOW); // COM

  f++;
  if (f > LCD_FREQ2) f = 0;

  toggle = !toggle;
}

// initialize
void setup() {
  Serial.begin(115200);

  // LCD segment pin
  for (int i = 0; i <= SEGMENTS; i++) {
    pinMode(segments[i], OUTPUT);
  }

  // digit selecter pin
  for (int i = 0; i < 3; i++) {
    pinMode(DIG0 + i, INPUT_PULLUP);
  }

  getKeta();
  // Serial.print("KETA:"); Serial.println(keta);

  // start IR receiver
  pinMode(LED, OUTPUT); // LED for feedback
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK, LED);
  IrReceiver.registerReceiveCompleteCallback(callbackIr);
  printActiveIRProtocols(&Serial);
  
  // start timer for LCD drive
  Timer1.initialize(1000000 / LCD_FREQ2); // micro second
  Timer1.attachInterrupt(callbackLCD);
}

// dummy loop
void loop() {
  // if (!sIRDataJustReceived) return;
  // sIRDataJustReceived = false;
  if (keta == 6) n = (n - 1) % 10;
  else if (keta == 7) n = random(10);
  delay(500);
}
