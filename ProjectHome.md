<img src='http://chibi-cw.googlecode.com/svn/wiki/attach/chibi-cw_rev1.0.png' align='right'>
<h1>chibi-CW</h1>

<b>A Shield for the <a href='http://freaklabs.org'>Freaklabs</a> Freakduino  (Arduino compatible board, with onboard 2.4GHz radio) designed using <a href='http://www.kicad-pcb.org'>KiCAD</a>.</b>

Chibi-CW is intended for Amateur Radio clubs or other morse code enthusiasts, scouts, etc. No operating license is required.<br>
<br>
"CW" or "Constant Wave" is what we hams call the type of signal most typically used for old fashioned, Morse code style communications. This device does not in fact use constant waves at the radio end of things. But it aims to produce a close emulation for the human operator. That is, it allows for audible (and visual, actually) wireless, Morse code communications, over distances similar to computer Wi-Fi. Sorry, but I'm going to have you leave you guessing as to the question, "WHY?" ;-)<br>
<br>
The chibi-CW unit plugs into an Arduino compatible, 2.24GHz wireless radio board, named the Freakduino. Such plug-in Arduino boards are coined, "shields", for reasons I don't really know.<br>
<br>
<img src='http://chibi-cw.googlecode.com/svn/wiki/attach/prototype2.png' />

<i>Above: First prototype chibi-cw PCB installed in one of two test radios.</i>

<i>Below: Earlier version (minus the chibi-cw board) showing OLED display and front panel knobs.</i>

<img src='http://chibi-cw.googlecode.com/svn/wiki/attach/prototype.png' />

My wife and I use these radios to practice sending and receiving morse code together.<br>
<br>
And now for a friendly dose of jargon ...<br>
<br>
The transmission is digital, using the Freaklabs Chibi ("Midget") 802.15.4 minimalist protocol stack. Only short key-up and key-down event packets are sent (at about 1Mbps) making for very light bandwidth usage.<br>
<br>
A large VFO dial is used to simulate a typical home made QRP radio, having space for about ten, 400Hz wide, simultaneous signals. This adds some realism, with the firmware providing similar tone frequency variance to that of "real" single sideband CW radio reception. That is, "spotting" of Tx vs. Rx. tones is required for the best experience. (Signals can overlap, but only one received tone can be heard at any given time.)<br>
<br>
The output audio is bandpass filtered, to provide a more pleasant tone, compared to the square waves produced by the ATmega328P MCU. Separate receive and transmit audio tones operate together, thus facilitating, "spotting by ear". That is, using one's ear to match transmit and receive audio tone frequency. There is also clear visual reference on the OLED display, which helps train "CW" newcomers to, "spot" effectively.<br>
<br>
An OLED display is employed for visual status and settings editing.<br>
<br>
A rotary encoder with push-button feature is used for a volume and menu / settings navigation controls.<br>
<br>
There is also a red/green LED to indicate RX/TX status. Interestingly, the receive LED will still light, even if the transmitter key is down. This may help in a CW trainee scenario, to indicate, "doubling".<br>
<br>
<b>Firmware Status</b>

As at 2013-11-12, all basic transceiver functions are working. WIth the first prototype PCB now built, the Texas Instruments, I2C controlled audio headphone amplifier chip is working well. Several OLED display functions are in place, including received station call-sign display, spotting visual aids and of course, the virtual operating frequency. The rotary encoder with push-button action front panel knob is not yet coded for. It is intended to use this primarily for earphone volume control but also for a basic on screen menu navigation, eventually (for entering station call-sign, for example.)<br>
<br>
TODO: Link to a video of the prototype in operation.