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

void setupLED();
void ledON(int x, int y);
void ledOFF(int x, int y);
