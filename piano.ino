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

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TS_CS);

float xCalC = 0.0, yCalC = 0.0;
float xCalM = 0.0, yCalM = 0.0;
const int8_t BlackNoteHeight = 120 ;
const int8_t BlackNoteWidth = 30 ;
const uint16_t WhiteNoteHeight = 240 ;
const int8_t WhiteNoteWidth = 40 ;
float BaseFreq = 261.6 ; //C4

class ScreenPoint {
    public:
        int16_t x;
        int16_t y;
        ScreenPoint(int16_t xIn, int16_t yIn){
            x = xIn;
            y = yIn;
        }
};
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

void drawPiano(){
    tft.fillScreen(ILI9341_WHITE);
    int count = 0 ;
    bool t = 0 ;
    String note[9] = {"C4","D4","E4","F4","G4","A4","B4","C5","\0"};
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK);
    for(int i = 0 ; i <= tft.width() ; i += tft.width() / 8){
        tft.drawFastVLine(i,0,tft.height(),ILI9341_BLACK);
        tft.setCursor(i + 10,210);
        tft.print(note[i/40]);
    }
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

void playNote(ScreenPoint p){
    bool isBlackPlayed = 0 ; 
    // Black
    bool t = 0 ;
    int count = 0 ; //For avoid white space
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
            tone(buzzer,getPianoFreq('B',i / 40),100);
            isBlackPlayed = 1 ; 
        }
        count++ ;
    }
    //White
    if(!isBlackPlayed){
        for(int i = 0 ; i <= tft.width() ; i += tft.width() / 8){
            if(p.x > i && p.x < i + WhiteNoteWidth && p.y > 0 && p.y < WhiteNoteHeight){
                tone(buzzer,getPianoFreq('W',i / 40),100);
            }
        }
    }
}

float getPianoFreq(char n,int p){
    if(n == 'B'){
        switch(p){
            case 0: p = 41 ;    break;
            case 1: p = 43 ;    break;
            case 3: p = 46 ;    break;
            case 4: p = 48 ;    break;
            case 5: p = 50 ;    break;
            case 7: p = 53 ;    break;    
        }
        return pow(pow(2,0.0833333),p-49) * 440.0 ;
    }else if(n == 'W'){
        switch(p){
            case 0: p = 40 ;    break;
            case 1: p = 42 ;    break;
            case 2: p = 44 ;    break;
            case 3: p = 45 ;    break;
            case 4: p = 47 ;    break;
            case 5: p = 49 ;    break;
            case 6: p = 51 ;    break;
            case 7: p = 52 ;    break;           
        }
        return pow(pow(2,0.0833333),p-49) * 440.0 ;
    }
}

void setup()
{
    Serial.begin(BUADRATE);

    pinMode(TS_CS, OUTPUT);
    digitalWrite(TS_CS, HIGH); 
    pinMode(TFT_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH); 

    pinMode(buzzer, OUTPUT);

    tft.begin(); 
    tft.setRotation(ROTATION); 
    tft.fillScreen(ILI9341_BLACK);
    ts.begin(); 
    ts.setRotation(ROTATION); 
    calibrateTouchScreen();
    delay(500);
    drawPiano();
}

void loop()
{
    while(!ts.touched());
    ScreenPoint sp = getScreenCoords(ts.getPoint());
    playNote(sp);
}

