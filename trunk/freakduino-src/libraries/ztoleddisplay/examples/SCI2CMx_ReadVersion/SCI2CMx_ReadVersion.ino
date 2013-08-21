#include <ZtLib.h>
#include <Wire.h>
#define OLED_ADDRESS    0x27

void setup()
{
  uint8_t versbuff[16];
  uint8_t temp;
  Serial.begin(9600);
  ZT.I2cInit();
  ZT.ScI2cMxReset(OLED_ADDRESS);
  delay(5);//复位，重新初始化，清理缓存，需要稍稍等待
  ZT.ScI2cMxDisplay8x16Str(OLED_ADDRESS,0,0,"Hobbyist Ltd");
  temp = ZT.ScI2cMxReadVersion(OLED_ADDRESS,versbuff);
  ZT.ScI2cMxDisplay8x16Str(OLED_ADDRESS,3,0,"OLED disp:");
  ZT.ScI2cMxDisplay8x16Str(OLED_ADDRESS,5,0,(char *)versbuff);
  Serial.print("Version:\n");
  for (int i=0; i<temp; i++)
  {
      Serial.print((char)versbuff[i]);
  }
   Serial.print('\n');
  
}
void loop()
{
}
