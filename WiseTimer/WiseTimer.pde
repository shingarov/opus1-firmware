/** 
 * WiseTimer
 * 
 */

#include <WProgram.h>
#include <avr/sleep.h>
#include "sym_KT.h"


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

//-----------------------------------------------------------------------------

// display colours;
#define BLACK   0
#define RED     1
#define GREEN   2
#define ORANGE  3


byte soft_prescaler = 0;
byte activeRow = 0;

// video memory for the 8x8 RG display;
byte screenMem[16] = {0};

// current colour used for display;
byte page = GREEN;



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


void setup()
{
  resetDisplay();
  
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


void resetDisplay()
{
  for (byte i = 0; i < 16; i++)  screenMem[i] = 0x00;
}


// added Feb14/09 to delete the display by moving a dot from
// the upper left corner to the lower right corner;
void resetDisplayByMovingDot()
{
  byte mask;

  // the dot has the color of the current page;
  // REM: will not work for orange (both pages would need to be masked dot by dot);
  byte idx = ((page & GREEN) == GREEN)? 0 : 8;

  for (byte y=0; y < 8; y++)
  {
    for (byte x = 0; x < 8; x++)
    {
      // show dot at the particular location;
      mask = 1<<(7-x);
      screenMem[y+idx] = screenMem[y+idx] | mask;

      delay(20);

      // delete dot from the particular location, leaving location empty;
      mask = 0x7F >> x;
      screenMem[y+idx] = screenMem[y+idx] & mask;
    }
  }
}


// statically displays the given sprite;
void setScreenMem(byte color, byte sprite[8])
{
  byte row;
  for (byte i = 0; i < 8; i++)
  {
    row = sprite[i];
    if ((color & RED)   == RED)    screenMem[i+8] = row;
    if ((color & GREEN) == GREEN)  screenMem[i]   = row;
  }
}



void loop()
{
  int x,y;
  for (x=0; x<8; x++)
    for (y=0; y<8; y++) {
      delay(600);
      setPixel(RED, x,y);
    }
  delay(900000);
}



void sleepNow()
{
}
void wakeUpNow()        // here the interrupt is handled after wakeup
{
}
void bigOptionSwitch()
{
}


void countDown()
{
  currentCount--;

  // change the pixels accordingly;
//  displayTransition_RowByRow(currentCount);
  displayTransition_Random(currentCount);

  delay(waitBetweenPixels);

  if (currentCount == 0 && initialTime > 0)
  {
    // stop the current countdown;
    isCounting = false;
    initialTime = 0;
    
    // REM: ring the bell;
    return;
  }

}


void displayTransition_RowByRow(int trNum)
{
  Serial.print("Transition number ");
  Serial.println(trNum);
  
  // trNum is between 120 and 0;
  // for trNum between 120 and 60, pixel changes color from green to yellow;
  // for trNum between  59 and  0, pixel color changes from yellow to red;

  // calculate pixel's location;
  int x = (trNum%60) % 8;
  int y = (trNum%60) / 8;

  setPixel(BLACK, x, y);
  setPixel((trNum >= 60 ? ORANGE : RED), x, y);
}


void displayTransition_Random(int trNum)
{
}


void setPixel(int color, int x, int y)
{
  byte mask = 1<<(7-x);

  if (color == RED)
  {
    screenMem[y+8] = screenMem[y+8] | mask;  // red
  }
  else if (color == GREEN)
  {
    screenMem[y]   = screenMem[y]   | mask;  // green
  }
  else if (color == ORANGE)
  {
    screenMem[y]   = screenMem[y]   | mask;  // green
    screenMem[y+8] = screenMem[y+8] | mask;  // red
  }
  else if (color == BLACK)
  {
    mask = ~mask;
    screenMem[y]   = screenMem[y]   & mask;
    screenMem[y+8] = screenMem[y+8] & mask;
  }
  else
  {
    // imposible color for an RG display;
  }
}


// returns the state (on or off) of the pixel;
boolean getPixel(int color, int x, int y)
{
  byte mask = 1<<(7-x);
  byte stateOfRed   = screenMem[y+8] & mask;;
  byte stateOfGreen = screenMem[y]   & mask;

  if (color == RED)
  {
    // looking for pure red, so need to check the green as well, just in case it's orange;
    return (stateOfRed != 0 && stateOfGreen == 0);
  }

  if (color == GREEN)
  {
    // looking for pure green, so need to check the red as well, just in case it's orange;
    return (stateOfGreen != 0 && stateOfRed == 0);
  }

  if (color == ORANGE)
  {
    return (stateOfGreen != 0 && stateOfRed != 0);
  }

  return false;
}



