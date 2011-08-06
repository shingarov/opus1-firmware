/** 
 * Ladarevo Opus.1 LED matrix driver.
 * Takes API commands and transforms them
 * into commands understood by MAX7221.
 *
 * API:
 *   setupLED();
 *   ledON(int x, int y);  0<=x<=7, 0<=y<=7;
 *   ledOFF(int x, int y);  0<=x<=7, 0<=y<=7;
 */

#include <WProgram.h>
#include <Sprite.h>
#include <Matrix.h>
#include "LED.h"

// SPI pins.
// On the 328 chip, these are 10, 11, (12), 13.
#define DATA_PIN   11
#define CLOCK_PIN  13
#define CS_PIN     10
#if 0
// On the Mega, these are 51, 52, 53.
#define DATA_PIN   51
#define CLOCK_PIN  52
#define CS_PIN     53
#endif
static Matrix m = Matrix(DATA_PIN, CLOCK_PIN, CS_PIN);

static void illuminate_all(void) {
  int x,y;
  for (x=0; x<8; x++)
  for (y=0; y<8; y++)
    ledON(x,y);
}

void setupLED()
{
  m.clear();
  m.setBrightness(15);
  illuminate_all(); 
}

void ledON(int x, int y)
{
  m.write(y, x, HIGH);
}

void ledOFF(int x, int y)
{
  m.write(y, x, LOW);
}

