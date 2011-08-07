/*
 * Ladarevo Opus.1
 * Console firmware
 *
 * Copyright (c) Ladarevo Software Inc.
 */

/*
 * Wire Protocol
 * =============
 *
 * Host->Console:
 * Each message is a single byte of form 2r000SLLLL.
 * It is a command for the console to switch the LED LLLL
 * either ON (S=1) or OFF (S=0).
 *
 * LLLL:
 *   0    = M2
 *   1-7  = 1-7
 *   8    = BASS
 *   9    = MELODY
 *   10   = CHIMES
 *   11-15 Ignored
 *
 * Console->Host:
 * Each message is two bytes.
 * For Analog messages, the form is 1N00000X XXXXXXXX.
 * N=0 for Volume, N=1 for Tuning; X is the 10-bit value.
 * Do not transmit a change if it is smaller than MINDELTA.
 * 
 * For Digital messages, the form is 0V000000 P000NNNN, where
 * V is ON/OFF,
 * P=1 for Pistons, P=0 for straight pull-down digital lines,
 * NNNN addresses the line.
 *
 * From time to time, the message 0xFFFF may be transmitted
 * for synchronization.
 */

/*
 * Parameters
 */
#define STEP_DELAY 1000

#define MINDELTA 5

#define VOLUME_PIN A2
#define TUNING_ONOFF_PIN 55
#define TUNING_PIN A0

#define NUM_DIGITAL_PINS 1
static unsigned char DigitalInPins[NUM_DIGITAL_PINS] = {
  TUNING_ONOFF_PIN,
};
static unsigned char DigitalStatus[NUM_DIGITAL_PINS];


#include "WProgram.h"

void setup();
void loop();

static void setup_pistons() {
}

static int X1, X2;
static void setup_analog() {
  X1=0;
  X2=0;
  pinMode(VOLUME_PIN, INPUT);
  pinMode(TUNING_PIN, INPUT);
}

static void setup_digital() {
  for (int i=0; i<NUM_DIGITAL_PINS; i++) {
    int pin = DigitalInPins[i];
    pinMode(pin, INPUT);
    digitalWrite(pin, HIGH);
    DigitalStatus[i]=1;
  }
}

void setup_led() {
}

void setup() {
  setup_pistons();
  setup_analog();
  setup_digital();
  setup_led();
  Serial.begin(115200);
}

void do_pistons() {
}

static void report_analog_value(int which, int value) {
//  Serial.println(which? "Tuning" : "Volume");
}

void do_analog() {
  int x1=analogRead(VOLUME_PIN);
  int delta = x1-X1;
  if (delta<0) delta=-delta;
  if (delta > MINDELTA) {
    X1=x1;
    report_analog_value(0, x1);
  }
  int x2=analogRead(TUNING_PIN);
  delta = x2-X2;
  if (delta<0) delta=-delta;
  if (delta > MINDELTA) {
    X2=x2;
    report_analog_value(1, x2);
  }
}

void do_digital() {
  for (int i=0; i<NUM_DIGITAL_PINS; i++) {
    int pin = DigitalInPins[i];
    unsigned x = digitalRead(pin);
    if (x != DigitalStatus[i]) {
      DigitalStatus[i] = x;
      Serial.println(x);
    }
  }
}

void do_led() {
}

void loop() {
  do_pistons();
  do_analog();
  do_digital();
  do_led();
  
  delay(STEP_DELAY);
}


