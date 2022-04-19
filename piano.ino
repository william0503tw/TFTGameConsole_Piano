/**
 * @file piano.ino
 * @author William Chiu 邱俊瑋
 * @brief 
 *  An digital piano on custom build arduino game console.
 *  In ver.1, simple piano function is completed, note frequency
 *  is ranged from C4 ~ C5.
 * @version 1.0
 * @date 2022-04-15
 */

/**
 * TODO: 
 * 1. Add more note frequency range
 * 2. Storge calibration in EEPROM
 * 3. Recording Songs in EEPROM
 * 4. Menu
 */

#include "config.h"
#include <math.h>

// Declare object tft(display) and ts(touch screen)
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TS_CS);

// parameters for screen calibration
float xCalC = 0.0, yCalC = 0.0;
float xCalM = 0.0, yCalM = 0.0;

// Piano has White note and black note 
// And below is the dimensions of them (pixel)
const int8_t BlackNoteHeight = 120 ;
const int8_t BlackNoteWidth = 30 ;
const uint16_t WhiteNoteHeight = 240 ;
const int8_t WhiteNoteWidth = 40 ;

// Declare "BaseFreq" for calculate output frequency
float BaseFreq = 261.6 ; //C4

// Class "ScreenPoint"
// Ex. (x,y) = (201,160)
// Max(x,y) = (320,160)
class ScreenPoint {
    public:
        int16_t x;
        int16_t y;

        //Constructor take (x,y) as parameters
        ScreenPoint(int16_t xIn, int16_t yIn){
            x = xIn;
            y = yIn;
        }
};

// Function "getScreenCoords": 
// return type: ScreenPoint after calibration
// 0 <= x <= 320 ; 0 <= y <= 160
// parameters: TS_Point(A type define in "XPT2046_Touchscreen.h")
ScreenPoint getScreenCoords(TS_Point p) {
    int16_t xCoord = round((p.x * xCalM) + xCalC);
    int16_t yCoord = round((p.y * yCalM) + yCalC);

    if(xCoord < 0)
        xCoord = 0;
    if(xCoord >= tft.width())
        xCoord = tft.width() - 1;
    if(yCoord < 0)
        yCoord = 0;
    if(yCoord >= tft.height())
        yCoord = tft.height() - 1;

    return(ScreenPoint(xCoord, yCoord));
}

// Function "calibrateTouchScreen"
// return type: void
// A procedure called in "setup" before using touchscreen
void calibrateTouchScreen(){
    TS_Point p;
    int16_t x1,y1,x2,y2;

    tft.fillScreen(ILI9341_BLACK);

    while(ts.touched());

    tft.drawFastHLine(10,20,20,ILI9341_WHITE);
    tft.drawFastVLine(20,10,20,ILI9341_WHITE);
    while(!ts.touched());

    p = ts.getPoint();
    x1 = p.x;
    y1 = p.y;
    tft.drawFastHLine(10,20,20,ILI9341_BLACK);
    tft.drawFastVLine(20,10,20,ILI9341_BLACK);
    delay(500);

    while(ts.touched());
    tft.drawFastHLine(tft.width() - 30,tft.height() - 20,20,ILI9341_WHITE);
    tft.drawFastVLine(tft.width() - 20,tft.height() - 30,20,ILI9341_WHITE);
    while(!ts.touched());

    p = ts.getPoint();
    x2 = p.x;
    y2 = p.y;
    tft.drawFastHLine(tft.width() - 30,tft.height() - 20,20,ILI9341_BLACK);
    tft.drawFastVLine(tft.width() - 20,tft.height() - 30,20,ILI9341_BLACK);

    int16_t xDist = tft.width() - 40;
    int16_t yDist = tft.height() - 40;


    xCalM = (float)xDist / (float)(x2 - x1);
    xCalC = 20.0 - ((float)x1 * xCalM);

    yCalM = (float)yDist / (float)(y2 - y1);
    yCalC = 20.0 - ((float)y1 * yCalM);
}

// Function to draw the appearance of the piano
void drawPiano(){
    // Initialize the display with white background
    tft.fillScreen(ILI9341_WHITE);

    //variable for drawing black notes
    int count = 0 ; //how many black note did it draw ? equal 0 every time 2 or 3 count 
    bool t = 0 ;    //jump between "draw 2" and "draw 3" //toggle

    //String array storge note's name
    String note[9] = {"C","D","E","F","G","A","B","C"};

    //Basic setup for printing texts
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK);

    //Print Vertical black lines, by setting i = 0, and add up 40px every iterations.
    for(int i = 0 ; i <= tft.width() ; i += tft.width() / 8){
        tft.drawFastVLine(i,0,tft.height(),ILI9341_BLACK);
        tft.setCursor(i + 15,210);
        //print note's text
        tft.print(note[i/40]);
    }
    //After printing vertical black lines, the white notes formed.

    //print black notes
    for(int i = 0 ; i <= tft.width() ; i += tft.width() / 8){
        if(count == 2 && !t){
            count = 0 ;
            t = 1 ;
            continue;
        }else if(count == 3){
            count = 0;
            t = 0 ;
            continue;
        }
        tft.fillRect( i + 25 ,0 , BlackNoteWidth , BlackNoteHeight ,ILI9341_BLACK);
        count++;
    }
}

// Function "playNote"
// return type: void
// parameters: ScreenPoint
// Description: the function is used to deicde the area i touched abd played the corresponding frequency
void playNote(ScreenPoint p,int8_t shift){
    //Avoiding playing two notes at the same time when touching where black and white notes overlapped.
    //So if the black notes are touched, "isBlackPlayed" would set to true, otherwise it remain false.
    bool isBlackPlayed = 0 ; 

    // Black Notes
    bool t = 0 ;
    int count = 0 ; //For avoiding white notes
    for(int i = 0 ; i <= tft.width() ; i += tft.width() / 8){
        if(count == 2 && !t){
                count = 0 ;
                t = 1 ;
                continue;
            }else if(count == 3){
                count = 0;
                t = 0 ;
                continue;
        }
        if(p.x > i + 25 && p.x < i + 25 + BlackNoteWidth && p.y > 0 && p.y < BlackNoteHeight){
            tone(buzzer,getPianoFreq('B',i / 40 , shift),100);
            isBlackPlayed = 1 ; 
        }
        count++ ;
    }
    //White Notes
    //Enter the loop if the black notes aren't touched
    if(!isBlackPlayed){
        for(int i = 0 ; i <= tft.width() ; i += tft.width() / 8){
            if(p.x > i && p.x < i + WhiteNoteWidth && p.y > 0 && p.y < WhiteNoteHeight){
                tone(buzzer,getPianoFreq('W',i / 40 , shift),100);
            }
        }
    }
}

//Calculte piano frequencies
//Wikipedia: https://zh.wikipedia.org/wiki/%E9%8B%BC%E7%90%B4%E9%8D%B5%E9%A0%BB%E7%8E%87
float getPianoFreq(char n,int p,int8_t shift){
    if(n == 'B'){
        switch(p){
            case 0: p = 41 + shift;    break;
            case 1: p = 43 + shift;    break;
            case 3: p = 46 + shift;    break;
            case 4: p = 48 + shift;    break;
            case 5: p = 50 + shift;    break;
            case 7: p = 53 + shift;    break;    
        }
        return pow(pow(2,0.0833333),p-49) * 440.0 ;
    }else if(n == 'W'){
        switch(p){
            case 0: p = 40 + shift;    break;
            case 1: p = 42 + shift;    break;
            case 2: p = 44 + shift;    break;
            case 3: p = 45 + shift;    break;
            case 4: p = 47 + shift;    break;
            case 5: p = 49 + shift;    break;
            case 6: p = 51 + shift;    break;
            case 7: p = 52 + shift;    break;           
        }
        return pow(pow(2,0.0833333),p-49) * 440.0 ;
    }
}

void printLevel(int shift){
    tft.fillRect(2,2,22,22,ILI9341_WHITE);
    tft.setCursor(4,4);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(2);
    tft.print(shift);
}


int8_t shift = 0 ;
int level = 4 ;

void setup()
{
    Serial.begin(BUADRATE);

    //Avoid confronation between touchscreen and tft display
    pinMode(TS_CS, OUTPUT);
    digitalWrite(TS_CS, HIGH); 
    pinMode(TFT_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH); 

    //pinMode
    pinMode(buzzer, OUTPUT);
    pinMode(UP,INPUT_PULLUP);
    pinMode(DOWN,INPUT_PULLUP);
    pinMode(RIGHT,INPUT_PULLUP);
    pinMode(LEFT,INPUT_PULLUP);

    //tft display and touch screen initialization
    tft.begin(); 
    tft.setRotation(ROTATION); 
    tft.fillScreen(ILI9341_BLACK);
    ts.begin(); 
    ts.setRotation(ROTATION); 

    //touchscreen's calibration
    calibrateTouchScreen();

    //500 ms delay for safety (optional)
    delay(500);

    //draw the piano
    drawPiano();

    printLevel(level);
}

void loop()
{
    //If I did not touch the screen, the program would stuck in while loop.
    while(!ts.touched()){
        if(digitalRead(RIGHT) == 0){
            shift += 12 ;
            level += 1 ;
            printLevel(level);
        }else if(digitalRead(LEFT) == 0){
            shift -= 12 ;
            level -= 1 ;
            printLevel(level);
        };
        delay(100);
    };
    //If I touch the screen, it will storge coordinate in variable "sp"
    //and play the corresponding frequency in function "playNote".
    ScreenPoint sp = getScreenCoords(ts.getPoint());
    playNote(sp,shift);
}

