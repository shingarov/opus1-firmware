/** 
 * Ladarevo Opus.1 LED matrix driver.
 * Based on FlorinC's LED matrix shield.
 * See timewitharduino.blogspot.com
 *
 * API:
 *   setupLED();
 *   ledON(int x, int y);  0<=x<=7, 0<=y<=7;
 *   ledOFF(int x, int y);  0<=x<=7, 0<=y<=7;
 * 
 * Pins used in Wise Clock are as follows:
 *   analog  0..2          available;
 *   analog  3 (INPUT)     menu button;
 *   analog  4, 5          I2C (RTC, eeprom);
 *   digital 0, 1          serial (tx/rx)
 *   digital 2             available
 *   digital 3-13 (OUTPUT) display;
 */

#include <WProgram.h>
#include <avr/sleep.h>
#include "LED.h"

// pins used for LED matrix rows (multiplexed);
#define SHIFT_CLOCK_PIN   4
#define STORE_CLOCK_PIN   5
#define SER_DATA_PIN      6

// pins assigned to LED matrix columns;
static byte pinForRow[8] = {8, 9, 10, 11, 12, 13, 7, 3};
static byte soft_prescaler = 0;
static byte activeRow = 0;
// video memory for the 8x8 RG display;
static byte screenMem[16] = {0};

static void shiftOutRow(byte red, byte green)
{
  digitalWrite(STORE_CLOCK_PIN, LOW);
  shiftOut(SER_DATA_PIN, SHIFT_CLOCK_PIN, LSBFIRST, red);   
  shiftOut(SER_DATA_PIN, SHIFT_CLOCK_PIN, LSBFIRST, green);   
  digitalWrite(STORE_CLOCK_PIN, HIGH);
}

void setupLED()
{
  // null out video memory
  for (byte i = 0; i < 16; i++)  screenMem[i] = 0xFF;
  
  // Calculation for timer 2
  // 16 MHz / 8 = 2 MHz (prescaler 8)
  // 2 MHz / 256 = 7812 Hz
  // soft_prescaler = 15 ==> 520.8 updates per second
  // 520.8 / 8 rows ==> 65.1 Hz for the complete display
  TCCR2A = 0;           // normal operation
  TCCR2B = (1<<CS21);   // prescaler 8
  TIMSK2 = (1<<TOIE2);  // enable overflow interrupt

  // define outputs for serial shift registers
  pinMode(SHIFT_CLOCK_PIN, OUTPUT);
  pinMode(STORE_CLOCK_PIN, OUTPUT);
  pinMode(SER_DATA_PIN,    OUTPUT);

  // set outputs for the 8 matrix rows;
  for (int i=0; i<8; i++)
    pinMode(pinForRow[i], OUTPUT);
  pinMode(14, OUTPUT);	// analog 0;
  pinMode(15, OUTPUT);  // analog 1;
  pinMode(16, OUTPUT);  // analog 2;
}

static void displayActiveRow()
{
  // disable current row;
  digitalWrite(pinForRow[activeRow], LOW);

  // set next row;
  activeRow = (activeRow+1) % 8;

  // shift out values for this row;
  shiftOutRow(screenMem[activeRow], screenMem[8+activeRow]);
  
  // switch to new row;
  digitalWrite(pinForRow[activeRow], HIGH);
}

/**
 * ISR TIMER2_OVF_vect
 * Gets called 7812 times per second. Used for display.
 */
ISR(TIMER2_OVF_vect)
{
  soft_prescaler++;
  if (soft_prescaler == 15)
  {
    // display the next row
    displayActiveRow();
    soft_prescaler = 0;
  }
}


void ledON(int x, int y)
{
  byte mask = 1<<(7-x);
  screenMem[y+8] = screenMem[y+8] | mask;
}

void ledOFF(int x, int y)
{
  byte mask = 1<<(7-x);
  mask = ~mask;
  screenMem[y+8] = screenMem[y+8] & mask;
}

