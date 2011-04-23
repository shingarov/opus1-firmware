#include "WProgram.h"
#include "PS2.h"
#include "LED.h"

void setup();
void loop();
const int DataPin = 20;
const int IRQpin =  21;

void setup() {
  setupLED();
  initialize_ps2();
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
 * o (bit 7): opcode=0
 * s (bit 6): LED status (1=ON, 0=OFF)
 * xxx (bits 0-2)
 * yyy (bits 3-5)
 *
 * Note that here, x/y are LED matrix coordinates, not Universal
 * Organ Coordinates.
 */
static void executeLedCommand(int command) {
Serial.println(command, HEX);
  int x = command & 0x07;
  int y = (command & 0x38) >> 3;
  if (command & 0x40)
    ledON(x,y);
  else
    ledOFF(x,y);
}

static void executeCommand(int command) {
  if ((command&0x80) == 0) {
    executeLedCommand(command);
  } else {
    // other commands -- TBD
  }
}

static void processInputFromHost() {
  if (Serial.available()==0) return;
  int command = Serial.read();
  executeCommand(command);
}

static void processKeyboard() {
  uint8_t scancode = get_scan_code();
  if (scancode) {
    Serial.println(scancode, HEX);
  }
}

void loop() {
  processInputFromHost();
  processKeyboard();
}


