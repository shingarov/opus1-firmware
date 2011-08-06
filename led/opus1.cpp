#include "WProgram.h"
#include "LED.h"
#include "OK.h"

void setup();
void loop();

void setup() {
  setupLED();
  setupOK();
  Serial.begin(9600);
}

/*
 * LED ON/OFF commands.
 *
 *  MSB                           LSB
 *  +---+---+---+---+---+---+---+---+
 *  | o | s | y | y | y | x | x | x |
 *  +---+---+---+---+---+---+---+---+
 *
 * Main LED Matrix commands:
 *
 * o (bit 7): opcode=0 - main LED matrix ON/OFF commands.
 *     (Note that in V.1.0, these were the only commands).
 * s (bit 6): LED status (1=ON, 0=OFF)
 * xxx (bits 0-2)
 * yyy (bits 3-5)
 *
 * Note that here, x/y are LED matrix coordinates, not Universal
 * Organ Coordinates.
 *
 * Extended command set: o=1.
 * 
 * 1000 0000  - OK LED OFF
 * 1000 0001  - OK LED ON
 */
static void executeLedCommand(int command) {
  int x = command & 0x07;
  int y = (command & 0x38) >> 3;
  if (command & 0x40)
    ledON(x,y);
  else
    ledOFF(x,y);
}

static void executeExtendedCommand(int command) {
  switch (command) {
    case 0x80:
      okOFF();
      break;
    case 0x81:
      okON();
      break;
  }
}

static void executeCommand(int command) {
  if ((command&0x80) == 0) {
    executeLedCommand(command);
  } else {
    executeExtendedCommand(command);
  }
}

static void processInputFromHost() {
  if (Serial.available()==0) return;
  int command = Serial.read();
  executeCommand(command);
}

void loop() {
  processInputFromHost();
  //processAuxInput();
}


