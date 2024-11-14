# Fibonacci-Nano-ESP32-clock
Fibonacci clock on Arduino Nano ESP32 with NTP time from WIFI.

More Fibonacci clocks:[here on Nano ](https://github.com/ednieuw/Fibonacci-Vierkantekokerklok)<br>
and [here on ESP32 C3 and S3](https://github.com/ednieuw/Fibonacci-ESP32-C3-S3-Clock).

![324437027-e79ee14f-d8e0-469f-83f9-d1d13a5135cf](https://github.com/user-attachments/assets/dfa9f4b1-f42c-4eee-beea-c9c7c5491311)

You can use one of the PCB's from [the PCB designs here](https://github.com/ednieuw/NanoESP32PCB)

But the "Arduino Nano ESP32 small PCB" is probably the best choice or connection directly to the Arduino Nano ESP32 pin is also possible.

![image](https://github.com/user-attachments/assets/e39ba5ca-d649-4f43-8106-5f46e602dc66)

![image](https://github.com/user-attachments/assets/dacd3f89-f4d3-4dd8-9007-4a9766bb75ff)


```
1 x PCB Nano ESP32
1 x Arduino Nano ESP32 with pins
1 x 1000 µF condensator 
1 x 470Ω resistor
1 x 10kΩ resistor
1 x diode 1N5817
1 x 74AHCT125 level shifter
1 x 74AHCT125 DIP14 socket
1 x 2-pin female connector (LDR)
1 x 5-pin female connector (Rotary)
1 x 6-pin female connector (DS3231 RTC)
2 x 15-pin female connector
1 x RCT DS3231 Precisie klok module ZS-042
1 x CR 2032 3V lithium battery
1 x LDR light sensor 
1 x power connector KF350 5.08 mm
1 x WS2812 LED-strip or
1 x SK6812 LED-strip 
```


# Before starting

The clock receives time from the internet. Therefore the name of the WIFI station and its password must be entered to be able to connect to a WIFI router.
The name of the WIFI-station and password has to be entered once. These credentials will be stored in memory of the MCU.
To make life easy it is preferred to use a phone and an communication app of a phone or tablet enter the WIFI credentials into the clock.
 	 	 
BLESerial nRF	BLE Serial Pro	Serial Bluetooth Terminal
- Download a Bluetooth UART serial terminal app on your phone, PC, or tablet.
- For IOS: BLE Serial Pro or BLESerial nRF.
- For Android: Serial Bluetooth Terminal.

# Compiling the program

```
//========================================================================================
// ------------------>   Define How many LEDs in fibonacci clock
const int NUM_LEDS = 14;    // How many leds in fibonacci clock? (12 / 14 / 24 / 32 /36 /174 )
                            // check the LED positions in  setPixel() (at the end of file) !!  

//------------------------------------------------------------------------------
// PIN Assigments for Arduino Nano ESP32
//------------------------------------------------------------------------------ 
 
enum DigitalPinAssignments {      // Digital hardware constants ATMEGA 328 ----
 SERRX        = D0,               // D1 Connects to Bluetooth TX
 SERTX        = D1,               // D0 Connects to Bluetooth RX
 encoderPinB  = D2,               // D8 left (labeled CLK on decoder)no interrupt pin   
 encoderPinA  = D3,               // D3 right (labeled DT on decoder)on interrupt  pin
 clearButton  = D4,               // D4 switch (labeled SW on decoder)
 LED_PIN      = D5,               // D5 / GPIO 8 Pin to control colour SK6812/WS2812 LEDs (replace D5 with 8 for NeoPixel lib)
 PCB_LED_D09  = D9,               // D9
 PCB_LED_D10  = D10,              // D10
 secondsPin   = 48,               // D13  GPIO48 (#ifdef LED_BUILTIN  #undef LED_BUILTIN #define LED_BUILTIN 48 #endif)
 };
 
enum AnaloguePinAssignments {     // Analogue hardware constants ----
 EmptyA0      = A0,               // Empty
 EmptyA1      = A1,               // Empty
 PhotoCellPin = A2,               // LDR pin
 OneWirePin   = A3,               // OneWirePin
 SDA_pin      = A4,               // SDA pin
 SCL_pin      = A5,               // SCL pin
 EmptyA6     =  A6,               // Empty
 EmptyA7     =  A7};              // Empty
```
At the top of the INO-file one has to define how the LEDs are installed. 
A few lines below the pin definitions for the attached LDR and LED-strip are defined. These can be changed if you use other pins

Select the Arduino ESP32 board
Select By GPIO numbering.
![image](https://github.com/user-attachments/assets/5cf12459-5826-417b-a234-e2081a01e1c3)


3)Flash and Monitor
  You can now upload your sketch to the device. 
  To monitor the device in the serial monitor, you need to select the USB port and open the Monitor tool selecting the correct baud rate (usually 115200) according to the Serial.begin() defined in your code.
4) Enter (or send) the letter I in the serial terminal to see the menu of the Fibonacci clock displayed.

When the serial monitor is on during start up something similar as below is printed. In this case the credentials for the WIFI router are known to the software.
```
Serial started
Mem.Checksum = 25065
Stored settings loaded
LED strip is WS2812
LED strip started
Rotary NOT used
Rotary available
Found I2C address: 0X57
Found I2C address: 0X68
External RTC module IS found
DS3231 RTC software started
BLE started
Scanning for networks
4: networks found
 1:            FRITZ!Box -50 * -- Will connect to
 2:          NETGEAR_EXT -72  
 3:     ENVY 5640 series -74 *
 4:          H369A209CE1 -74 *
[WiFi] WiFi is disconnected, will reconnect
[WiFi-event] event: 4  : Connected to access point
[WiFi-event] event: 7  : Obtained IP address: 192.168.178.141
IP Address: 192.168.178.141
NTP is On
Web page started
WIFI started
14/11/2024 17:38:38 
___________________________________
A SSID B Password C BLE beacon name
D Date (D15012021) T Time (T132145)
E Timezone  (E<-02>2 or E<+01>-1)
F Own colour  (Hex FWWRRGGBB)
G Scan WIFI networks
H Toggle use rotary encoder
I To print this Info menu
J Toggle use DS3231 RTC module
K LDR reads/sec toggle On/Off
N Display off between Nhhhh (N2208)
O Display toggle On/Off
P Status LED toggle On/Off
Q Display colour choice (Q for options)
R Reset settings @ = Reset MCU
U Demo mode (msec) (M200)
--Light intensity settings (1-250)--
S Slope, L Min, M Max  (S80 L5 M200)
W WIFI, X NTP&, CCC BLE, + Fast BLE
# Self test, ! See RTC, & Update RTC
Ed Nieuwenhuys November 2024
___________________________________
Display off between: 23h - 08h
Display choice: Yellow
Slope: 10     Min: 5     Max: 255 
SSID: FRITZ!BoxEd
BLE name: RedPCBV01
IP-address: 192.168.178.141 (/update)
Timezone:CET-1CEST,M3.5.0,M10.5.0/3
WIFI=On NTP=On BLE=On FastBLE=Off
LED strip: WS2812 (Send % to switch)
Software: ESP32ArduinoFibonacci_V003.ino
ESP32 Arduino core version: 2.0.17
14/11/2024 17:38:38 
___________________________________

LDR:100= 5% 220266 l/s 17:38:39
LDR:102= 5% 220081 l/s 17:39:00
```
5) Perform a reset with option R in the menu of the program after the first upload to be sure everything is set to their default settings.

- The working of the code is explained here: https://github.com/ednieuw/ESP32-C3-Clock-and-more
- Building a Fibonacci clock is explained here: https://github.com/ednieuw/Fibonacci-Vierkantekokerklok
- a case can  be 3d-printed. See here:  https://www.thingiverse.com/thing:6483258
- And here: https://ednieuw.home.xs4all.nl/Woordklok/FibonacciClock/FibonacciClock.html

Connect a WS2812 or SK6812 LED strip with at least 12 LEDs to the Arduino Nano ESP32 and the rest of the hardware. 

# Using OTA
An easy method is to install the Fibonacci clock program for the first time Over The Air (OTA).
In my program I use the ElegantOTA library but I found out recently that the OTA from Arduino also is also fine to use.

Compile and upload the Arduino OTA.ino file from the examples in the Arduino IDE. (File --> Examples -> Arduino-OTA).

Open in Examples in the Arduino IDE: ArduinoOTA 
Enter your WIFI credentials in the program at line 8 and 9
Select the proper board in the IDE.
Compile and upload to the ESP32. 
Open the serial monitor in the IDE to see the IP address
Or find out in your router what IP address it has received.
Type in a  browser the IP-address. Something like http://192.168.178.156/ in my case.
Login with ‘’admin’’ and ‘’admin’’
Load the “ESP32-C3S3-FiboV010.ino.esp32s3.bin” file. (or a higher V010) 
And hops, the app is running.
Enter your WIFI credentials with BLE op your mobile phone in the menu of the clock, Reset and the clock is running.
Or open the ip-addres in your router (in my case: http://192.168.178.156/) and enter the credential wit optie A and B from the menu.(See below for a detailed description)

#  Installations  

To connect to a WIFI network a SSID (WIFI name) and password must be entered.
There are a few methods:
Connect the MCU in the clock with a micro USB serial cable to a PC and use a serial terminal.  
Use a BLE serial terminal app on a phone or tablet for connection.

For a PC the app Termite is fine as serial terminal.

For IOS use:  BLE Serial Pro or BLESerial nRF.  
![image](https://github.com/ednieuw/Fibonacci-ESP32-C3-S3-Clock/assets/12166816/7b269328-cea3-433c-bee3-04485b151789)

<br>For Android use: Serial Bluetooth terminal.

Bluetooth Low Energy (BLE) can use two types of protocol CC25nn or nRF52nn where nn is a specific number. This clock uses nRF52 from the company Nordic.  

- Start the app and start a connection with the clock. Some apps automatically start with a connection window but for some a connection symbol must be pressed. You will most probably find one station to select from. 

- Select the clock in the list.

- The app will display a window and a line where commands can be entered and send to the clock.


On the Arduino Nano ESP32 there is a LED that will have a red dot lighted when the program is running. 
A green dot will turn on when there is a WIFI connection.
When there is a Bluetooth connection a blue dot in the LED will light on.
The LEDs on the Arduino Nano ESP32 and connected to pin D9 and D10 will turn On and Off every second.

- Sending the letter I or i for information will display the menu followed with the actual settings of several preferences.

Enter the first letter of the setting you want to changes followed with a code.
Some entries just toggle On and Off. Like the W to set WIFI Off or On. 

To change the SSID and password:
Send the letter A or a followed with the WIFI station name. 
Amy-ssid and send this command. Eg AFRITZ!Box01 or aFRITZ!Box01. Starting with an upper or lower case character is an identical instruction in the command string

Then the letter B followed with the password.
Bmypassword and send the password.

Cbroadcastname  will change to name displayed in the Bluetooth connection list. Something like: cMyClock

If the length of the SSID and/or password is less then 5 characters the WIFI will be turned off automatically to avoid connection errors.
Use a length of minimal 8 characters for SSID and password.
Check in the menu (third row from the bottom) if WIFI and NTP are on.
![image](https://github.com/ednieuw/Fibonacci-ESP32-C3-S3-Clock/assets/12166816/2eeb1005-affa-4c01-b7c8-1cc9cb60b0be)

Enter @ to restart the MCU. It will restart and connections will be made. 
Sometimes a second or third reset must be given to get the clock connected to WIFI. If connection still fails check the SSID name and the entered password. (send the letter b, an easter egg))
If WIFI is connected the LED on the MCU will turn on a green dot.	
`
 	Menu displayed in serial output.

To set a time zone. Send the time zone string between the quotes prefixed with the character E or e.
See the entry strings for the time zones at the bottom of this page.
For example; if you live in Australia/Sydney send the string, eAEST-10AEDT,M10.1.0,M4.1.0/3

![MenuWebpage](https://github.com/ednieuw/Fibonacci-ESP32-C3-S3-Clock/assets/12166816/e73d9e20-f8b0-4c25-b4e6-d9b4f81bb65e)
HTML page on iPhone 	

![Menu-iPhone](https://github.com/ednieuw/Fibonacci-ESP32-C3-S3-Clock/assets/12166816/edad0443-be74-4dee-b2a2-8f5a400ef070)
iPhone BLE serial terminal

# Upgrading software
Software can be upgraded over the air. (OTA) by opening a web browser and entering the IP-address of the clock followed with /update. 
For example: 192.168.178.78/update. 
Choose firmware and click on Choose File.
Choose the appropriate bin file. 

# Control and settings of the clock
As mentioned before the clock can be controlled with the WIFI webpage or BLE UART terminal app.
When the clock is connected to WIFI the IP-address is displayed when the clock is in Digital display mode. (Q6 in the menu)
The IP-address is also printed in the menu. As a last resort the IP-address can be found in you WIFI router. 

To start the menu in a web page the IP-address numbers and dots (for example: 192.168.178.77) must be entered in the web browser of your mobile or PC where you type your internet addresses (URL).

Or with a Bluetooth connection:

Open the BLE terminal app. 
Look for the clock to connect to and connect.

Unfortunately some apps can not read strings longer than 20 characters and you will see the strings truncated or garbled.
If you see a garbled menu enter and send the character 'Z' to select the slower transmission mode.
If transmission is too garbled and it is impossible to send the character Z try the web page to send the character Z.
A third method is to use an iPhone, iPad of iMac with the free BLE nRF app.
If all fails you have to connect the MCU inside the clock  with a micro USB cable to a PC and use a serial terminal app to send a Z.
 

# Regulating the light intensity of the display

In the menu light intensity of the display can be controlled with three parameters:
--Light intensity settings (1-250)--
S=Slope V=Min  U=Max   (S80 V5 U200)
The default values are between the ().

S How fast the brightness reaches maximum brightness.
V How bright the display is in complete darkness.
U the maximum brightness of the display.

In the bottom half of the menu the stored values are displayed
Slope: 80     Min: 5     Max: 255

The clock reacts on light with its LDR (light dependent resistor).


When it gets dark the display does not turn off completely but will stay dimmed at a minimum value. 
With the parameter V the lowest brightness can be controlled. With a value between 0 and 255 this brightness can be set.
V5 is the default value. 
The maximum brightness of the display is controlled with the parameter U. Also a value between 0 and 255.
With parameter S the slope can be controlled how fast maximum brightness is achieved. 



 
Settings are set by entering the first character of a command following by parameters if necessary.
For example: 
To set the colours of the fonts in the display to white enter: Q2



Turn off WIFI by sending a W.
Restart the clock with the letter @.
Reset to default setting by send R. 

  	 
HTML page	BLE menu

 
# Updating the software
 
The software can be updated ‘Over The Air’ when the clock is connected to WIFI.
You can find the IP-address in the menu or in the digital display mode menu option Q6.
Enter the IP-address of the clock followed with /update
http://192.168.178.78/update

‘Choose File’ in the menu and select the bin file to update.
Something like: Liygo-WristWatchESP32-V011.ino.twatch.bin 
where V011 is the version number
 


# Detailed description

With the menu many preferences can be set. These preferences are stored on a SD-card or in the ESP32-S3 storage space.
 
Enter the first character in the menu of the item to be changed followed with the parameter.
There is no difference between upper or lower case. Both are OK.
Between the ( ).

```
A SSID B Password C BLE beacon name
Change the name of the SSID of the router to be connected to. 
For example: aFRITZ!BoxEd or AFRITZ!BoxEd.
Then enter the password. For example: BSecret_password.
Restart the clock by sending @. 
Entering a single 'b' will show the used password. This ‘Easter egg’ can be used to check if a valid password was entered.

D Set Date  and T Set Time 
If you are not connected to WIFI you have to set time and date by hand.
For example enter: D06112022 to set the date to 6 November 2022.
Enter for example T132145 (or 132145 , or t132145)  to set the time to 45 seconds and 21 minute past one o'clock.

E Set Timezone E<-02>2 or E<+01>-1
At the bottom of this page you can find the time zones used in 2022. 
It is a rather complicated string and it is therefore wise to copy it.
Let's pick one if you happen to live here: Antarctica/Troll,"<+00>0<+02>-2,M3.5.0/1,M10.5.0/3"
Copy the string between the " "'s and send it with starting with an 'E' or 'e' in front.
E<+00>0<+02>-2,M3.5.0/1,M10.5.0/3.
Time zones and daylight savings should be ended and replaced by one universal date and time for the while planet cq universe. But that is my opinion.

"F Fibonacci or Chrono display",
switch 12 LED clock between Fibonacci or chrono display

G Scan WIFI networks
See a list of WIFI network that can be connected to

H Toggle use rotary encoder
Toggle the use of the rotary encoder.
Switch to Off when there in no rotary encoder connected to the PCB

I To print this Info menu
Print the menu to Bluetooth and the serial monitor connected with an USB-cable.

K Reads/sec toggle On/Off
Entering a K toggles printing of the LDR reading of the measured light intensity. 
It also shows how many times the processor loops through the program and checks its tasks to run the clock. 

N Display off between Nhhhh (N2208)
With N2208 the display will be turned off between 22:00 and 8:00.

O Toggle the display off and on.

P Toggle the status LEDs on the MCU off and on.

Q Display colour choice (Q0-9)
  Q0= Mondriaan1
  Q1= Mondriaan2
  Q2= RGB
  Q3= Greens
  Q4= Pastel
  Q5= Modern
  Q6= Cold
  Q7= Warm
  Q8= Mondriaan3
  Q9= Mondriaan4

R Reset settings 
R will set all preferences to default settings, it also clears the SSID and password.

--Light intensity settings (1-250)--
S=Slope V=Min  U=Max   (S80 V5 U200)
See chapter: Regulating the light intensity of the display

@ = Reset MCU
@ will restart the MCU. This is handy when the SSID, et cetera are changed and the program must be restarted. Settings will not be deleted.
U Demo mode (msec) (U200)
Let the run the time at a higher speed. U200 will run the clock 5 times faster.


W=WIFI, X=NTP&, Y=BLE, Z=Use SD
Toggle WIFI, NTP on and off.
Enter the character will toggle it on or off. 
At the bottom of the menu the stated is printed.
 
Sending a & will start a query from the time server.
Z Fast BLE
The BLE UART protocol sends default packets of 20 bytes. Between every packet there is a delay of 50 msec.
The IOS BLEserial app, and maybe others too, is able to receive packets of 80 bytes or more before characters are missed. 
Option Z toggles between the long and short packages. 
```
