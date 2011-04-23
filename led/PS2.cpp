/*
  PS2Keyboard.cpp - PS2Keyboard library
  Copyright (c) 2007 Free Software Foundation.  All right reserved.
  Written by Christian Weichel <info@32leaves.net>

  ** Mostly rewritten Paul Stoffregen <paul@pjrc.com>, June 2010
  ** Modified for use beginning with Arduino 13 by L. Abraham Smith, <n3bah@microcompdesign.com> * 
  ** Modified for easy interrup pin assignement on method begin(datapin,irq_pin). Cuningan <cuninganreset@gmail.com> **

  Version 2.0 (June 2010)
  - Buffering added, many scan codes can be captured without data loss
    if your sketch is busy doing other work
  - Shift keys supported, completely rewritten scan code to ascii
  - Slow linear search replaced with fast indexed table lookups
  - Support for Teensy, Arduino Mega, and Sanguino added

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <WProgram.h>
#include "PS2.h"
#include "binary.h"
typedef uint8_t boolean;
typedef uint8_t byte;

#define BUFFER_SIZE 45
static volatile uint8_t buffer[BUFFER_SIZE];
static volatile uint8_t head, tail;
static uint8_t ps2Keyboard_DataPin;
static char ps2Keyboard_CharBuffer=0;

// The ISR for the external interrupt
static void ps2interrupt(void)
{
	static uint8_t bitcount=0;
	static uint8_t incoming=0;
	uint8_t n;

	n = bitcount - 1;
	if (n <= 7) {
		uint8_t val = digitalRead(ps2Keyboard_DataPin);
		incoming |= (val << n);
	}
	bitcount++;
	if (bitcount == 11) {
		uint8_t i = head + 1;
		if (i >= BUFFER_SIZE) i = 0;
		if (i != tail) {
			buffer[i] = incoming;
			head = i;
		}
		bitcount = 0;
		incoming = 0;
	}
}

uint8_t get_scan_code(void)
{
	uint8_t c, i;

	i = tail;
	if (i == head) return 0;
	i++;
	if (i >= BUFFER_SIZE) i = 0;
	c = buffer[i];
	tail = i;
	return c;
}


/*
 * INT-to-PIN correspondence:
 * IRQ 0 = PIN 2,
 * IRQ 1 = PIN 3,
 * IRQ 2 = PIN 21,
 * IRQ 3 = PIN 20,
 * IRQ 4 = PIN 19,
 * IRQ 5 = PIN 18.
 * Opus.1 uses IRQ PIN 21 and Data PIN 20.
 */
#define IRQ_NUMBER 2
#define IRQ_PIN  21
#define DATA_PIN 20

void initialize_ps2() {
  ps2Keyboard_DataPin = DATA_PIN;
  pinMode(IRQ_PIN, INPUT);
  digitalWrite(IRQ_PIN, HIGH);
  pinMode(DATA_PIN, INPUT);
  digitalWrite(DATA_PIN, HIGH);
  head = 0;
  tail = 0;
  attachInterrupt(IRQ_NUMBER, ps2interrupt, FALLING);
}


