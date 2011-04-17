/** 
 * WiseTimer
 * 
 */

#include <WProgram.h>
#include <avr/sleep.h>


//-----------------------------------------------------------------------------
/* Pins used in Wise Clock are as follows:
	analog  0..2          available;
	analog  3 (INPUT)     menu button;
	analog  4, 5          I2C (RTC, eeprom);
	digital 0, 1          serial (tx/rx)
	digital 2             available
	digital 3-13 (OUTPUT) display;
*/
//-----------------------------------------------------------------------------



// push button on analog pin 3; used for menu selection;
#define MENU_BUTTON_PIN   3


// pins used for LED matrix rows (multiplexed);
#define SHIFT_CLOCK_PIN   4
#define STORE_CLOCK_PIN   5
#define SER_DATA_PIN      6

// pins assigned to LED matrix columns;
byte pinForRow[8] = {8, 9, 10, 11, 12, 13, 7, 3};


byte soft_prescaler = 0;
byte activeRow = 0;

// video memory for the 8x8 RG display;
byte screenMem[16] = {0};

// indexes in sprite array;
#define OPTION_SLEEP         9
#define ALL_LIT             11


// maximum number of options in the menu; MUST be incremented when a new option is added!;
#define MAX_OPTION           9

// set default option according to preference;
#define DEFAULT_OPTION       0


// current menu option, a value between 0 and MAX_OPTION;
volatile int menuOption = DEFAULT_OPTION;

float waitBetweenPixels = 0.0;

// last time a command (IR or button) was received; used for debouncing;
volatile unsigned long menuTime = 0;

// indicates that display needs to be refreshed;
boolean mustRefreshDisplay = false;

boolean isCounting = false;

int initialTime = 0;
int currentCount = 0;


void setupLED()
{
  // null out video memory
  for (byte i = 0; i < 16; i++)  screenMem[i] = 0x00;
  
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

  // (Jun 20/09) set unused pins as outputs to save power;
  pinMode(14, OUTPUT);	// analog 0;
  pinMode(15, OUTPUT);  // analog 1;
  pinMode(16, OUTPUT);  // analog 2;
}

void setup() {
  setupLED();
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


void displayActiveRow()
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


void shiftOutRow(byte red, byte green)
{
  digitalWrite(STORE_CLOCK_PIN, LOW);
  shiftOut(SER_DATA_PIN, SHIFT_CLOCK_PIN, LSBFIRST, red);   
  shiftOut(SER_DATA_PIN, SHIFT_CLOCK_PIN, LSBFIRST, green);   
  digitalWrite(STORE_CLOCK_PIN, HIGH);
}


void loop()
{
  int x,y;
  for (x=0; x<8; x++)
    for (y=0; y<8; y++) {
      delay(800);
      ledON(x,y);
    }
  for (x=0; x<8; x++)
    for (y=0; y<8; y++) {
      delay(100);
      ledOFF(x,y);
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

