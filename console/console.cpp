/*
 * Ladarevo OPUS.1
 * Console firmware
 *
 * Copyright (c) 2009-2011 Ladarevo Software Inc.
 *
 * OPUS.1 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OPUS.1 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OPUS.1.  If not, see <http://www.gnu.org/licenses/>.
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
 * For Analog messages, the form is 1N0000XX XXXXXXXX.
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
#define STEP_DELAY 30

#define MINDELTA 8

#define VOLUME_PIN A2
#define TUNING_ONOFF_PIN 55 // A1
#define TUNING_PIN A0

#define NUM_DIGITAL_PINS 7
static unsigned char DigitalInPins[NUM_DIGITAL_PINS] = {
  48, 49, 50, 51, 52, 53,
  TUNING_ONOFF_PIN
};
static unsigned char DigitalStatus[NUM_DIGITAL_PINS];


#include "WProgram.h"

void setup();
void loop();

static int PistonRowPullPins[2] = { 33, 35 };
static int PistonColumnPins[8] = { 23,25,26,27,28,29,30,32 };
static int piston_status[2] = { 0x7F, 0x7F };
static int row = 0;
static void setup_pistons() {
  for (int row=0; row<=1; row++) {
    pinMode(PistonRowPullPins[row], OUTPUT);
    digitalWrite(PistonRowPullPins[row], HIGH);
  }
  for (int column=0; column<8; column++) {
    pinMode(PistonColumnPins[column], INPUT);
    digitalWrite(PistonColumnPins[column], HIGH);
  }
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

static void setup_led() {
  for (int i=3; i<=13; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
}

void setup() {
  setup_pistons();
  setup_analog();
  setup_digital();
  setup_led();
  Serial.begin(9600);
}

void do_pistons() {
  for (int column=0; column<8; column++) {
    int x = digitalRead(PistonColumnPins[column]);
    int saved = (piston_status[row]>>column) & 1;
    if (x != saved) {
      // Report to the host that piston column@row is now in state x
      Serial.write(x << 6); // first byte
      unsigned char NNNN = column | (row << 3);
      Serial.write(NNNN | 0x80); // second byte

      // Update saved status
      piston_status[row] ^= (1 << column);
    }
  }

  // release
  digitalWrite(PistonRowPullPins[row], HIGH);

  // flip row
  row = row? 0 : 1;

  digitalWrite(PistonRowPullPins[row], LOW);
}

static void report_analog_value(int which, int value) {
  int higher = 0x03 & ((value & 0x0300) >> 8);
  int lower = value & 0x00FF;
  Serial.write(0x80 | (which? 0x40:0x00) | higher);
  Serial.write(lower);
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
      // Report to host that digital pull-down line i is in state x
      Serial.write(x<<6);
      Serial.write(i);
    }
  }
}

void do_led() {
  if (Serial.available()==0) return;
  unsigned command = Serial.read();
  if (command & 0xE0) return; // invalid command, ignore
  unsigned status = (command & 0x10) ? HIGH : LOW;
  unsigned LLLL = command & 0x0F;
  if (LLLL > 10) return; // invalid command, ignore
  digitalWrite(3+LLLL, status);
}

void loop() {
  do_pistons();
  do_analog();
  do_digital();
  do_led();
  
  delay(STEP_DELAY);
}


