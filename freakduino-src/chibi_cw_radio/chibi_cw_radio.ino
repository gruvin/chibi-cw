/************************************************************/
/*
  TODO: Code description.
*/
/************************************************************/
#include <chibi.h>
#include <ZtLib.h>
#include <Wire.h>

#define RX_AUDIO_PIN   3 // OC2B
#define TX_AUDIO_PIN   5 // OC0B
#define ROT_ENC_A      6
#define ROT_ENC_B      7
#define CW_KEY_PIN     8
#define LED_PIN        9

#define OLED_ADDRESS   0x27
#define OLED_TXSPOT_PAGE 2
#define OLED_RXSPOT_PAGE 3

#define RX_KEY_TIMEOUT 2000
#define TX_KEY_TIMEOUT 1500

#define CHIBI_CW_IDENT 0xCC

#define oledStr(p, c, s) ZT.ScI2cMxDisplay8x16Str(OLED_ADDRESS,p,c,s);

#define oLEDdelay(t) for(unsigned int volatile dxi=0; dxi < t; dxi++)

// TODO: Make changeable in a menu
const unsigned int audioCenterFreq = 600;  // Hz


/** 
 * FUNCTIONS
 */

void setRxToneFreq(unsigned int hertz)
{
  // OCR2A is TOP. OCR2B is the toggle point, which should be half of OCR2A for 50% duty cycle
  // OCR2A = 250; --> 500Hz
  // OCR2A = 96; --> 1300Hz
  unsigned int top;
  unsigned char middle;
  
  top = 8000000UL / 128 / hertz;
// Serial.print("RXTOP="); Serial.println(top, DEC);
  
  middle = (top / 2);
  OCR2A = top;
  OCR2B = middle;
  TCCR2B = (1<<WGM22) | (1<<CS22);   /* CLK divided by 64 (we're running at 8MHz) */
  TCCR2A = (1<<COM2B1) | (1<<WGM20); // tone output on (non-inverting)
  
  Serial.print("SET RX: "); Serial.println(audioCenterFreq);
    
  // oLED: clear the RX spot indicator (whole row)
  ZT.ScI2cMxFillArea(OLED_ADDRESS, OLED_RXSPOT_PAGE, OLED_RXSPOT_PAGE, 0, 159, 0);
  oLEDdelay(320);
  // oLED: show RX spot indicator
  int offset = (hertz - audioCenterFreq)/8;
  ZT.ScI2cMxFillArea(OLED_ADDRESS, OLED_RXSPOT_PAGE, OLED_RXSPOT_PAGE, 63+offset, 65+offset, 0xff);
  oLEDdelay(320);
  
}

void setTxToneFreq(unsigned int hertz)
{
  // OCR0A is TOP. OCR0B is the toggle point, which should be half of OCR0A for 50% duty cycle
  // OCR0A = 250; --> 500Hz
  // OCR0A = 96; --> 1300Hz
  unsigned int top;
  unsigned char middle;
 
  top = 8000000UL / 128 / hertz;
// Serial.print("TXTOP="); Serial.println(top, DEC);
  
  middle = (top / 2);
  OCR0A = top;
  OCR0B = middle;
  TCCR0B = (1<<WGM02) | (1<<CS01) | (1<<CS00); /* CLK divided by 64 (we're running at 8MHz) */
  TCCR0A = (1<<COM0B1) | (1<<WGM00);           // tone output on (non-inverting)

  // oLED: set transmitting status TX spot indicator
  ZT.ScI2cMxFillArea(OLED_ADDRESS, OLED_TXSPOT_PAGE, OLED_TXSPOT_PAGE, 60, 69, 0xff);
  oLEDdelay(320);

}

void setNoRxTone()
{
  TCCR2A = (1<<WGM20);
  
  Serial.println("NO RX");
  
  // clear the RX spot indicator (whole row)
  ZT.ScI2cMxFillArea(OLED_ADDRESS, OLED_RXSPOT_PAGE, OLED_RXSPOT_PAGE, 0, 159, 0);
  oLEDdelay(320);
}

void setNoTxTone()
{
  TCCR0A = (1<<WGM00);

  // oLED: set receiving status TX spot indicator
  ZT.ScI2cMxFillArea(OLED_ADDRESS, OLED_TXSPOT_PAGE, OLED_TXSPOT_PAGE, 0, 159, 0);
  oLEDdelay(500);
  ZT.ScI2cMxFillArea(OLED_ADDRESS, OLED_TXSPOT_PAGE, OLED_TXSPOT_PAGE, 63, 65, 0xff);
  oLEDdelay(320);
}

// 16x16 OLED format font -- characters '0' through '9', 32 bytes each.
prog_uchar font16x16[] PROGMEM = {
  0x00,0xf8,0xf8,0xfc,0x0e,0x0e,0x06,0x06,0x06,0x0e,0x0e,0xfc,0xf8,0xf8,0x00,0x00,
  0x00,0x1f,0x1f,0x3f,0x70,0x70,0x60,0x60,0x60,0x70,0x70,0x3f,0x1f,0x1f,0x00,0x00,
  0x00,0x18,0x18,0x18,0x1c,0x1c,0xfe,0xfe,0xfe,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x60,0x60,0x60,0x60,0x60,0x7f,0x7f,0x7f,0x60,0x60,0x60,0x60,0x60,0x00,0x00,
  0x00,0x18,0x18,0x1c,0x0e,0x0e,0x06,0x06,0x86,0xce,0xce,0xfc,0x78,0x78,0x00,0x00,
  0x00,0x78,0x78,0x7c,0x6e,0x6e,0x67,0x67,0x63,0x61,0x61,0x60,0x60,0x60,0x00,0x00,
  0x00,0x06,0x06,0x06,0x86,0x86,0x86,0x86,0x86,0xce,0xce,0xfc,0x78,0x78,0x00,0x00,
  0x00,0x60,0x60,0x60,0x61,0x61,0x61,0x61,0x61,0x73,0x73,0x3f,0x1e,0x1e,0x00,0x00,
  0x00,0x80,0x80,0xc0,0xe0,0xe0,0x70,0x70,0x38,0x1c,0x1c,0xfe,0xfe,0xfe,0x00,0x00,
  0x00,0x07,0x07,0x07,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x7f,0x7f,0x7f,0x00,0x00,
  0x00,0x7e,0x7e,0x7e,0x66,0x66,0x66,0x66,0x66,0xe6,0xe6,0xc6,0x86,0x86,0x00,0x00,
  0x00,0x18,0x18,0x38,0x70,0x70,0x60,0x60,0x60,0x70,0x70,0x3f,0x1f,0x1f,0x00,0x00,
  0x00,0xf8,0xf8,0xfc,0x8e,0x8e,0x86,0x86,0x86,0x8e,0x8e,0x1c,0x18,0x18,0x00,0x00,
  0x00,0x1f,0x1f,0x3f,0x71,0x71,0x61,0x61,0x61,0x73,0x73,0x3f,0x1e,0x1e,0x00,0x00,
  0x00,0x06,0x06,0x06,0x86,0x86,0xc6,0xc6,0xe6,0x76,0x76,0x3e,0x1e,0x1e,0x00,0x00,
  0x00,0x00,0x00,0x00,0x7f,0x7f,0x7f,0x7f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x78,0x78,0xfc,0xce,0xce,0x86,0x86,0x86,0xce,0xce,0xfc,0x78,0x78,0x00,0x00,
  0x00,0x1e,0x1e,0x3f,0x73,0x73,0x61,0x61,0x61,0x73,0x73,0x3f,0x1e,0x1e,0x00,0x00,
  0x00,0x78,0x78,0xfc,0xce,0xce,0x86,0x86,0x86,0xce,0xce,0xfc,0xf8,0xf8,0x00,0x00,
  0x00,0x00,0x00,0x00,0x61,0x61,0x71,0x71,0x39,0x1d,0x1d,0x0f,0x07,0x07,0x00,0x00
};

void oled16x16digit(uint8_t page, uint8_t col, char c)
{
  if (c == '.')
    ZT.ScI2cMxFillArea(OLED_ADDRESS, 1, 1, (3*16)+5, (3*16)+9, 0x0f);

  if ((c < '0') || (c > '9')) return; // ignore non-digits
  ZT.ScI2cMxDisplayDot16x16_prog(OLED_ADDRESS, page, col, (prog_char *)&(font16x16[(c-'0')*32]));
}

void oled16x16string(uint8_t page, uint8_t col, char *str)
{
  for (byte i=0; str[i]; i++)
    oled16x16digit(page, col+(i*16), str[i]);
}

/**************************************************************************/
// Initialize
/**************************************************************************/

struct ccBuffer {
  byte id;            // Chibi-CW identifier 0xCC, to help filter out foreign traffic
  word freq;          // 16-bit virtualFreq
  byte state;         // 8-bit state (currently, 255= key down, 0 = key up)
  char callsign[7];   // 6-character callsign, with null terminator
};    
static struct ccBuffer txBuf;
  
union {
  struct ccBuffer rxBuf;
  byte buf[CHB_MAX_PAYLOAD];
} rxData;

void setup()  
{ 
  chibiCmdInit(57600);  // initialize the chibi command line to 57600 bps
  chibiInit();

  chibiCmdAdd("getsaddr", cmdGetShortAddr);  // set the short address of the node
  chibiCmdAdd("setsaddr", cmdSetShortAddr);  // get the short address of the node

  analogReference(EXTERNAL);

  pinMode(CW_KEY_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  pinMode(RX_AUDIO_PIN, OUTPUT);
  pinMode(TX_AUDIO_PIN, OUTPUT);
  
  pinMode(ROT_ENC_A, INPUT_PULLUP);
  pinMode(ROT_ENC_B, INPUT_PULLUP);

  ZT.I2cInit();
  ZT.ScI2cMxReset(OLED_ADDRESS);
  delay(5);
  
  oledStr(6, 64-strlen("Chibi-CW")*4, "Chibi-CW"); delay(2);
  oledStr(0, 6*16, "KVz"); delay(2); // Kilo-Virtz (Vi)rtual He(rtz)
  
  // TX spot indicator
  ZT.ScI2cMxFillArea(OLED_ADDRESS, OLED_TXSPOT_PAGE, OLED_TXSPOT_PAGE, 63, 65, 0xff);

  // NOTE: Can't use delay() after here,becaue the TX tone generator greaks it
  
  //temp
  strcpy(txBuf.callsign, "ZL1HIT"); 
} 

/**************************************************************************/
// Loop
/**************************************************************************/
void loop()  { 

  static const int addr = 0xffff;            // broadcast
  
  static unsigned int tuningDial = 0;        // virtual dial frequency 0 - 4095
  const unsigned int filterHalfWidth = 250;  // Hz
  static struct ccBuffer *rxBuf = &rxData.rxBuf;
  static unsigned int lastTuningDial = 0;    // for detectng dial movement
  boolean dialChanged = false; 
  static unsigned int audioFreq = 0;         // Hz
  static unsigned int lastAudioFreq = 0;     // Hz
  static unsigned int rxKeyTimer = 0;
  
  static boolean keyState = false;
  static unsigned int lastTxFreq = 0;        // virtual dial frequency 0 - 4095
  static unsigned int txKeyTimer = 0;
  const unsigned int monitorToneFreq = 600;  // Hz
  
  static unsigned int timer = 0;
  static unsigned int callsignTimer = 0;
  
  // temp
  static unsigned char volumeDial = 16;

  timer++;
  
  /* READ TUNING DIAL */
  // Read the tuning dial, ADC0 - 2x over-sampled to 11-bit resolution, with running average
  // if ((timer % 32) == 0) // read slower, to allow freq to change more slowly/realistically as averaged
  tuningDial = ((tuningDial * 3) + ((analogRead(0)+500)<<1)) / 4;
  
  if (tuningDial != lastTuningDial)
  {
    dialChanged = true;
    lastTuningDial = tuningDial;
  }
  
  // display "frequency" on oLED display
  if (dialChanged)
  {
    char dialString[4];
    // Have the display read 28-30MHz
    sprintf(dialString, "%2d.%02d", (tuningDial/1000)+27, tuningDial/10 % 100);
    oled16x16string(0, 1*16, dialString);
  }

  //oled16x16num(0, 0, dialString);
  
  // debug
  // Serial.println(tuningDial, DEC);
  
  /*****************************/
  /* RECEIVER EVENT PROCESSING */
  
  // EVENT: Incoming radio data
  chibiCmdPoll();       // poll the command line for any user input from the serial port
  if (chibiDataRcvd())
  {
    chibiGetData((uint8_t *)rxData.buf);

    // require 0xCC to be first byte or ignore entire packet
    if (rxBuf->id == CHIBI_CW_IDENT)
    {
      // is the received signal within our filter pass-band?
      if (rxBuf->freq > (tuningDial - filterHalfWidth) && rxBuf->freq < (tuningDial + filterHalfWidth))
      {
        // is this a key odwn or key-up event?
        if (rxBuf->state == 255)
        { // KEY-DOWN event
          audioFreq = audioCenterFreq + (int)(tuningDial - rxBuf->freq);
          if (audioFreq != lastAudioFreq)
          {
            if (!keyState)            // if not tx key-down state
              setRxToneFreq(audioFreq);
            digitalWrite(LED_PIN, 1);
            lastAudioFreq = audioFreq;
          }
          rxKeyTimer = RX_KEY_TIMEOUT;
          
          // Display received callsign
          oledStr(4, 64-strlen(rxBuf->callsign)*4, rxBuf->callsign);
          callsignTimer = 5000;
          oLEDdelay(320);
        }
        else // KEY-UP event
        {
          setNoRxTone();
          digitalWrite(LED_PIN, 0);
          lastAudioFreq = 0;
          rxBuf->freq = 0;
          rxKeyTimer = 0;
        }
      } // else ignore the data
    } // else ignore (bad ID)
    
    // debug
    Serial.print("RXD: "); Serial.print(rxBuf->freq, DEC); Serial.print(" / "); Serial.println(audioFreq, DEC);
  }

  if (--callsignTimer == 0)
  {
    oledStr(4, 64-6*4, "      "); 
    oLEDdelay(320);
  } 
  /* RECEIVE TUNING DIAL CHANGED DURING RX KEY DOWN STATE */
  if (rxKeyTimer) // is rx key down? (if so, we can assume there is a valid receivedFreq) 
  {
    // debug
    //Serial.print("RXTC: "); Serial.print(tuningDial, DEC); Serial.print(" / "); Serial.println(receivedFreq, DEC);

    if (dialChanged) // tuning dial has moved since we last checked
    {
      // are we're still inside the filter
      if (rxBuf->freq > (tuningDial - filterHalfWidth) && rxBuf->freq < (tuningDial + filterHalfWidth))
      {
        // calculate and update new audio tone frequency
        audioFreq = audioCenterFreq + (int)(tuningDial - rxBuf->freq);
        if (!keyState)              // if not tx key-down state
          setRxToneFreq(audioFreq);
      }
      else // we have moved outside the filter pass-band
      {
        setNoRxTone();
        digitalWrite(LED_PIN, 0);
      }
    }      
  }
  
  /* RECEIVER KEY TIMEOUT */
  // turn tone & LED off if no KEY-DOWN event received within timeout count
  if (rxKeyTimer && --rxKeyTimer == 0) 
  {
    setNoRxTone();
    digitalWrite(LED_PIN, 0);
    lastAudioFreq = 0;
  }


  /***********************/
  /* TX EVENT PROCESSING */
  
  /* TRANSMITTER KEY TIMEOUT */
  
  // If the TX key is still down, then we need to resend (refresh) the 
  // keydown signal before it times out at the receiver
  if (keyState && txKeyTimer && --txKeyTimer == 0) 
  {
      txBuf.id = CHIBI_CW_IDENT;
      txBuf.freq = (lastTxFreq) ? lastTxFreq : tuningDial;
      txBuf.state = 255;  // KEY-DOWN event
      chibiTx(addr, (uint8_t *)&txBuf, sizeof(txBuf));  // transmit the data
      lastTxFreq = txBuf.freq;
      
      txKeyTimer = TX_KEY_TIMEOUT;
      
      //debug
      Serial.println("TX TIMEOUT");
  }    
  
  // Check the CW Key, send make or break signals accordingly
  if (!digitalRead(CW_KEY_PIN))  // if KEY-DOWN (electrically inverted input)
  {
    if (!keyState) // send KEY-UP event only if key was last UP
    {
      txBuf.id = CHIBI_CW_IDENT;
      txBuf.freq = tuningDial;
      txBuf.state = 255;  // KEY-DOWN event
      chibiTx(addr, (uint8_t *)&txBuf, sizeof(txBuf));  // transmit the data
      lastTxFreq = tuningDial;
      
      txKeyTimer = TX_KEY_TIMEOUT;

      keyState = true;
      setNoRxTone();                   // tx monitor tone overrides rx tone
      setTxToneFreq(monitorToneFreq);  // monitor tone on
      
      Serial.print("KEY DOWN at: "); Serial.println(txBuf.freq, DEC);
    }
  }
  else // KEY-UP
  {
    if (keyState) // send KEY-UP event only if key was last DOWN
    {
      // send KEY-UP events on same virutal freq as last KEY-UP was sent
      txBuf.id = CHIBI_CW_IDENT;
      txBuf.freq = lastTxFreq;
      txBuf.state = 0;  // KEY-UP event
      chibiTx(addr, (uint8_t *)&txBuf, sizeof(txBuf));  // transmit the data
      lastTxFreq = 0;
      
      keyState = false;
      setNoTxTone();               // tx monitor tone off
      if (rxKeyTimer)              // (re)enable receiver tone output if needed 
        setRxToneFreq(audioFreq);
      
      //Serial.print("KEY UP at: "); Serial.println(lastTxFreq, DEC);
    }
  }

  /* TRANSMIT TUNING DIAL CHANGED DURING RX KEY DOWN STATE */
  if (keyState && dialChanged) // is tx key down and dial changed?
  {
      txBuf.id = CHIBI_CW_IDENT;
      txBuf.freq = tuningDial;
      txBuf.state = 255;  // KEY-DOWN event
      chibiTx(addr, (uint8_t *)&txBuf, sizeof(txBuf));  // transmit the data
      lastTxFreq = tuningDial;

      txKeyTimer = TX_KEY_TIMEOUT;
  } 
  
  
  /**************************************************************************/
  /* ROTARY ENCODER -- sadly, we have to poll the rot. encoder becasue      */
  /* chibi uses PCINTs and I don't really want to mess with that code       */ 
/*
  static unsigned char rotEncABLastState = 0;
  unsigned char rotEncABNow, rotEncChanged;
  
  rotEncABNow  = digitalRead(ROT_ENC_A) | (digitalRead(ROT_ENC_B)<<1);
  rotEncChanged = (rotEncABLastState ^ rotEncABNow);
  if (rotEncChanged) // has either input changed?
  {

    rotEncABLastState = rotEncABNow;

    if (rotEncChanged & 1)
      if ((rotEncABNow == 0 || rotEncABNow == 3) && volumeDial > 0) volumeDial--;
    if (rotEncChanged & 2)
      if ((rotEncABNow == 0 || rotEncABNow == 3) && volumeDial < 16) volumeDial++;

    //Serial.print("ROTENC: "); Serial.println(volumeDial, DEC); // debug
  }
*/

/**************************************************************************/

}

/**************************************************************************/
// CHIBI USER CMD FUNCTIONS
/**************************************************************************/

/**************************************************************************/
/*!
    Get short address of device from EEPROM
    Usage: getsaddr
*/
/**************************************************************************/
void cmdGetShortAddr(int arg_cnt, char **args)
{
  int val;
  
  val = chibiGetShortAddr();
  Serial.print("Short Address: "); Serial.println(val, HEX);
}

/**************************************************************************/
/*!
    Write short address of device to EEPROM
    Usage: setsaddr <addr>
*/
/**************************************************************************/
void cmdSetShortAddr(int arg_cnt, char **args)
{
  int val;
  
  val = chibiCmdStr2Num(args[1], 16);
  chibiSetShortAddr(val);
}    
