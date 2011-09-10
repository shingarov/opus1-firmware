/*
 * drawknobs.cpp -- firmware for Arduino-based MIDI scanner.
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

#include <WProgram.h>
#include <avr/sleep.h>

void setup();
void loop();

static char s[10];

int getStatus(int out, int in) {
  return (s[out] >> in) & 1;
}

// activate
void turnDown(int out, int in) {
  s[out] &= ~(1<<in);
}

// deactivate
void turnUp(int out, int in) {
  s[out] |= 1<<in;
}

void setup() {
  int l;
  for (l=0; l<10; l++) s[l]=0x3F;

  Serial.begin(9600);
  
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  
  digitalWrite(A0, HIGH);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A3, HIGH);
  digitalWrite(A4, HIGH);
  digitalWrite(A5, HIGH);
}

/*
 * Calculate the code of the knob:
 * 0xXY,
 * X - column (1-9, 1-5, 1-4),
 * Y - row:
 *   0: Organ selection
 *   1-9: Speaking stops
 *   A: Couplers
 */
static unsigned int encode(int in, int out) {
  static unsigned int outputOrder[10] =
    { 3, 2, 4, 5, 0xF, 7, 6, 9, 8, 1 };

  if (in==0) {
    // Organ selection
    return outputOrder[out] << 4;
  }

  if (out==4) {
    return (in<<4) + 0x0A;
  }

  return (in<<4) + outputOrder[out];
}

static void reportPressedButton(int in, int out) {
  unsigned char x = encode(in, out);
  Serial.write(x);
}

void lineCycle(int outputLine) {
  pinMode(outputLine+2, OUTPUT);
  digitalWrite(outputLine+2, LOW);
  delay(2);

  for (int in=0; in<6; in++) {
    int x = digitalRead(A0+in);
    if (x == LOW) {
      if (getStatus(outputLine, in)) {
        turnDown(outputLine, in);
        reportPressedButton(in, outputLine);
    }
      
  }
  if (x==HIGH) {
    if (!getStatus(outputLine,in)) {
      // detected a released button
      turnUp(outputLine,in);
    }
  }
  }

  pinMode(outputLine+2, INPUT);
}

void loop() {
  int outputLine;
  for (outputLine=0; outputLine<10; outputLine++) {
    lineCycle(outputLine);
  }
}


