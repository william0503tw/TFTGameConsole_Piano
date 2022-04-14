//For Screen
#define TFT_CS 10
#define TFT_DC 9    
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_RST 8
#define TFT_MISO 12
#define TS_CS 7
#define ROTATION 3

//For Pcb
#define joy_x A7
#define joy_y A6
#define joy_sw A3
#define start A2
#define select A1
#define LED 5
#define buzzer 6

//Screen's Coordinate
/*(0,0)---------[+x]
 *  |             |
 *  |             |
 *  |             |
 *  |             |
 *  |             |
 * [+y]------------
 */

//Serial Buadrate
#define BUADRATE 9600 

//Libraries
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "XPT2046_Touchscreen.h"
#include "Wire.h"
