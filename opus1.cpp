#include "PS2Keyboard.h"
#include "WProgram.h"

void setup();
void loop();
const int DataPin = 20;
const int IRQpin =  21;

void setup() {
  delay(100);
  initialize_ps2();
  Serial.begin(9600);
  Serial.println("***");
}

void loop() {
  uint8_t scancode = get_scan_code();
  if (scancode) {
    Serial.println(scancode, HEX);
  }
}


