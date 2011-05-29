/** 
 * Ladarevo Opus.1 LED matrix driver.
 *
 * API:
 *   setupLED();
 *   ledON(int x, int y);  0<=x<=7, 0<=y<=7;
 *   ledOFF(int x, int y);  0<=x<=7, 0<=y<=7;
 */

void setupLED();
void ledON(int x, int y);
void ledOFF(int x, int y);
