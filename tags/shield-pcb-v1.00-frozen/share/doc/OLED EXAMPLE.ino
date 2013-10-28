// Wire Master Writer
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Writes data to an I2C/TWI slave device
// Refer to the "Wire Slave Receiver" example for use with this

// Created 29 March 2006

// This example code is in the public domain.

#include <Wire.h>

#define OLED_ADDRESS    0x51

#define WRITE_CMD       0x01
#define WRITE_DAT       0x02
#define RESET           0x03

#define DISP_8X16STR    0x10
#define DISP_AREA       0x11
#define FILL_AREA       0x12
#define SET_SCROHOR     0x13
#define SET_SCROVER     0x14
#define SET_SCROVERHOR  0x15

#define SET_ADDRESS     0x21

#define PAGE0           0x00
#define PAGE1           0x01
#define PAGE2           0x02
#define PAGE3           0x03
#define PAGE4           0x04
#define PAGE5           0x05
#define PAGE6           0x06
#define PAGE7           0x07

#define SCROLL_UP       0x01
#define SCROLL_DOWN     0x00
#define SCROLL_RIGHT    0x26
#define SCROLL_LEFT     0x27
#define SCROLL_VR       0x29
#define SCROLL_VL       0x2A

#define FRAMS_2         0x07
#define FRAMS_3         0x04
#define FRAMS_4         0x05
#define FRAMS_5         0x00
#define FRAMS_25        0x06
#define FRAMS_64        0x01
#define FRAMS_128       0x02
#define FRAMS_256       0x03

void Display8x16Str(uint8_t page, uint8_t column, const char *str)
{
    Wire.beginTransmission(OLED_ADDRESS);
    Wire.write(DISP_8X16STR);
    Wire.write(page);
    Wire.write(column);
    
    while(*str != '\0')
    {
         Wire.write(*str++);
    }
    Wire.endTransmission();
}
void FillArea(uint8_t spage, uint8_t epage,uint8_t scolumn, uint8_t ecolumn,uint8_t filldata)
{
    Wire.beginTransmission(OLED_ADDRESS);
    Wire.write(FILL_AREA);
    Wire.write(spage);
    Wire.write(epage);
    Wire.write(scolumn);
    Wire.write(ecolumn);
    Wire.write(filldata);
    Wire.endTransmission();
}
void ScrollingHorizontal(uint8_t lr, uint8_t spage, uint8_t epage,uint8_t frames)
{
    Wire.beginTransmission(OLED_ADDRESS);
    Wire.write(SET_SCROHOR);
    Wire.write(lr);
    Wire.write(spage);
    Wire.write(epage);
    Wire.write(frames);
    Wire.endTransmission();
}
void ScrollingVertical(uint8_t scrollupdown, uint8_t rowsfixed, uint8_t rowsscroll, uint8_t scrollstep, uint8_t stepdelay)
{
    Wire.beginTransmission(OLED_ADDRESS);
    Wire.write(SET_SCROVER);
    Wire.write(scrollupdown);
    Wire.write(rowsfixed);
    Wire.write(rowsscroll);
    Wire.write(scrollstep);
    Wire.write(stepdelay);
    Wire.endTransmission();
}
void ScrollingVertivalHorizontal(uint8_t fixedarea, uint8_t scrollarea, uint8_t vlr, uint8_t spage, uint8_t epage, uint8_t frames, uint8_t offset)
{
    Wire.beginTransmission(OLED_ADDRESS);
    Wire.write(SET_SCROVERHOR);
    Wire.write(fixedarea);
    Wire.write(scrollarea);
    Wire.write(vlr);
    Wire.write(spage);
    Wire.write(epage);
    Wire.write(frames);
    Wire.write(offset);
    Wire.endTransmission();
}
void DeactivateScroll()
{
    Wire.beginTransmission(OLED_ADDRESS);
    Wire.write(WRITE_CMD);
    Wire.write(0x2E);
    Wire.endTransmission();
}
void WriteCommand(const char *cmd)
{
    Wire.beginTransmission(OLED_ADDRESS);
    Wire.write(WRITE_CMD);    
    while(*cmd != '\0')
    {
         Wire.write(*cmd++);
    }
    Wire.endTransmission();
}
void WriteData()
{
    unsigned int i;
    Wire.beginTransmission(OLED_ADDRESS);
    Wire.write(WRITE_DAT);    
    for (i=0; i<20; i++)
    {
        // Wire.send(0xF0);  
    }
    for (i=0; i<20; i++)
    {
        // Wire.send(0x0F);  
    }
    for (i=0; i<32; i++)
    {
         Wire.write(0xFF);  
    }
    Wire.endTransmission();
}
void SetAddress(uint8_t page, uint8_t column)
{
    Wire.beginTransmission(OLED_ADDRESS);
    Wire.write(SET_ADDRESS);    
    Wire.write(page); 
    Wire.write(column); 
    Wire.endTransmission();
}
void Reset()
{
    Wire.beginTransmission(OLED_ADDRESS);
    Wire.write(RESET);    
    Wire.endTransmission();
}
void setup()
{
  Wire.begin(); 
  Serial.begin(9600);
  pinMode(13, OUTPUT); 
  pinMode(12, INPUT); 
  Reset();
}
void loop()
{
  int i;
  delay(1000);
  digitalWrite(13,HIGH);
  for(i=0; i<8; i++)
  {
      SetAddress(i,0);
      WriteData();
      WriteData();
      WriteData();
      WriteData();
  }
  delay(1000);
  
  digitalWrite(13,LOW);
  FillArea(0,7,0,128,0);
  delay(1000);
 
  digitalWrite(13,HIGH);
  Display8x16Str(0,0, "Hello World!");
  Display8x16Str(2,0, "Hello OLED!");
  Display8x16Str(4,0, "Wellcom to use");
  Display8x16Str(6,0, "Gem.Arduino");
  digitalWrite(13,LOW);
  ScrollingVertical(SCROLL_UP,0,32,1,250);
   delay(3000);
  ScrollingHorizontal(0x27, 4, 5, 0x07);
  delay(2000);
   DeactivateScroll();
  ScrollingHorizontal(0x26, 6, 7, 0x07);
  delay(2000);
  DeactivateScroll();
  FillArea(0,7,0,127,0);
  delay(1000);
  Reset();
  delay(2000);
 //*/
}


