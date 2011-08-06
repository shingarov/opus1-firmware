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

