/************************************************************/
/*
   Chibi LED Brightness Example
   This is an example of controlling the brighntess of an LED wirelessly. 
   The command line is implemented along with three commands: led, getsaddr, setsaddr.
   "getsaddr" gets the address of the node. "setsaddr" sets the address 
   of the node. Each of the nodes should be set with a unique 16-bit
   address. The "led" command will change the brightness of an LED connected to pin 9
   on the remote node. There is also a printout on each node for the received brightness 
   value. It can be viewed by connecting the node to a serial terminal program.
   
   Directions for use:
   1. Load two nodes with this software.    
   2. Set unique addresses for each node.
   3. Connect LED to pin 9 and GND.
   4. Connect to at least one node via serial terminal program. Ex: Teraterm
   5. Send led command to remote node: 
       led <addr> <brightness>
   Note: the LED brightness value must be a number between 0 and 255
*/
/************************************************************/
#include <chibi.h>

#define RX_AUDIO_PIN   3 // OC2B
#define TX_AUDIO_PIN   5 // OC0B
#define ROT_ENC_A      6
#define ROT_ENC_B      7
#define CW_KEY_PIN     8
#define LED_PIN        9


/**************************************************************************/
// Initialize
/**************************************************************************/

void setup()  
{ 
  chibiCmdInit(57600);  // initialize the chibi command line to 57600 bps
  chibiInit();

  chibiCmdAdd("led", cmdLed);  // send LED brightness remote node
  chibiCmdAdd("getsaddr", cmdGetShortAddr);  // set the short address of the node
  chibiCmdAdd("setsaddr", cmdSetShortAddr);  // get the short address of the node

  analogReference(EXTERNAL);

  pinMode(CW_KEY_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  pinMode(RX_AUDIO_PIN, OUTPUT);
  pinMode(TX_AUDIO_PIN, OUTPUT);
  
  pinMode(ROT_ENC_A, INPUT_PULLUP);
  pinMode(ROT_ENC_B, INPUT_PULLUP);
} 


void setRxToneFreq(unsigned int hertz, unsigned char volume=16/*0 to 16*/)
{
  // OCR2A is TOP. OCR2B is the toggle point, which should be half of OCR2A for 50% duty cycle
  // OCR2A = 250; --> 500Hz
  // OCR2A = 96; --> 1300Hz
  unsigned int top;
  unsigned char middle;
  
  top = 8000000UL / 128 / hertz;
// Serial.print("RXTOP="); Serial.println(top, DEC);
  
  if (volume < 1) 
  {
    setNoRxTone();
    return;
  }  
  // top/2 = full volume. So, 16 steps from 1 to 16. Ex: vol=1, top=96 => middle=3
  middle = (top / 2 / 16) * ((volume < 17) ? volume : 16);
  OCR2A = top;
  OCR2B = middle;
  TCCR2B = (1<<WGM22) | (1<<CS22);   /* CLK divided by 64 (we're running at 8MHz) */
  TCCR2A = (1<<COM2B1) | (1<<WGM20); // tone output on (non-inverting)
}

void setTxToneFreq(unsigned int hertz, unsigned char volume=16/*0 to 16*/)
{
  // OCR0A is TOP. OCR0B is the toggle point, which should be half of OCR0A for 50% duty cycle
  // OCR0A = 250; --> 500Hz
  // OCR0A = 96; --> 1300Hz
  unsigned int top;
  unsigned char middle;
 
  top = 8000000UL / 128 / hertz;
// Serial.print("TXTOP="); Serial.println(top, DEC);
  
  if (volume < 1) 
  {
    setNoTxTone();
    return;
  }  
  // top/2 = full volume. So, 16 steps from 1 to 16. Ex: vol=1, top=96 => middle=3
  middle = (top / 2 / 16) * ((volume < 17) ? volume : 16);
  OCR0A = top;
  OCR0B = middle;
  TCCR0B = (1<<WGM02) | (1<<CS01) | (1<<CS00); /* CLK divided by 64 (we're running at 8MHz) */
  TCCR0A = (1<<COM0B1) | (1<<WGM00);           // tone output on (non-inverting)
}

void setNoRxTone()
{
  TCCR2A = (1<<WGM20);
}

void setNoTxTone()
{
  TCCR0A = (1<<WGM00);
}

/**************************************************************************/
// Loop
/**************************************************************************/
#define RX_KEY_TIMEOUT 4000
#define TX_KEY_TIMEOUT 3000
void loop()  { 

  static const int addr = 0xffff; // broadcast
  static uint8_t txBuf[3] = { 0, 0, 0 };     // [0-1]=16-bit virtualFreq, [2]=8-bit boolean key - up or down
  
  static unsigned int tuningDial = 0;        // virtual dial frequency 0 - 4095
  const unsigned int filterHalfWidth = 250;  // Hz
  static unsigned int lastTuningDial = 0;    // for detectng dial movement

  static unsigned int receivedFreq = 0;      // virtual freq, 0 - 4095
  const unsigned int audioCenterFreq = 600;  // Hz
  static unsigned int audioFreq = 0;         // Hz
  static unsigned int lastAudioFreq = 0;     // Hz
  static unsigned int rxKeyTimer = 0;
  
  static boolean keyState = false;
  static unsigned int lastTxFreq = 0;        // virtual dial frequency 0 - 4095
  static unsigned int txKeyTimer = 0;
  const unsigned int monitorToneFreq = 600; // Hz
  
  static unsigned int timer = 0;

  // temp
  static unsigned char volumeDial = 16;

  
  timer++;
  
  /* READ TUNING DIAL */
  // Read the tuning dial, ADC0 - 4x over-sampled to 12-bit resolution, with running average
  if ((timer % 128) == 0) // read slower, to allow freq to change more slowly/realistically as averaged
    tuningDial = ((tuningDial * 7) + (analogRead(0)<<2)) / 8;
  
  // debug
  // Serial.println(tuningDial, DEC);
  
  /*****************************/
  /* RECEIVER EVENT PROCESSING */
  
  // EVENT: Incoming radio data
  chibiCmdPoll();       // poll the command line for any user input from the serial port
  if (chibiDataRcvd())
  {
    byte buf[CHB_MAX_PAYLOAD];
    chibiGetData(buf);
 
    receivedFreq = buf[0] + (buf[1] * 256); // 16-bit, little endian
 
    // is the received signal within our filter pass-band?
    if (receivedFreq > (tuningDial - filterHalfWidth) && receivedFreq < (tuningDial + filterHalfWidth))
    {
      // is this a key odwn or key-up event?
      if (buf[2])
      { // KEY-DOWN event
        audioFreq = audioCenterFreq + (int)(tuningDial - receivedFreq);
        if (audioFreq != lastAudioFreq)
        {
          if (!keyState)            // if not tx key-down state
            setRxToneFreq(audioFreq, volumeDial);
          digitalWrite(LED_PIN, 1);
          lastAudioFreq = audioFreq;
        }
        rxKeyTimer = RX_KEY_TIMEOUT;
      }
      else // KEY-UP event
      {
        setNoRxTone();
        digitalWrite(LED_PIN, 0);
        lastAudioFreq = 0;
        receivedFreq = 0;
      }
    } // else ignore the data
    
    // debug
    Serial.print("RXD: "); Serial.print(receivedFreq, DEC); Serial.print(" / "); Serial.println(audioFreq, DEC);
  }
  
  /* RECEIVE TUNING DIAL CHANGED DURING RX KEY DOWN STATE */
  if (rxKeyTimer) // is rx key down? (if so, we can assume there is a valid receivedFreq) 
  {
    // debug
    //Serial.print("RXTC: "); Serial.print(tuningDial, DEC); Serial.print(" / "); Serial.println(receivedFreq, DEC);

    if (lastTuningDial != tuningDial) // tuning dial has moved since we last checked
    {
      // are we're still inside the filter
      if (receivedFreq > (tuningDial - filterHalfWidth) && receivedFreq < (tuningDial + filterHalfWidth))
      {
        // calculate and update new audio tone frequency
        audioFreq = audioCenterFreq + (int)(tuningDial - receivedFreq);
        if (!keyState)              // if not tx key-down state
          setRxToneFreq(audioFreq, volumeDial);
      }
      else // we have moved outside the filter pass-band
      {
        setNoRxTone();
        digitalWrite(LED_PIN, 0);
      }
      lastTuningDial = tuningDial;
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
  
  // If the TX key is still down out timoue then we need to resend (refresh) the 
  // keydown signal before it times out at the receiver
  if (keyState && txKeyTimer && --txKeyTimer == 0) 
  {
      unsigned int freq;
      freq = (lastTxFreq) ? lastTxFreq : tuningDial;
      txBuf[0] = freq % 256; // 16-bit LO byte
      txBuf[1] = freq / 256; // 16-bit HI byte
      txBuf[2] = 255;  // KEY-DOWN event
      chibiTx(addr, txBuf, 3);  // transmit the data
      lastTxFreq = freq;
      
      txKeyTimer = TX_KEY_TIMEOUT;
      
      //debug
      Serial.println("TX TIMEOUT");
  }    
  
  // Check the CW Key, send make or break signals accordingly
  if (!digitalRead(CW_KEY_PIN))  // if KEY-DOWN (electrically inverted input)
  {
    if (!keyState) // send KEY-UP event only if key was last UP
    {
      txBuf[0] = tuningDial % 256; // 16-bit LO byte
      txBuf[1] = tuningDial / 256; // 16-bit HI byte
      txBuf[2] = 255;              // signal KEY-DOWN event
      chibiTx(addr, txBuf, 3);     // transmit the data
      lastTxFreq = tuningDial;
      
      txKeyTimer = TX_KEY_TIMEOUT;

      keyState = true;
      setNoRxTone();                   // tx monitor tone overrides rx tone
      setTxToneFreq(monitorToneFreq, volumeDial);  // monitor tone on
      
      // debug
      Serial.print("TXMAKE: "); Serial.println(tuningDial, DEC);
    }
  }
  else // KEY-UP
  {
    if (keyState) // send KEY-UP event only if key was last DOWN
    {
      // send KEY-UP events on same virutal freq as last KEY-UP was sent
      txBuf[0] = lastTxFreq % 256; // 16-bit LO byte
      txBuf[1] = lastTxFreq / 256; // 16-bit HI byte
      txBuf[2] = 0;                // signal KEY-UP event
      chibiTx(addr, txBuf, 3);     // transmit the data
      lastTxFreq = 0;
      
      keyState = false;
      setNoTxTone();               // tx monitor tone off
      if (rxKeyTimer)              // (re)enable receiver tone output if needed 
        setRxToneFreq(audioFreq, volumeDial);
      
      // debug
      Serial.print("TXBREAK: "); Serial.println(lastTxFreq, DEC);
    }
  }

  /* TRANSMIT TUNING DIAL CHANGED DURING RX KEY DOWN STATE */
  if (keyState && lastTuningDial != tuningDial) // is tx key down and dial changed?
  {
    // debug
    //Serial.print("TXTC: "); Serial.print(tuningDial, DEC); Serial.print(" / "); Serial.println(receivedFreq, DEC);

      txBuf[0] = tuningDial % 256; // 16-bit LO byte
      txBuf[1] = tuningDial / 256; // 16-bit HI byte
      txBuf[2] = 255;  // KEY-DOWN event
      chibiTx(addr, txBuf, 3);  // transmit the data
      lastTxFreq = tuningDial;

      lastTuningDial = tuningDial;

      txKeyTimer = TX_KEY_TIMEOUT;
  } 
  
  
  /**************************************************************************/
  /* ROTARY ENCODER -- sadly, we have to poll the rot. encoder becasue      */
  /* chibi uses PCINTs and I don't really want to mess with that code       */ 

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

    if (keyState) setTxToneFreq(monitorToneFreq, volumeDial);
    else if (rxKeyTimer) setRxToneFreq(audioFreq, volumeDial);
    
    //Serial.print("ROTENC: "); Serial.println(volumeDial, DEC); // debug
  }

/**************************************************************************/


}

/**************************************************************************/
// USER FUNCTIONS
/**************************************************************************/

/**************************************************************************/
/*!
   Send data to control the servo.
   Usage: servo <addr> <position>
*/
/**************************************************************************/
void cmdLed(int argc, char **argv)
{
  unsigned int addr;
  byte ledVal, data[1];
  
  // argv[1] is the first argument entered into the command line. This should
  // be the destination address of the remote node we want to send the command to.
  // The address is always a hexadecimal value.
  addr = chibiCmdStr2Num(argv[1], 16);
  
  // argv[2] is the second argument entered into the command line. This should
  // be the servo position. We're going to send this data to the remote node.
  ledVal = chibiCmdStr2Num(argv[2], 10);
  data[0] = ledVal;
  
  // Send out the data
  chibiTx(addr, data, 1);
}

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
