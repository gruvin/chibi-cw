#include <stdio.h>
#include <stdlib.h>

#include "font16x16_0-9.xbm"

#define byte unsigned char

int main(void)
{
  // char buf[32];
  byte buf;
  byte charByte;
  int xbmIndex;
  int count;
  FILE *of;
  
  // open output file
  if ((of = fopen("font16x16_0-9.lbm", "w")) == 0)
  {
    printf("ERROR: Could not open output file\n");
    exit (1);
  }

  // Each oLED character is a 16x16 bit buffer as 2 x 8bit high rows
  // left to right, with LSb at top of each column

  count = 0;

  for (xbmIndex = 0; xbmIndex < 20; xbmIndex += 2) // each char is two bytes wide by 16 bits high
  {
    // Convert one character ...

    for (charByte=0; charByte < 32; charByte++)
    {  
      byte b=1<<(charByte % 8);

      byte fi = xbmIndex;
      if (charByte>23) fi += 161;
      else if (charByte>15) fi +=160;
      else if (charByte>7) fi += 1;

      buf = 
        (((font16x1609_bits[fi] & b) == b) << 0)
        | (((font16x1609_bits[fi+20] & b) == b) << 1)
        | (((font16x1609_bits[fi+40] & b) == b) << 2)
        | (((font16x1609_bits[fi+60] & b) == b) << 3)
        | (((font16x1609_bits[fi+80] & b) == b) << 4)
        | (((font16x1609_bits[fi+100] & b) == b) << 5)
        | (((font16x1609_bits[fi+120] & b) == b) << 6)
        | (((font16x1609_bits[fi+140] & b) == b) << 7);

      fprintf(of, "0x%02x", buf);
      if (count < 319) fprintf(of, ",");
      if ((count % 16) == 15) fprintf(of, "\n");
      count++;
      printf("%6d %d\n", xbmIndex, count);
    }
    // ... one character converted.
    // loop to next char
  }
  fclose(of);
  printf("Font conversion compelted.\n");

}


