/** 
 * WiseTimer
 * 
 */

#include <WProgram.h>
#include <avr/sleep.h>
#include "sym_KT.h"


// used to enable/disable Serial.print, time-consuming operation;
// to minimize the code size, all references to Serial should be commented out;
#define _DEBUG_     true


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


// button debouncing adapted from http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210559123/7;
#define BOUNCE_TIME_BUTTON  600   // bounce time in ms for the menu button;
#define BOUNCE_TIME_IR      500   // bounce time in ms for the IR receiver;


// last time a command (IR or button) was received; used for debouncing;
volatile unsigned long menuTime = 0;

// indicates that one of the menu options (0..MAX_OPTION) is currently displayed;
boolean isMenuOptionOnDisplay = false;

// indicates that display needs to be refreshed;
boolean mustRefreshDisplay = false;

boolean isCounting = false;

int initialTime = 0;
int currentCount = 0;


// executed as a result of the menu button being pressed;
// determines the menu to be displayed;
void processMenuButton()
{
  // debouncing;
  if (abs(millis() - menuTime) < BOUNCE_TIME_BUTTON)
    return;

  menuTime = millis();

  menuOption++;
  if (menuOption > MAX_OPTION) menuOption = 0;
  mustRefreshDisplay = true;

  isCounting = false;
}


void setup()
{
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

  resetDisplay();

#ifdef _DEBUG_
    Serial.begin(9600);
#endif
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
  // check menu button (connects analog pin 3 to ground);
  int val = analogRead(MENU_BUTTON_PIN);
  if (val < 5)
  {
    // menu button was pressed;
    processMenuButton();
  }
  
  if (isCounting)
  {
    Serial.print("isCounting=");
    Serial.println(isCounting, DEC);
    countDown();
    return;
  }

  // display the menu option for 5 seconds after menu button was pressed;
  if ((menuTime > 0) && (millis() - menuTime < 5000))
  {
      isMenuOptionOnDisplay = true;

      if (mustRefreshDisplay)
      {
        resetDisplay();
        setScreenMem(GREEN, sprites[menuOption]);
        mustRefreshDisplay = false;
      }
  }
  else
  {
    // clear screen after displaying menu option for 5 seconds;
    if (isMenuOptionOnDisplay)
    {
      resetDisplayByMovingDot();
      isMenuOptionOnDisplay = false;

      bigOptionSwitch();
    }
  }
}



//------------------------------------------------------------------------
// put the arduino to sleep to save power;
// function copied from http://www.arduino.cc/playground/Learning/arduinoSleepCode;
//------------------------------------------------------------------------
void sleepNow()
{
    /* Now is the time to set the sleep mode. In the Atmega8 datasheet
     * http://www.atmel.com/dyn/resources/prod_documents/doc2486.pdf on page 35
     * there is a list of sleep modes which explains which clocks and 
     * wake up sources are available in which sleep modus.
     *
     * In the avr/sleep.h file, the call names of these sleep modus are to be found:
     *
     * The 5 different modes are:
     *     SLEEP_MODE_IDLE         -the least power savings 
     *     SLEEP_MODE_ADC
     *     SLEEP_MODE_PWR_SAVE
     *     SLEEP_MODE_STANDBY
     *     SLEEP_MODE_PWR_DOWN     -the most power savings
     *
     * For now, we want as much power savings as possible, so we 
     * choose the according 
     * sleep modus: SLEEP_MODE_PWR_DOWN
     * 
     */  
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here

    sleep_enable();          // enables the sleep bit in the mcucr register so sleep is possible; just a safety pin;

    detachInterrupt(0);      // disables the previous routine (IR receiver ISR);
    attachInterrupt(0, wakeUpNow, LOW); // use interrupt 0 (pin 2) and run function wakeUpNow when pin 2 gets LOW (IR command);

#ifdef _DEBUG_
    Serial.println("going to sleep...");
#endif

    sleep_mode();        // the device is actually put to sleep! THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP.

#ifdef _DEBUG_
    Serial.println("I am back!");
#endif

    sleep_disable();         // first thing after waking from sleep: disable sleep...

    detachInterrupt(0);      // disables interrupt 0 on pin 2 so the wakeUpNow code will not be executed during normal running time;
//    attachInterrupt(0, irReceiverISR, LOW);
}


void wakeUpNow()        // here the interrupt is handled after wakeup
{
  // execute code here after wake-up before returning to the loop() function
  // timers and code using timers (serial.print and more...) will not work here.
  // we don't really need to execute any special functions here, since we just want the thing to wake up;
}


void bigOptionSwitch()
{
    switch (menuOption)
    {
      case OPTION_SLEEP:
        // used for power saving; it puts the processor in "sleep mode power down";
        resetDisplay();
        delay(100);     // this delay is needed;
        sleepNow();
        break;

      default:
        // count down from the specified time (in seconds);
        initialTime = 60 + menuOption * 30;
        currentCount = 120;

        // total number of pixels to update is 120 which is made of
        // 60 transitions from green to yellow, 60 transitions from yellow to red;
        // screen is updated depending on the initial timer setting;
        // e.g. for 60 seconds, update screen every half second;
        waitBetweenPixels = 1000.0 * initialTime / 120;

  Serial.print("initial time=");
  Serial.println(initialTime);
  Serial.print("waitBetweenPixels=");
  Serial.println(waitBetweenPixels);

        // initial screen is all green;
        setScreenMem(GREEN, sprites[ALL_LIT]);

        isCounting = true;
        break;
    }
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
  Serial.print("Transition number ");
  Serial.print(trNum);
  
  // trNum is between 120 and 0;
  // for trNum between 120 and 60, pixel changes color from green to yellow;
  // for trNum between  59 and  0, pixel color changes from yellow to red;

  // generate pixel's location;
  for (int i=0; i<20; i++)
  {
    // max 20 tries to find a pixel to change;
    // if not found randomly, then search for one;
    int randomAddr = random(64);  // return 0..63
  Serial.print("  randomAddr=");
  Serial.print(randomAddr);

    int x0 = randomAddr % 8;
    int y0 = randomAddr / 8;
    if (trNum >= 60)
    {
      // looking for green pixels;
      if (getPixel(GREEN, x0, y0))
      {
        // change it to orange;
        setPixel(BLACK,  x0, y0);
        setPixel(ORANGE, x0, y0);
  Serial.println("  found randomly green pixel");
  
        return;
      }
    }
    else
    {
      // looking for orange pixels;
      if (getPixel(ORANGE, x0, y0))
      {
        // change it to red;
        setPixel(BLACK, x0, y0);
        setPixel(RED,   x0, y0);
  Serial.println("  found randomly orange pixel");
        return;
      }
    }
  }
  
  // being here, all random attempts to find a pixel to change failed;
  // will find it systematically, by scanning every row;
  for (int x=0; x<8; x++)
  {
    for (int y=0; y<8; y++)
    {
      if (trNum >= 60)
      {
        // looking for green pixels;
        if (getPixel(GREEN, x, y))
        {
  Serial.println("  found systematically green pixel");
          // change it to orange;
          setPixel(BLACK,  x, y);
          setPixel(ORANGE, x, y);
          return;
        }
      }
      else
      {
        // looking for orange pixels;
        if (getPixel(ORANGE, x, y))
        {
  Serial.println("  found systematically orange pixel");
          // change it to red;
          setPixel(BLACK, x, y);
          setPixel(RED,   x, y);
          return;
        }
      }
    }
  }
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



