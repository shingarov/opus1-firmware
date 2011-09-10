/*
 * OK.cpp -- green light driver.
 * This file is part of LADAREVO OPUS.1.
 *
 * Copyright (c) 2009-2011 LADAREVO SOFTWARE INC.
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
#include "OK.h"

#define OK_PIN A0

void setupOK() {
  pinMode(OK_PIN, OUTPUT);
  digitalWrite(OK_PIN, LOW);
}

void okON() {
  digitalWrite(OK_PIN, HIGH);
}

void okOFF() {
  digitalWrite(OK_PIN, LOW);
}

