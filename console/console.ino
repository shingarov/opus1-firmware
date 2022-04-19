/*
 * Ladarevo OPUS.1R2
 * Console firmware
 *
 * Copyright (c) 2009-2022 Ladarevo Software Inc.
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
 * --------------
 * Each message is a single byte of form 2rSDXXXYYY,
 * and is a command to switch a LED on or off.
 * S=0 is OFF, S=1 is ON.
 * D is Display Number: 0=stop knob lights, 1=sevensegment.
 * The five stop knobs in each row are numbered from x=0..4,
 * the three rows of the left jamb are y=0,1,2 and on the
 * right jamb y=3,4,5.
 * The 8 digits of the 7segment display are ....
 *
 * Console->Host:
 * --------------
 * Each message is a single byte reporting that a control has been pressed
 * or released.  The Most-Significant Bit is either 1 (representing change
 * from OFF to ON) or 0 (representing change from ON to OFF).  There is no
 * attempt to interpret any controls as "modifiers" or anything at a level
 * higher than purely physical scan-codes.  For example, reinterpreting M2
 * as a second modifier is done at the host level, not in the firmware.
 *
 * The next most-significant bit divides the 128-point scancode space into
 * the lower half, for the French drawknobs: S0XXXYYY, and the upper half,
 * which is further subdivided into Pistons S100NNNN and Studs S110TTTT.
 *
 * As with the LED direction, the numeration of knob rows and columns is
 * 0-based.  For example, "top leftmost knob pressed" will be 0x80, whereas
 * "lowest rightmost knob released" will be 0x25.
 *
 * The nibble numbering the pistons is as follows:
 *
 *  *0  R
 *  *1  1
 *  *2  4
 *  *3  2
 *  *4  5
 *  *5  3
 *  *6  6
 *  *7  7
 *  *8  M2
 *  *9  RG(0)
 *  *A
 *  *B  SET
 *  *C  BASS
 *  *D  CHIMES
 *  *E  MEL
 *  *F
 *
 * Here the "*" upper nibble is 2r1100 (piston ON) or 2r0100 (piston OFF).
 */


static void report(byte x) {
  Serial.write(x);
}

/**********************************************
 *     LED                                    *
 **********************************************/
#include "LedControl.h"

#define DATA_IN 51
#define CLK 52
#define CS 53

static byte JAMB_COLUMNS[6] = {
  0x02, 0x40, 0x20, 0x80, 0x10, 0x04
};

static byte JAMB_LED_ORDER[] = {
  5,1,0,4,7,
  7,1,0,4,5,
  5,4,7,1,0,
  1,4,5,0,7,
  5,0,4,1,7,
  5,4,1,0,7
};
static byte SEVEN_DIGIT_PLACES[] = {5,1,7,3,2,6,0,4};
static byte jambLedStatus[] = {0,0,0,0, 0,0,0,0};
static byte sevenLedStatus[] = {0,0,0,0, 0,0,0,0};

LedControl lc = LedControl(DATA_IN, CLK, CS, 2);

static void flashLEDs() {
  lc.setDisplayTest(0,1);
  lc.setDisplayTest(1,1);
  delay(200);
  lc.setDisplayTest(0,0);
  lc.setDisplayTest(1,0);
  delay(200);
  lc.setDisplayTest(0,1);
  lc.setDisplayTest(1,1);
  delay(200);
  lc.setDisplayTest(0,0);
  lc.setDisplayTest(1,0);
  delay(200);
  lc.setDisplayTest(0,1);
  lc.setDisplayTest(1,1);
  delay(200);
  lc.setDisplayTest(0,0);
  lc.setDisplayTest(1,0);
  delay(200);
  lc.setDisplayTest(0,1);
  lc.setDisplayTest(1,1);
  delay(200);
  lc.setDisplayTest(0,0);
  lc.setDisplayTest(1,0);
}

static void setupLEDs() {
  lc.shutdown(0,false);
  lc.shutdown(1,false);
  lc.clearDisplay(0);
  lc.clearDisplay(1);
  lc.setIntensity(0,15);
  lc.setIntensity(1,15);
  flashLEDs();

  
}

void JAMB_LED_ON(int x, int y) {
  int row = JAMB_LED_ORDER[y*5 + x];
  jambLedStatus[row] |= JAMB_COLUMNS[y];
  lc.setRow(0, row, jambLedStatus[row]);
}

void JAMB_LED_OFF(int x, int y) {
  int row = JAMB_LED_ORDER[y*5 + x];
  jambLedStatus[row] &= ~JAMB_COLUMNS[y];
  lc.setRow(0, row, jambLedStatus[row]);
}

static void SEVEN_LED_ON(int x, int y) {
  byte seg=1<<y;
  int col = SEVEN_DIGIT_PLACES[x];
  sevenLedStatus[col] |= seg;
  lc.setRow(1, col, sevenLedStatus[col]); 
}

static void SEVEN_LED_OFF(int x, int y) {
  byte seg=1<<y;
  int col = SEVEN_DIGIT_PLACES[x];
  sevenLedStatus[col] &= ~seg;
  lc.setRow(1, col, sevenLedStatus[col]); 
}

static void do_led() {
  if (Serial.available()==0) return;
  unsigned command = Serial.read();
  boolean on = (command & 0x80)? 1:0;
  int displayNo = (command & 0x40)? 1:0;
  int x = (command&0x38)>>3;
  int y = command&0x07;
  if (displayNo) {
    // 7-segment
    if (on)
      SEVEN_LED_ON(x,y);
      else SEVEN_LED_OFF(x,y);
  } else {
    // stop LEDs
    if (on)
      JAMB_LED_ON(x, y);
      else JAMB_LED_OFF(x,y);
  }
  do_led();
}



/**********************************************
 *     STOP DRAWKNOBS                         *
 **********************************************/

#define NUM_SENSING_LINES 5
static int SENSING_LINES[NUM_SENSING_LINES] = {8,9,10,11,12};

#define NUM_PULLDOWN_LINES 6
static int PULLDOWN_LINES[NUM_PULLDOWN_LINES] = {2,3,4,5,6,7};

static void setupStopButtons() {
  for(int i=0; i<NUM_SENSING_LINES; i++)
    pinMode(SENSING_LINES[i], INPUT_PULLUP);
    
  for(int i=0; i<NUM_PULLDOWN_LINES; i++) {
    pinMode(PULLDOWN_LINES[i], INPUT);
    digitalWrite(PULLDOWN_LINES[i], HIGH);
  }
}

static void reportDrawknobPressed(unsigned x, unsigned y) {
  report(x<<3 | y | 0x80);
}

static void reportDrawknobReleased(unsigned x, unsigned y) {
  report(x<<3 | y);
}

static byte drawknobStatus[] = {0,0,0,0,0};

/*
 * Answer true if the corresponding saved bit is set.
 * x = 0..4, y = 0..5
 */
static boolean currentSavedDrawknobStatus(unsigned x, unsigned y) {
  return (drawknobStatus[x] & (1<<y)) != 0;
}

static void saveDrawknobPressed(unsigned x, unsigned y) {
  drawknobStatus[x] |= (1<<y);
}

static void saveDrawknobReleased(unsigned x, unsigned y) {
  drawknobStatus[x] &= ~(1<<y);
}

static void stateXYis(unsigned x, unsigned y, int s) {
    if (s == LOW) {
      if (!currentSavedDrawknobStatus(x,y)) {
        reportDrawknobPressed(x, y);
        saveDrawknobPressed(x,y);
      }
    } else {
      if (currentSavedDrawknobStatus(x,y)) {
        reportDrawknobReleased(x, y);
        saveDrawknobReleased(x,y);
      }
    }
}

static void stateRawXYis(unsigned x, unsigned y, int s) {
  switch(x) {
  case 0: switch(y) {
    case 0: stateXYis(2,3,s); return;
    case 1: stateXYis(2,1,s); return;
    case 2: stateXYis(4,4,s); return;
    case 3: stateXYis(1,5,s); return;
    case 4: stateXYis(0,0,s); return;
    case 5: stateXYis(0,2,s); return;
  }
  case 1: switch(y) {
    case 0: stateXYis(4,3,s); return;
    case 1: stateXYis(3,1,s); return;
    case 2: stateXYis(2,4,s); return;
    case 3: stateXYis(2,5,s); return;
    case 4: stateXYis(4,0,s); return;
    case 5: stateXYis(1,2,s); return;
  }
  case 2: switch(y) {
    case 0: stateXYis(3,3,s); return;
    case 1: stateXYis(4,1,s); return;
    case 2: stateXYis(3,4,s); return;
    case 3: stateXYis(4,5,s); return;
    case 4: stateXYis(2,0,s); return;
    case 5: stateXYis(2,2,s); return;
  }
  case 3: switch(y) {
    case 0: stateXYis(1,3,s); return;
    case 1: stateXYis(0,1,s); return;
    case 2: stateXYis(0,4,s); return;
    case 3: stateXYis(3,5,s); return;
    case 4: stateXYis(3,0,s); return;
    case 5: stateXYis(3,2,s); return;
  }
  case 4: switch(y) {
    case 0: stateXYis(0,3,s); return;
    case 1: stateXYis(1,1,s); return;
    case 2: stateXYis(1,4,s); return;
    case 3: stateXYis(0,5,s); return;
    case 4: stateXYis(1,0,s); return;
    case 5: stateXYis(4,2,s); return;
  }
  }
}

/*
 * Pull on one of the pulldown lines and see what we got.
 * lineToPull is the sequential number (0-based),
 * 0<=lineToPull<NUM_PULLDOWN_LINES.
 */
static void lineCycle(int lineToPull) {
  int pin = PULLDOWN_LINES[lineToPull];
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(5);
  for (int in=0; in<NUM_SENSING_LINES; in++) {
    stateRawXYis(in, lineToPull, digitalRead(SENSING_LINES[in]));
  }
  pinMode(pin, INPUT);
}

/*
 * Scan all stop drawknobs once.
 * Report any changes found.
 */
static void do_stops() {
  for (int lineToPull=0; lineToPull<NUM_PULLDOWN_LINES; lineToPull++)
    lineCycle(lineToPull);
}



/**********************************************
 *     STUDS                                  *
 **********************************************/

#define NUM_DIGITAL_PINS 4
// Order is important!!!
static unsigned char DigitalInPins[NUM_DIGITAL_PINS] = {
  42, 38, 40, 44
};
static unsigned char DigitalStatus[NUM_DIGITAL_PINS];

static void setup_studs() {
  for (int i=0; i<NUM_DIGITAL_PINS; i++) {
    int pin = DigitalInPins[i];
    pinMode(pin, INPUT);
    digitalWrite(pin, HIGH);
    DigitalStatus[i]=1;
  }
}
static void reportToeStud(unsigned stud, unsigned updown) {
  report(stud | ((updown==0)?0x80:0) | 0x60);
}

static void do_digital() {
  for (int i=0; i<NUM_DIGITAL_PINS; i++) {
    int pin = DigitalInPins[i];
    unsigned x = digitalRead(pin);
    if (x != DigitalStatus[i]) {
      DigitalStatus[i] = x;
      // Report to host that digital pull-down line i is in state x
      reportToeStud(i, x);
    }
  }
}


/**********************************************
 *     PISTONS                                *
 **********************************************/

static int PistonRowPullPins[2] = { 33, 35 };
static int PistonColumnPins[8] = { 23,25,26,27,28,29,30,32 };
static int piston_status[2] = { 0xFF, 0xFF };

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

static void reportPiston(int x, int y, int s) {
  report((s?0:0x80) | 0x40 | (x<<3) | y);
}

static void do_pistons_row(unsigned row) {
  for (int column=0; column<8; column++) {
    int x = digitalRead(PistonColumnPins[column]);
    int saved = (piston_status[row]>>column) & 1;
    if (x != saved) {
      reportPiston(row, column, x);

      // Update saved status
      piston_status[row] ^= (1 << column);
    }
  }
  // release
  digitalWrite(PistonRowPullPins[row], HIGH);
  digitalWrite(PistonRowPullPins[row? 0 : 1], LOW);
}

static void do_pistons() {
  do_pistons_row(0);
  delay(5);
  do_pistons_row(1);
}



/**********************************************/

void setup() {
  setupStopButtons();
  setup_pistons();
  setup_studs();
  setupLEDs();
  Serial.begin(9600);
}

void loop() {
  do_stops();
  do_led();
  do_pistons();
  do_digital();
//  delay(30);
}

