/* 
 Author .    : Ed Nieuwenhuys
 Changes V001: Derived from ESP32Arduino-wordclock-V055.ino and ESP32_C3S3_FiboV010
 Changes V002: Working version
 Changes V003: On Github and updated Github page. MDNS.begin(BLEbroadcastName)) 
               Identical common routines ESP32ArduinoFibonacci_V003 / ESP32WordClockUltimatePCB_V028 / ESP32WordClockV055 
 Changes V004: BBBB, WIFI custom name in router
 Changes V005: 
 Changes V006: 
 Changes V007: 
 Changes V008: 
 Changes V009:
 Changes V010:


How to compile: 

Check if ELEGANTOTA_USE_ASYNC_WEBSERVER 1 in ElegantOTA.h
// Locate the ELEGANTOTA_USE_ASYNC_WEBSERVER macro in the ElegantOTA.h file, and set it to 1:
// #define ELEGANTOTA_USE_ASYNC_WEBSERVER 1

Install ESP32 boards
Board: Arduino Nano ESP32
Partition Scheme: With FAT
Pin Numbering: By GPIO number (legacy). Not  By Arduino pin (default)
Select below, with only one #define selected, the clock type

*/
// =============================================================================================================================

// ------------------>   Define How many LEDs in fibonacci clock
const int NUM_LEDS = 14;    // How many leds in fibonacci clock? (12 / 14 / 24 / 32 /36 /174 )
                            // check the LED positions in  setPixel() (at the end of file) !!  


//--------------------------------------------
// ESP32 Definition of installed modules
// Define the modules installed in the clock by removing the // before the #define
//--------------------------------------------
//#define ONEWIREKEYPAD3x1       // Use a 3x1 keypad with one wire
//#define ONEWIREKEYPAD3x4       // Use a 3x4 keypad with one wire

//--------------------------------------------
// ESP32 Includes defines and initialisations
//--------------------------------------------

//#include <Arduino.h>
#include <Preferences.h>
                      #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
#include "EdSoftLED.h"         // https://github.com/ednieuw/EdSoftLED for LED strip WS2812 or SK6812 
                      #else
#include <Adafruit_NeoPixel.h> // https://github.com/adafruit/Adafruit_NeoPixel   for LED strip WS2812 or SK6812
                      #endif
#include <NimBLEDevice.h>      // For BLE communication  https://github.com/h2zero/NimBLE-Arduino
#include <ESPNtpClient.h>      // https://github.com/gmag11/ESPNtpClient
#include <WiFi.h>              // Used for web page 
#include <AsyncTCP.h>          // Used for webpage  https://github.com/dvarrel/AsyncTCP                    old https://github.com/me-no-dev/ESPAsyncWebServer
#include <ESPAsyncWebServer.h> // Used for webpage  https://github.com/mathieucarbou/ESPAsyncWebServer     Old one with  https://github.com/me-no-dev/ESPAsyncWebServer
#include <ElegantOTA.h>        // If a large bunch of compile error see here :https://docs.elegantota.pro/async-mode
                               // Locate the ELEGANTOTA_USE_ASYNC_WEBSERVER macro in the ElegantOTA.h file, and set it to 1:
                               // #define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
#include <ESPmDNS.h>
#include <Wire.h>              // Ter zijner tijd Wire functies gaan gebruiken. Staan al klaar in de code 
#include <RTClib.h>            // Used for connected DS3231 RTC // Reference https://adafruit.github.io/RTClib/html/class_r_t_c___d_s3231.html
#include <Encoder.h>


//--------------------------------------------
// SPIFFS storage
//--------------------------------------------
Preferences FLASHSTOR;

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
 EmptyA6      = A6,               // Empty
 EmptyA7      = A7};              // Empty

 //--------------------------------------------
// COLOURS stored in Mem.DisplayChoice
//--------------------------------------------   

const byte DEFAULTCOLOUR = 0;
const byte HOURLYCOLOUR  = 1;          
const byte WHITECOLOR    = 2;
const byte OWNCOLOUR     = 3;
const byte OWNHETISCLR   = 4;
const byte WHEELCOLOR    = 5;
const byte DIGITAL       = 6;
const byte ANALOOG       = 7;

#define CLOCK_PIXELS    5                                           // Number of cells in clock = 5 (1,1,2,3,5)
#define DISPLAY_PALETTE 1                                           // Default palette to start with
bool    FiboChrono =  true;                                         // true = Fibonacci, false = chrono clock display
byte    NormalExtremeUltimate = 0;                                  // 0 = Normal, 1 = Extreme, 2 = Ultimate display of colours                
byte    bits[CLOCK_PIXELS+1];                                       // Stores the hours=1 and minutes = 2 to set in LEDsetTime(byte hours, byte minutes)
byte    BitSet[CLOCK_PIXELS+1];                                     // For calculation of the bits to set

//------------------------------------------------------------------------------
const uint32_t black    = 0x000000, darkorange    = 0xFF8C00, red        = 0xFF0000, chartreuse   = 0x7FFF00;
const uint32_t brown    = 0x503000, cyberyellow   = 0xFFD300, orange     = 0xFF8000; 
const uint32_t yellow   = 0xFFFF00, cadmiumyellow = 0xFFF600, dyellow    = 0xFFAA00, chromeyellow = 0xFFA700;
const uint32_t green    = 0x00FF00, brightgreen   = 0x66FF00, apple      = 0x80FF00, grass        = 0x00FF80;  
const uint32_t amaranth = 0xE52B50, edamaranth    = 0xFF0050, amber      = 0xFF7E00;
const uint32_t marine   = 0x0080FF, darkviolet    = 0x800080, pink       = 0xFF0080, purple       = 0xFF00FF; 
const uint32_t blue     = 0x0000FF, cerulean      = 0x007BA7, sky        = 0x00FFFF, capri        = 0x00BFFF;
const uint32_t edviolet = 0X7500BC, frenchviolet  = 0X8806CE, coquelicot = 0xFF3800;
const uint32_t greenblue= 0x00F2A0, hotmagenta    = 0xFF00BF, dodgerblue = 0x0073FF, screamingreen= 0x70FF70;
      uint32_t white    = 0xFFFFFF, lgray         = 0x666666, wgray      = 0xAAAAAA;
      uint32_t gray     = 0x333333, dgray         = 0x222222;  
const uint32_t colors[][5] =                                        // The colour palettes
   {//off   hours   minutes both;
   { white, red   , yellow , blue  , green },  // #0 Mondriaan
   { white, red   , dyellow, blue  , green },  // #1 Mondriaan1 
   { white, red   , green  , blue  , green },  // #2 RGB  
   { white, apple , green  , grass , blue  },  // #3 Greens
   { white, red   , grass  , purple, green },  // #4 Pastel                                                                 
   { white, orange, green  , marine, blue  },  // #5 Modern
   { white, sky   , purple , blue  , green },  // #6 Cold
   { white, red   , yellow , orange, green },  // #7 Warm
   { wgray, red   , dyellow, blue  , white },  // #8 Mondriaan 2
   { wgray, red   , yellow,  blue  , white }}; // #9 Mondriaan 3  
//------------------------------------------------------------------------------
// LED
//------------------------------------------------------------------------------
                      #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
EdSoftLED LEDstrip ;//    = EdSoftLED();                                                         // Use EdSoftLED with ESP32 compiler V3.x.x. Neopixel crashes
EdSoftLED LED6812strip = EdSoftLED(NUM_LEDS, LED_PIN, SK6812WRGB);
EdSoftLED LED2812strip = EdSoftLED(NUM_LEDS, LED_PIN, WS2812RGB);
                      #else
// LED_PIN = 8;  // bug in Neopixel library. Does not translate D5 to GPIO 8
Adafruit_NeoPixel LEDstrip;
Adafruit_NeoPixel LED6812strip = Adafruit_NeoPixel(NUM_LEDS, 8, NEO_GRBW + NEO_KHZ800); // NEO_RGBW
Adafruit_NeoPixel LED2812strip = Adafruit_NeoPixel(NUM_LEDS, 8, NEO_GRB  + NEO_KHZ800); // NEO_RGB NEO_GRB
                      #endif
//--------------------------------------------
// DS3231 CLOCK MODULE
//--------------------------------------------
#define DS3231_I2C_ADDRESS          0x68
#define DS3231_TEMPERATURE_MSB      0x11
#define DS3231_TEMPERATURE_LSB      0x12

RTC_DS3231 RTCklok; 
DateTime Inow;
bool DS3231Installed = false;                                                                     // True if the DS3231 is detected
//------------------------------------------------------------------------------
// KY-040 ROTARY
//------------------------------------------------------------------------------                       
Encoder myEnc(encoderPinA, encoderPinB);                                                      // Use digital pin  for encoder    
long     Looptime          = 0;
byte     RotaryPress       = 0;                                                               // Keeps track display choice and how often the rotary is pressed.
uint32_t RotaryPressTimer  = 0;
byte     NoofRotaryPressed = 0;

//--------------------------------------------                                                //
// One-wire keypad
//--------------------------------------------
 bool     ChangeTime           = false;
 bool     ChangeLightIntensity = false;
 bool     KeyInputactivated    = false;
 uint64_t KeyLooptime          = 0;
 String   KeypadString         ="";

//------------------------------------------------------------------------------
// LDR PHOTOCELL
//------------------------------------------------------------------------------
//                                                                                            //
const byte SLOPEBRIGHTNESS    = 32;                                                           // Steepness of with luminosity of the LED increases
const int  MAXBRIGHTNESS      = 255;                                                          // Maximum value in bits  for luminosity of the LEDs (1 - 255)
const byte LOWBRIGHTNESS      = 5;                                                            // Lower limit in bits of Brightness ( 0 - 255)   
byte       TestLDR            = 0;                                                            // If true LDR inf0 is printed every second in serial monitor
int        OutPhotocell;                                                                      // stores reading of photocell;
int        MinPhotocell       = 999;                                                          // stores minimum reading of photocell;
int        MaxPhotocell       = 1;                                                            // stores maximum reading of photocell;
uint32_t   SumLDRreadshour    = 0;
uint32_t   NoofLDRreadshour   = 0;

//--------------------------------------------
// CLOCK initialysations
//--------------------------------------------                                 

static uint32_t msTick;                                                                       // Number of millisecond ticks since we last incremented the second counter
byte      lastminute = 0, lasthour = 0, lastday = 0, sayhour = 0;
bool      Demo                 = false;
bool      Zelftest             = false;
bool      Is                   = true;                                                        // toggle of displaying Is or Was
bool      ZegUur               = true;                                                        // Say or not say Uur in NL clock
struct    tm timeinfo;                                                                        // storage of time 

//--------------------------------------------                                                //
// BLE   //#include <NimBLEDevice.h>
//--------------------------------------------
BLEServer *pServer      = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected    = false;
bool oldDeviceConnected = false;
std::string ReceivedMessageBLE;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"                         // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

//----------------------------------------
// WIFI and webserver
//----------------------------------------
bool SSIDandNetworkfound = false;                                                             // True if Mem.SSID and a found WIFI.scan'ed network are identical
bool WIFIwasConnected = false;                                                                   // Is wIFI connected?
AsyncWebServer server(80);                                                                    // For OTA Over the air uploading
#include "Webpage.h"

//--------------------------------------------                                                //
// NTP
//----------------------------------------
boolean syncEventTriggered = false;                                                           // True if a time even has been triggered
NTPEvent_t ntpEvent;                                                                          // Last triggered event
//----------------------------------------
// Common
//----------------------------------------
 
#define   MAXTEXT 255
char      sptext[MAXTEXT];                                                                   // For common print use 
bool     LEDsAreOff         = false;                                                          // If true LEDs are off except time display
bool     NoTextInLeds       = false;                                                          // Flag to control printing of the text in function ColorLeds()
int      Previous_LDR_read  = 512;                                                            // The actual reading from the LDR + 4x this value /5
int      ProgressLedNr      = 0;                                                              // Startup Progress LED number

bool      SerialConnected   = true;   
uint16_t  MilliSecondValue  = 10;                                                            // The duration of a second  minus 1 ms. Used in Demo mode
uint64_t  Loopcounter       = 0;
struct    EEPROMstorage {                                                                    // Data storage in EEPROM to maintain them after power loss
  byte DisplayChoice    = 0;
  byte TurnOffLEDsAtHH  = 0;
  byte TurnOnLEDsAtHH   = 0;
  byte LanguageChoice   = 0;
  byte LightReducer     = 0;
  int  LowerBrightness  = 0;
  int  UpperBrightness  = 0;
  int  NVRAMmem[24];                                                                          // LDR readings
  byte BLEOn            = 1;
  byte NTPOn            = 1;
  byte WIFIOn           = 1;  
  byte StatusLEDOn      = 1;
  int  ReconnectWIFI    = 0;                                                                  // No of times WIFI reconnected 
  byte DCF77On          = 0;
  byte UseRotary        = 0;                                                                  // Use the rotary coding
  byte UseDS3231        = 0;                                                                  // Use the DS3231 time module 
  byte LEDstrip         = 0;                                                                  // 0 = SK6812 LED strip. 1 = WS2812 LED strip
  byte FiboChrono       = 0;                                                                  // true = Fibonacci, false = chrono clock display
  byte NoExUl           = 0;                                                                  // 0 = Normal, 1 = Extreme, 2 = Ultimate display of colours
  int  IntFuture1       = 0;                                                                  // For future use
  int  IntFuture2       = 0;                                                                  // For future use
  int  IntFuture3       = 0;                                                                  // For future use
  int  IntFuture4       = 0;                                                                  // For future use   
  byte UseBLELongString = 0;                                                                  // Send strings longer than 20 bytes per message. Possible in IOS app BLEserial Pro 
  uint32_t OwnColour    = 0;                                                                  // Self defined colour for clock display
  uint32_t DimmedLetter = 0;
  uint32_t BackGround   = 0;
  char SSID[30];                                                                              // 
  char Password[40];                                                                          // 
  char BLEbroadcastName[30];                                                                  // Name of the BLE beacon
  char Timezone[50];
  int  Checksum        = 0;
}  Mem; 
//--------------------------------------------                                                //
// Menu
//0        1         2         3         4
//1234567890123456789012345678901234567890----  
 char menu[][40] = {
 "A SSID B Password C BLE beacon name",
 "D Date (D15012021) T Time (T132145)",
//    "E Normal, Extreme or Ultimate mode",
//    "F Fibonacci or Chrono display",
 "E Timezone  (E<-02>2 or E<+01>-1)",
 "F Fibonacci or Chrono display",
 "G Scan WIFI networks",
 "H Toggle use rotary encoder", 
 "I To print this Info menu",
 "J Toggle use DS3231 RTC module",
 "K LDR reads/sec toggle On/Off", 
 "N Display off between Nhhhh (N2208)",
 "O Display toggle On/Off",
 "P Status LED toggle On/Off", 
 "Q Display colour choice (Q for options)",
 "R Reset settings @ = Reset MCU",
 "U Demo mode U0-U999 in msec (U100)",
 "V Normal, Extreme or Ultimate mode",
 "--Light intensity settings (1-250)--",
 "S Slope, L Min, M Max  (S80 L5 M200)",
 "W WIFI, X NTP&, CCC BLE, + Fast BLE",
 "# Self test, ! See RTC, & Update RTC",
 "Ed Nieuwenhuys November 2024" };
 
//  -------------------------------------   End Definitions  ---------------------------------------

//--------------------------------------------                                                //
// ARDUINO Setup
//--------------------------------------------
void setup() 
{
 Serial.begin(115200);                                                                        // Setup the serial port to 115200 baud //
 SetStatusLED(10,0,0);                                                                        // Set the status LED to red
 Wire.begin();
 Previous_LDR_read = ReadLDR();                                                               // Set the initial LDR reading
 int32_t Tick = millis(); 
  while (!Serial)  
 {if ((millis() - Tick) >5000) break;}  Tekstprintln("Serial started");                       // Wait max 5 seconds until serial port is started   
 InitStorage();                         Tekstprintln("Stored settings loaded");               // Load settings from storage and check validity  
 StartLeds();                           Tekstprintln("LED strip started");                    // LED RainbowCycle 
 InitRotaryMod();                       Tekstprintln("Rotary available");                     // Start the Rotary encoder
 InitDS3231Mod();                       Tekstprintln("DS3231 RTC software started");          // Start the DS3231 RTC-module even if not installed. It can be turned it on later in the menu
 ColorLed(ProgressLedNr++,frenchviolet); ShowLeds();
 if(Mem.BLEOn) { StartBLEService();     Tekstprintln("BLE started"); }                        // Start BLE service
 SetStatusLED(0,0,10);                                                                        // Set the status LED to blue 
 ColorLed(ProgressLedNr++,frenchviolet); ShowLeds();
 if(Mem.WIFIOn &&  CheckforWIFINetwork() ){StartWIFI_NTP(); Tekstprintln("WIFI started");}    // Start WIFI and optional NTP if Mem.WIFIOn = 1 
 ColorLed(ProgressLedNr++,frenchviolet); ShowLeds();
 GetTijd(true);                                                                               // Get the time and print it
 Tekstprintln(""); 
 SWversion();                                                                                 // Print the menu + version 
 Displaytime();                                                                               // Print the tekst time in the display 
 Tekstprintln("");
 SetStatusLED(10,0,0);                                                                        // Set the status LED to red                                  
 msTick = millis();                                                                           // start the seconds loop counter
}
//--------------------------------------------                                                //
// ARDUINO Loop
//--------------------------------------------
void loop() 
{
 Loopcounter++;
 if (Demo)         Demomode();                                                                // 
 else              EverySecondCheck();                                                        // Let the second led tick and run the clock program
 CheckDevices();
}
//--------------------------------------------                                                //
// COMMON Check connected input devices
//--------------------------------------------
void CheckDevices(void)
{
 CheckBLE();                                                                                  // Something with BLE to do?
 SerialCheck();                                                                               // Check serial port every second 
 ElegantOTA.loop();                                                                           // For Over The Air updates This loop block is necessary for ElegantOTA to handle reboot after OTA update.
  
 if (Mem.UseRotary) RotaryEncoderCheck(); 

                                  #ifdef ONEWIREKEYPAD3x4   
 OnewireKeypad3x4Check(); 
                                  #endif  //ONEWIREKEYPAD3x4
                                  #ifdef ONEWIREKEYPAD3x1   
 OnewireKeypad3x1Check(); 
                                  #endif  //ONEWIREKEYPAD3x1
}
//--------------------------------------------                                                //
// COMMON Update routine done every second
//--------------------------------------------
void EverySecondCheck(void)
{
 static int Toggle = 0;
 uint32_t msLeap = millis() - msTick;                                                         // 
 if (msLeap >999)                                                                             // Every second enter the loop
 {
  msTick = millis();
  GetTijd(false);                                                                             // Get the time for the seconds 
  Toggle = 1-Toggle;                                                                          // Used to turn On or Off Leds
  UpdateStatusLEDs(Toggle);
  DimLeds(TestLDR);                                                                           // Every second an intensity check and update from LDR reading 
  if (timeinfo.tm_min != lastminute) EveryMinuteUpdate();                                     // Enter the every minute routine after one minute; 
  Loopcounter=0;
 }  
}

//--------------------------------------------                                                //
// COMMON Update routine done every minute
//-------------------------------------------- 
void EveryMinuteUpdate(void)
{   
 lastminute = timeinfo.tm_min;  
 if(Mem.WIFIOn)                                                                               // If WIFI switch is On.
   {
    if(WiFi.localIP()[0] == 0)
       {
        if(WIFIwasConnected)  WiFi.reconnect();                                                 // If connection lost and WIFI is used reconnect
        if(CheckforWIFINetwork(false) && !WIFIwasConnected) StartWIFI_NTP();                    // If there was no WIFI at start up start a WIFI connection 
        if(WiFi.localIP()[0] != 0) 
          {
          sprintf(sptext, "Reconnected to IP address: %d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
          Tekstprintln(sptext);
          }
        }
   }
 GetTijd(false);
 Displaytime();                                                                               // PrintRTCTime();
 DimLeds(true);  
 if(timeinfo.tm_hour != lasthour) EveryHourUpdate(); 
}
//--------------------------------------------                                                //
// COMMON Update routine done every hour
//--------------------------------------------
void EveryHourUpdate(void)
{
 lasthour = timeinfo.tm_hour;
 if (!Mem.StatusLEDOn) SetStatusLED(0,0,0);                                                   // If for some reason the LEDs are ON and after a MCU restart turn them off.  
 if( (timeinfo.tm_hour == Mem.TurnOffLEDsAtHH) && (Mem.TurnOffLEDsAtHH != Mem.TurnOnLEDsAtHH))
       { LEDsAreOff = true;  ClearScreen(); }                                                 // Is it time to turn off the LEDs?
 if(timeinfo.tm_hour == Mem.TurnOnLEDsAtHH)
   { 
    LEDsAreOff = false;
    lastminute = 99;                                                                          // Force a minute update
    Displaytime();  
   }
 Mem.NVRAMmem[lasthour] =(byte)((SumLDRreadshour / NoofLDRreadshour?NoofLDRreadshour:1));     // Update the average LDR readings per hour
 SumLDRreadshour  = 0;
 NoofLDRreadshour = 0;
 if (timeinfo.tm_mday != lastday) EveryDayUpdate();  
}
//--------------------------------------------                                                //
// COMMON Update routine done every day
//--------------------------------------------
void EveryDayUpdate(void)
{
 if(timeinfo.tm_mday != lastday) 
   {
    lastday           = timeinfo.tm_mday; 
    Previous_LDR_read = ReadLDR();                                                            // to have a start value
    MinPhotocell      = Previous_LDR_read;                                                    // Stores minimum reading of photocell;
    MaxPhotocell      = Previous_LDR_read;                                                    // Stores maximum reading of photocell;
//    Mem.ReconnectWIFI = 0;                                                                    // Reset WIFI reconnection counter     
//    StoreStructInFlashMemory();                                                             // 
    }
}

//--------------------------------------------                                                //
// COMMON Update routine for the status LEDs
//-------------------------------------------- 
void UpdateStatusLEDs(int Toggle)
{
 if(Mem.StatusLEDOn)   
   {
    SetStatusLED((Toggle && WiFi.localIP()[0]==0) * 20, 
                 (Toggle && WiFi.localIP()[0]!=0) * 20 , 
                 (Toggle && deviceConnected) * 20);
    SetPCBLED09( Toggle * 10);                                                                // Left LED
    SetPCBLED10((1-Toggle) * 10);                                                             // Right LED
    SetNanoLED13((1-Toggle) * 50);                                                            // LED on ESP32 board
   }
   else
   {
    SetStatusLED(0, 0, 0); 
    SetPCBLED09(0);                                                                           //
    SetPCBLED10(0);                                                                           //
    SetNanoLED13(0);      
   }
}
//--------------------------------------------                                                //
// COMMON Control the RGB LEDs on the Nano ESP32
// Analog range 0 - 512. 0 is LED On max intensity
// 512 is LED off. Therefore the value is subtracted from 512 
//--------------------------------------------
void SetStatusLED(int Red, int Green, int Blue)
{
 analogWrite(LED_RED,   512 - Red);                                                                 // !Red (not Red) because 1 or HIGH is LED off
 analogWrite(LED_GREEN, 512 - Green);
 analogWrite(LED_BLUE,  512 - Blue);
}
//--------------------------------------------                                                //
// COMMON Control orange LED D13 on the Arduino 
//--------------------------------------------
void SetNanoLED13(int intensity) {analogWrite(secondsPin, intensity);}
//--------------------------------------------                                                //
// COMMON Control the RGB LED on the PCB
//--------------------------------------------
void SetPCBLED09(int intensity) {analogWrite(PCB_LED_D09, intensity);}
void SetPCBLED10(int intensity) {analogWrite(PCB_LED_D10, intensity);}

//--------------------------------------------                                                //
// COMMON check for serial input
//--------------------------------------------
void SerialCheck(void)
{
 String SerialString; 
 while (Serial.available())
    { 
     char c = Serial.read();                                                                  // Serial.write(c);
     if (c>31 && c<128) SerialString += c;                                                    // Allow input from Space - Del
     else c = 0;                                                                              // Delete a CR
    }
 if (SerialString.length()>0) 
    {
     ReworkInputString(SerialString);                                                         // Rework ReworkInputString();
     SerialString = "";
    }
}

//--------------------------------------------                                                //
// COMMON Reset to default settings
//--------------------------------------------
void Reset(void)
{
 Mem.Checksum         = 25065;                                                                //
 Mem.DisplayChoice    = DEFAULTCOLOUR;                                                        // Default colour scheme 
 Mem.OwnColour        = green;                                                                // Own designed colour.
 Mem.DimmedLetter     = dgray;
 Mem.BackGround       = black; 
 Mem.LanguageChoice   = 0;                                                                    // 0 = NL, 1 = UK, 2 = DE, 3 = FR, 4 = Wheel
 Mem.LightReducer     = SLOPEBRIGHTNESS;                                                      // Factor to dim ledintensity with. Between 0.1 and 1 in steps of 0.05
 Mem.UpperBrightness  = MAXBRIGHTNESS;                                                        // Upper limit of Brightness in bits ( 1 - 1023)
 Mem.LowerBrightness  = LOWBRIGHTNESS;                                                        // Lower limit of Brightness in bits ( 0 - 255)
 Mem.TurnOffLEDsAtHH  = 0;                                                                    // Display Off at nn hour
 Mem.TurnOnLEDsAtHH   = 0;                                                                    // Display On at nn hour Not Used
 Mem.BLEOn            = 1;                                                                    // default BLE On
 Mem.UseBLELongString = 0;                                                                    // Default off. works only with iPhone/iPad with BLEserial app
 Mem.NTPOn            = 0;                                                                    // NTP default off
 Mem.WIFIOn           = 0;                                                                    // WIFI default off
 Mem.ReconnectWIFI    = 0;                                                                    // Correct time if necessary in seconds
 //Mem.UseRotary      = 0;    // Do not erase this setting with a reset                       // Use the rotary coding
 Mem.DCF77On          = 0;                                                                    // Default off
 Mem.UseDS3231        = 0;                                                                    // Default off
 //Mem.LEDstrip       = 0;    // Do not erase this setting with a reset                       // 0 = SK6812, 1=WS2812
 Mem.FiboChrono       = true;                                                                 // true = Fibonacci, false = chrono clock display
 Mem.NoExUl           = 0; 
 Previous_LDR_read    = ReadLDR();                                                            // Read LDR to have a start value. max = 4096/8 = 255
 MinPhotocell         = Previous_LDR_read;                                                    // Stores minimum reading of photocell;
 MaxPhotocell         = Previous_LDR_read;                                                    // Stores maximum reading of photocell;                                            
 TestLDR              = 0;                                                                    // If true LDR display is printed every second
// WIFIwasConnected     = false;
 strcpy(Mem.SSID,"");                                                                         // Default SSID
 strcpy(Mem.Password,"");                                                                     // Default password
 strcpy(Mem.BLEbroadcastName,"FiboESP32");
 strcpy(Mem.Timezone,"CET-1CEST,M3.5.0,M10.5.0/3");                                           // Central Europe, Amsterdam, Berlin etc.                                                         // WIFI On  

 Tekstprintln("**** Reset of preferences ****"); 
 StoreStructInFlashMemory();                                                                  // Update Mem struct       
 GetTijd(false);                                                                                  // Get the time and store it in the proper variables
 SWversion();                                                                                 // Display the version number of the software
 Displaytime();
}
//--------------------------------------------                                                //
// COMMON common print routines
//--------------------------------------------
void Tekstprint(char const *tekst)    { if(Serial) Serial.print(tekst);  SendMessageBLE(tekst);sptext[0]=0;   } 
void Tekstprintln(char const *tekst)  { sprintf(sptext,"%s\n",tekst); Tekstprint(sptext);  }
void TekstSprint(char const *tekst)   { printf(tekst); sptext[0]=0;}                          // printing for Debugging purposes in serial monitor 
void TekstSprintln(char const *tekst) { sprintf(sptext,"%s\n",tekst); TekstSprint(sptext); }
//--------------------------------------------                                                //
//  COMMON String upper
//--------------------------------------------
void to_upper(char* string)
{
 const char OFFSET = 'a' - 'A';
 while (*string) {(*string >= 'a' && *string <= 'z') ? *string -= OFFSET : *string;   string++;  }
}
//--------------------------------------------                                                //
// COMMON Constrain a string with integers
// The value between the first and last character in a string is returned between the low and up bounderies
//--------------------------------------------
int SConstrainInt(String s,byte first,byte last,int low,int up){return constrain(s.substring(first, last).toInt(), low, up);}
int SConstrainInt(String s,byte first,          int low,int up){return constrain(s.substring(first).toInt(), low, up);}
//--------------------------------------------                                                //
// COMMON Init and check contents of EEPROM
//--------------------------------------------
void InitStorage(void)
{
 // if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){ Tekstprintln("Card Mount Failed");   return;}
 // else Tekstprintln("SPIFFS mounted"); 

 GetStructFromFlashMemory();
 if( Mem.Checksum != 25065)
   {
    sprintf(sptext,"Checksum (25065) invalid: %d\n Resetting to default values",Mem.Checksum); 
    Tekstprintln(sptext); 
    Reset();                                                                                  // If the checksum is NOK the Settings were not set
   }
 Mem.LanguageChoice  = _min(Mem.LanguageChoice, 4);                                           // Constrain the value to valid ranges 
 Mem.DisplayChoice   = _min(Mem.DisplayChoice, ANALOOG);                                      // Constrain the value to valid ranges 
 if(Mem.OwnColour == 0) Mem.OwnColour = green;                                                // If memory is empty cq black colour then store default value, green  
 Mem.LightReducer    = constrain(Mem.LightReducer,1,250);                                     // 
 Mem.LowerBrightness = constrain(Mem.LowerBrightness, 1, 250);                                // 
 Mem.UpperBrightness = _min(Mem.UpperBrightness, 255); 
 if(strlen(Mem.Password)<5 || strlen(Mem.SSID)<3)     Mem.WIFIOn = Mem.NTPOn = 0;             // If ssid or password invalid turn WIFI/NTP off
 if(Mem.LEDstrip  > 1) Mem.LEDstrip = 0;                                                      // Default SK6812
 StoreStructInFlashMemory();
}
//--------------------------------------------                                                //
// COMMON Store mem.struct in FlashStorage or SD
// Preferences.h  
//--------------------------------------------
void StoreStructInFlashMemory(void)
{
  FLASHSTOR.begin("Mem",false);       //  delay(100);
  FLASHSTOR.putBytes("Mem", &Mem , sizeof(Mem) );
  FLASHSTOR.end();          
  
// Can be used as alternative
//  SPIFFS
//  File myFile = SPIFFS.open("/MemStore.txt", FILE_WRITE);
//  myFile.write((byte *)&Mem, sizeof(Mem));
//  myFile.close();
 }
//--------------------------------------------                                                //
// COMMON Get data from FlashStorage
// Preferences.h
//--------------------------------------------
void GetStructFromFlashMemory(void)
{
 FLASHSTOR.begin("Mem", false);
 FLASHSTOR.getBytes("Mem", &Mem, sizeof(Mem) );
 FLASHSTOR.end(); 

// Can be used as alternative if no SD card
//  File myFile = SPIFFS.open("/MemStore.txt");  FILE_WRITE); myFile.read((byte *)&Mem, sizeof(Mem));  myFile.close();

 sprintf(sptext,"Mem.Checksum = %d",Mem.Checksum);Tekstprintln(sptext); 
}

//--------------------------------------------                                                //
// COMMON Version info
//--------------------------------------------
void SWversion(void) 
{ 
 #define FILENAAM (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
 PrintLine(35);
 for (uint8_t i = 0; i < sizeof(menu) / sizeof(menu[0]); Tekstprintln(menu[i++]));
 PrintLine(35);
 byte dp = Mem.DisplayChoice;
 sprintf(sptext,"Display off between: %02dh - %02dh",Mem.TurnOffLEDsAtHH, Mem.TurnOnLEDsAtHH);  Tekstprintln(sptext);
 sprintf(sptext,"Display choice: %s",dp==0?"Yellow":dp==1?"Hourly":dp==2?"White":
              dp==3?"All Own":dp==4?"Own":dp==5?"Wheel":dp==6?"Digital":dp==7?"Analog":"NOP");  Tekstprintln(sptext);
 sprintf(sptext,"Slope: %d     Min: %d     Max: %d ",
                 Mem.LightReducer, Mem.LowerBrightness,Mem.UpperBrightness);                    Tekstprintln(sptext);
 sprintf(sptext,"SSID: %s", Mem.SSID);                                                          Tekstprintln(sptext); 
// sprintf(sptext,"Password: %s", Mem.Password);                                                Tekstprintln(sptext);
 sprintf(sptext,"BLE name: %s", Mem.BLEbroadcastName);                                          Tekstprintln(sptext);
 sprintf(sptext,"IP-address: %d.%d.%d.%d (/update)", WiFi.localIP()[0], WiFi.localIP()[1], 
                                                     WiFi.localIP()[2], WiFi.localIP()[3] );    Tekstprintln(sptext);
 sprintf(sptext,"Timezone:%s", Mem.Timezone);                                                   Tekstprintln(sptext); 
 sprintf(sptext,"%s %s %s %s", Mem.WIFIOn?"WIFI=On":"WIFI=Off", 
                               Mem.NTPOn? "NTP=On":"NTP=Off",
                               Mem.BLEOn? "BLE=On":"BLE=Off",
                               Mem.UseBLELongString? "FastBLE=On":"FastBLE=Off" );              Tekstprintln(sptext);
 sprintf(sptext,"LED strip: %s (Send %% to switch)", 
                 Mem.LEDstrip==0?"SK6812":Mem.LEDstrip==1?"WS2812":"NOP");                      Tekstprintln(sptext);        
 sprintf(sptext,"Software: %s",FILENAAM);                                                       Tekstprintln(sptext);  // VERSION);
 sprintf(sptext,"ESP32 Arduino core version: %d.%d.%d", 
          ESP_ARDUINO_VERSION_MAJOR,ESP_ARDUINO_VERSION_MINOR,ESP_ARDUINO_VERSION_PATCH);       Tekstprintln(sptext);
 PrintRTCTime(); Tekstprintln(""); 
 PrintLine(35);
}
//--------------------------------------------                                                //
// COMMON PrintLine
//--------------------------------------------
void PrintLine(byte Lengte)
{
 for(int n=0; n<Lengte; n++) sptext[n]='_';
 sptext[Lengte] = 0;
 Tekstprintln(sptext);
}

//--------------------------------------------                                                //
//  COMMON Input from Bluetooth or Serial
//--------------------------------------------
void ReworkInputString(String InputString)
{
 if(InputString.length()> 40){Serial.printf("Input string too long (max40)\n"); return;}      // If garbage return
 for (int n=0; n<InputString.length()+1; n++)                                                 // remove CR and LF
       if (InputString[n] == 10 || InputString[n]==13) InputString.remove(n,1);
 sptext[0] = 0;                                                                               // Empty the sptext string
 
 if(InputString[0] > 31 && InputString[0] <127)                                               // Does the string start with a letter?
  { 
  switch (InputString[0])
   {
    case 'A':
    case 'a': 
            if (InputString.length() >4 && InputString.length() <30)
            {
             InputString.substring(1).toCharArray(Mem.SSID,InputString.length());
             sprintf(sptext,"SSID set: %s", Mem.SSID);  
            }
            else sprintf(sptext,"**** Length fault. Use between 4 and 30 characters ****");
            break;
    case 'B':
    case 'b': 
             if (InputString.equals("BBBB"))                                                 // 
               {   
                sprintf(sptext,"%s,**** Length fault. Use between 5 and 40 characters ****",Mem.Password);
                break;
               } 
             if (InputString.length() >4 && InputString.length() <40)
              {  
               InputString.substring(1).toCharArray(Mem.Password,InputString.length());
               sprintf(sptext,"Password set: %s\n Enter @ to reset ESP32 and connect to WIFI and NTP\n WIFI and NTP are turned ON", Mem.Password); 
               Mem.NTPOn        = 1;                                                          // NTP On
               Mem.WIFIOn       = 1;                                                          // WIFI On  
              }
             else sprintf(sptext,"**** Length fault. Use between 5 and 40 characters ****");
             break;   
    case 'C':
    case 'c': 
             if (InputString.equalsIgnoreCase("ccc"))                                         // Toggle BLE ON or OFF
               {   
                Mem.BLEOn = 1 - Mem.BLEOn; 
                sprintf(sptext,"BLE is %s after restart", Mem.BLEOn?"ON":"OFF" );
               }    
             if (InputString.length() >4 && InputString.length() <30)
               {  
                InputString.substring(1).toCharArray(Mem.BLEbroadcastName,InputString.length());
                sprintf(sptext,"BLE broadcast name set: %s", Mem.BLEbroadcastName); 
                Mem.BLEOn = 1;                                                                // BLE On
              }
            else sprintf(sptext,"**** Length fault. Use between 4 and 30 characters ****");
            break;      
    case 'D':
    case 'd':  
             if (InputString.length() == 9 )
               {
                timeinfo.tm_mday = (int) SConstrainInt(InputString,1,3,0,31);
                timeinfo.tm_mon  = (int) SConstrainInt(InputString,3,5,0,12) - 1; 
                timeinfo.tm_year = (int) SConstrainInt(InputString,5,9,2000,9999) - 1900;
                if (DS3231Installed)
                  {
                   sprintf(sptext,"Time set in external RTC module");  
                   SetDS3231Time();
                   PrintDS3231Time();
                  }
                else sprintf(sptext,"No external RTC module detected");
                } 
              else sprintf(sptext,"****\nLength fault. Enter Dddmmyyyy\n****");
              break;
    case 'E':
    case 'e':  
             if (InputString.length() >2 )
              {  
               InputString.substring(1).toCharArray(Mem.Timezone,InputString.length());
               sprintf(sptext,"Timezone set: %s", Mem.Timezone); 
              }
              else sprintf(sptext,"**** Length fault. Use more than 2 characters ****");
              break;  
   case 'V':
   case 'v':
            if(InputString.length() == 1)
              {
               if (++Mem.NoExUl>2) Mem.NoExUl=0;
               sprintf(sptext,"Only working in chrono modus\nDisplay is %s", Mem.NoExUl==1?"Extreme":Mem.NoExUl==2?"Ultimate":"Normal" );
              }
            break;    
   case 'F':
   case 'f':
            if(InputString.length() == 1)
              {
               Mem.FiboChrono = !Mem.FiboChrono;
               sprintf(sptext,"Display is %s", Mem.FiboChrono?"Fibonacci":"Chrono" );
              }
            break;
    case 'G':
    case 'g':
             if (InputString.length() == 1) 
               {
                ScanWIFI(); 
                if(WIFIwasConnected)  WiFi.reconnect();
               }
             else sprintf(sptext,"**** Length fault. Enter G ****");            
             break;
    case 'H':
    case 'h':
             if (InputString.length() == 1)
               {   
                Mem.UseRotary = 1 - Mem.UseRotary; 
                sprintf(sptext,"Use rotary encoder is %s", Mem.UseRotary?"ON":"OFF" );
               }                                
             else sprintf(sptext,"**** Length fault. Enter H ****");
             break;          
    case 'I':
    case 'i': 
            SWversion();
            break;
    case 'J':
    case 'j':
             if (InputString.length() == 1)
               {   
                Mem.UseDS3231 = 1 - Mem.UseDS3231; 
                Mem.NTPOn = (1 - Mem.UseDS3231);
                sprintf(sptext,"Use DS3231 is %s, WIFI is %s, NTP is %s", Mem.UseDS3231?"ON":"OFF",Mem.WIFIOn?"ON":"OFF",Mem.NTPOn?"ON":"OFF" );
               }                                
             else sprintf(sptext,"**** Length fault. Enter J ****");
             break; 
    case 'K':
    case 'k':
             TestLDR = 1 - TestLDR;                                                           // If TestLDR = 1 LDR reading is printed every second instead every 30s
             sprintf(sptext,"TestLDR: %s \nLDR reading, %%Out, loops per second and time",TestLDR? "On" : "Off");
             break;      
    case 'L':                                                                                 // Language to choose
    case 'l':
             if (InputString.length() > 1 &&  InputString.length() < 5)
               {      
                Mem.LowerBrightness = (byte) SConstrainInt(InputString,1,0,255);
                sprintf(sptext,"Lower brightness: %d bits",Mem.LowerBrightness);
               }
             else sprintf(sptext,"**** Input fault. \nEnter Lnnn where n between 1 and 255");               
             break;    
    case 'M':
    case 'm':   
            if (InputString.length() > 1 &&  InputString.length() < 5)
               {    
                Mem.UpperBrightness = SConstrainInt(InputString,1,1,255);
                sprintf(sptext,"Upper brightness changed to: %d bits",Mem.UpperBrightness);
               }
            else sprintf(sptext,"**** Input fault. \nEnter Mnnn where n between 1 and 255");
 
              break;  
    case 'N':
    case 'n':
             sprintf(sptext,"**** Length fault N. ****");  
             if (InputString.length() == 1 )         Mem.TurnOffLEDsAtHH = Mem.TurnOnLEDsAtHH = 0;
             if (InputString.length() == 5 )
              {
               Mem.TurnOffLEDsAtHH =(byte) InputString.substring(1,3).toInt(); 
               Mem.TurnOnLEDsAtHH  =(byte) InputString.substring(3,5).toInt(); 
              }
             Mem.TurnOffLEDsAtHH = _min(Mem.TurnOffLEDsAtHH, 23);
             Mem.TurnOnLEDsAtHH  = _min(Mem.TurnOnLEDsAtHH, 23); 
             sprintf(sptext,"Display is OFF between %2d:00 and %2d:00", Mem.TurnOffLEDsAtHH,Mem.TurnOnLEDsAtHH );
             break;
    case 'O':
    case 'o':
             sprintf(sptext,"**** Length fault O. ****");    
             if(InputString.length() == 1)
               {
                LEDsAreOff = !LEDsAreOff;
                sprintf(sptext,"Display is %s", LEDsAreOff?"OFF":"ON" );
                if(LEDsAreOff) { ClearScreen();}                                              // Turn the display off
                else 
                {
                  Tekstprintln(sptext); 
                  lastminute = 99;
                  Displaytime();                                                              // Turn the display on   
                }
               }
             break;                                                                   
    case 'P':
    case 'p':  
             sprintf(sptext,"**** Length fault P. ****");  
             if(InputString.length() == 1)
               {
                Mem.StatusLEDOn = !Mem.StatusLEDOn;
                UpdateStatusLEDs(0);
                sprintf(sptext,"StatusLEDs are %s", Mem.StatusLEDOn?"ON":"OFF" );               
               }
             break;        

    case 'q':
    case 'Q':   
             sprintf(sptext,"**** Length fault Q. ****"); 
             if (InputString.length() == 1 )
               {
             Tekstprintln("  Q0= Mondriaan1");
             Tekstprintln("  Q1= Mondriaan2");
             Tekstprintln("  Q2= RGB");
             Tekstprintln("  Q3= Greens");
             Tekstprintln("  Q4= Pastel");
             Tekstprintln("  Q5= Modern");
             Tekstprintln("  Q6= Cold");
             Tekstprintln("  Q7= Warm");
             Tekstprintln("  Q8= Mondriaan3");
             Tekstprint(  "  Q9= Mondriaan4");
             sptext[0]=0;
               }
             if (InputString.length() == 2 )
               {
                Mem.DisplayChoice = InputString.substring(1).toInt();
 //               if (Mem.DisplayChoice>9) Mem.DisplayChoice = 0;
                sprintf(sptext,"Palette: %d",Mem.DisplayChoice);
                lastminute = 99;                                                              // Force a minute update
               }    
             break;
    case 'R':
    case 'r':
             sprintf(sptext,"**** Length fault R. ****");       
             if (InputString.length() == 1)
               {   
                Reset();
                sprintf(sptext,"\nReset to default values: Done");
                Displaytime();                                                                // Turn on the display with proper time
               }                                
             break;      
    case 'S':                                                                                 // Slope. factor ( 0 - 1) to multiply brighness (0 - 255) with 
    case 's':
             sprintf(sptext,"**** Length fault S. ****");    
             if (InputString.length() > 1 && InputString.length() < 5)
               {                
                Mem.LightReducer = (byte) SConstrainInt(InputString,1,1,255);
                sprintf(sptext,"Slope brightness changed to: %d%%",Mem.LightReducer);
               }
             else sprintf(sptext,"**** Input fault. \nEnter Snnn where n between 1 and 255");
             break;                   
    case 'T':
    case 't':
//                                                                                            //
             if(InputString.length() == 7)  // T125500
               {
                timeinfo.tm_hour = (int) SConstrainInt(InputString,1,3,0,23);
                timeinfo.tm_min  = (int) SConstrainInt(InputString,3,5,0,59); 
                timeinfo.tm_sec  = (int) SConstrainInt(InputString,5,7,0,59);
                if (DS3231Installed)
                  {
                   sprintf(sptext,"Time set in external RTC module");  
                   SetDS3231Time();
                   PrintDS3231Time();
                  }
                else sprintf(sptext,"No external RTC module detected");
               } 
             else sprintf(sptext,"**** Length fault. Enter Thhmmss ****");
             break;            
    case 'U':                                                                                 // factor to multiply brighness (0 - 255) with 
    case 'u':
            sprintf(sptext,"**** Length fault U. Demo mode (Unnn or U) ****");
            if (InputString.length() == 1)
               {   
                Demo = false; 
                sprintf(sptext,"Demo mode: %s",Demo?"ON":"OFF"); 
               }
            if (InputString.length() >1 && InputString.length() < 5 )                        // time between 0 and 999
              {
                MilliSecondValue = InputString.substring(1,5).toInt();                
                Demo = true;                                                                 // Toggle Demo mode
                sprintf(sptext,"Demo mode: %s MillisecondTime=%d",Demo?"ON":"OFF", MilliSecondValue); 
              }
             break;     
    case 'W':
    case 'w':
             if (InputString.length() == 1)
               {   
                Mem.WIFIOn = 1 - Mem.WIFIOn; 
                Mem.ReconnectWIFI = 0;                                                       // Reset WIFI reconnection counter 
                Mem.NTPOn = Mem.WIFIOn;                                                      // If WIFI is off turn NTP also off
                sprintf(sptext,"WIFI is %s after restart", Mem.WIFIOn?"ON":"OFF" );
               }                                
             else sprintf(sptext,"**** Length fault. Enter W ****");
             break; 
    case 'X':
    case 'x':
             if (InputString.length() == 1)
               {   
                Mem.NTPOn = 1 - Mem.NTPOn; 
                sprintf(sptext,"NTP is %s after restart", Mem.NTPOn?"ON":"OFF" );
               }                                
             else sprintf(sptext,"**** Length fault. Enter X ****");
             break; 
    case 'Y':
    case 'y':
             sprintf(sptext,"**** Nothing done");     
             break;  
    case 'Z':
    case 'z':
             sprintf(sptext,"**** Nothing done");     
             break;  
//--------------------------------------------                                                //        
     case '!':                                                                                // Print the NTP, RTC and DS3231 time
             if (InputString.length() == 1)  PrintAllClockTimes();
             break;       
    case '@':
             if (InputString.length() == 1)
               {   
               Tekstprintln("\n*********\n ESP restarting\n*********\n");
                ESP.restart();   
               }                                
             else sprintf(sptext,"**** Length fault. Enter @ ****");
             break;     
    case '#':
             if (InputString.length() == 1)
               {
                Zelftest = 1 - Zelftest; 
                sprintf(sptext,"Zelftest: %s", Zelftest?"ON":"OFF" ); 
                Play_Lights();  //Selftest();   
                Zelftest = 1 - Zelftest; 
                sprintf(sptext,"Zelftest: %s", Zelftest?"ON":"OFF" ); 
                Displaytime();                                                                // Turn on the display with proper time
               }                                
             else sprintf(sptext,"**** Length fault. Enter # ****");
             break; 
    case '$':
             break; 
    case '%':
             if (InputString.length() == 1)
               {   
                Mem.LEDstrip = 1 - Mem.LEDstrip; 
                sprintf(sptext,"LED strip is %s after restart", Mem.LEDstrip?"WS2812":"SK6812" );
               }                                
             else sprintf(sptext,"**** Length fault. ****");
             break; 
    case '&':
             sprintf(sptext,"**** Length fault &. ****");                                     // Forced get NTP time and update the DS32RTC module
             if (InputString.length() == 1)
              {
               NTP.getTime();                                                                // Force a NTP time update  
               SetDS3231Time();
               SetRTCTime();    
               PrintAllClockTimes();
               } 
             break;
    case '+':
             if (InputString.length() == 1)
               {   
                Mem.UseBLELongString = 1 - Mem.UseBLELongString; 
                sprintf(sptext,"Fast BLE is %s", Mem.UseBLELongString?"ON":"OFF" );
               }                                
             else sprintf(sptext,"**** Length fault %. Enter + ****");
             break;                       
    case '0':
    case '1':
    case '2':        
             if (InputString.length() == 6)                                                    // For compatibility input with only the time digits
              {
               timeinfo.tm_hour = (int) SConstrainInt(InputString,0,2,0,23);
               timeinfo.tm_min  = (int) SConstrainInt(InputString,2,4,0,59); 
               timeinfo.tm_sec  = (int) SConstrainInt(InputString,4,6,0,59);
               if (DS3231Installed)
                 {
                  sprintf(sptext,"Time set in external RTC module");  
                  SetDS3231Time();
                  PrintDS3231Time();
                 }
               else sprintf(sptext,"No external RTC module detected");
               } 
    default: break;
    }
  }  
 Tekstprintln(sptext); 
 StoreStructInFlashMemory();                                                                  // Update EEPROM                                     
 InputString = "";
}


//--------------------------------------------                                                //
// LDR reading are between 0 and 255. 
// ESP32 analogue read is between 0 - 4096 --   is: 4096 / 8
//--------------------------------------------
int ReadLDR(void)
{
 return analogRead(PhotoCellPin)/16;
}

// --------------------Colour Clock Light functions -----------------------------------
//--------------------------------------------                                                //
//  LED Set color for LEDs in strip and print tekst
//---------------------------------------------
void ColorLeds(char const *Tekst, int FirstLed, int LastLed, uint32_t RGBWColor)
{ 
 Stripfill(RGBWColor, FirstLed, ++LastLed - FirstLed );                                        //
 if (!NoTextInLeds && strlen(Tekst) > 0 )
     {sprintf(sptext,"%s ",Tekst); Tekstprint(sptext); }                                      // Print the text  
}
//--------------------------------------------
//  LED Set color for one LED
//--------------------------------------------
void ColorLed(int Lednr, uint32_t RGBWColor)
{   
 Stripfill(RGBWColor, Lednr, 1 );
}
//--------------------------------------------                                                //
//  LED Clear display settings of the LED's
//--------------------------------------------
void LedsOff(void) 
{ 
 Stripfill(0, 0, NUM_LEDS );                                                                  // 
}
//--------------------------------------------                                                //
// LED Turn On and Off the LED's after 200 milliseconds
//--------------------------------------------
void Laatzien()
{ 
 ShowLeds();
 delay(300);
 LedsOff(); 
 CheckDevices();                                                                              // Check for input from input devices
}

//--------------------------------------------                                                //
//  LED Push data in LED strip to commit the changes
//--------------------------------------------
void ShowLeds(void)
{
 LEDstrip.show();
}
//--------------------------------------------                                                //
//  LED Set brighness of LEDs
//--------------------------------------------
void SetBrightnessLeds(byte Bright)
{
 LEDstrip.setBrightness(Bright);                                                              // Set brightness of LEDs   
 ShowLeds();
}
//--------------------------------------------
//  LED Fill the strip array for LEDFAB library
//--------------------------------------------
void Stripfill(uint32_t RGBWColor, int FirstLed, int NoofLEDs)
{   
 LEDstrip.fill(RGBWColor, FirstLed, NoofLEDs);
}
//--------------------------------------------
//  LED Strip Get Pixel Color 
//--------------------------------------------
uint32_t StripGetPixelColor(int Lednr)
{
return(LEDstrip.getPixelColor(Lednr));
}
//--------------------------------------------                                                //
//  LED convert HSV to RGB  h is from 0-360, s,v values are 0-1
//  r,g,b values are 0-255
//--------------------------------------------
uint32_t HSVToRGB(double H, double S, double V) 
{
  int i;
  double r, g, b, f, p, q, t;
  if (S == 0)  {r = V;  g = V;  b = V; }
  else
  {
    H >= 360 ? H = 0 : H /= 60;
    i = (int) H;
    f = H - i;
    p = V * (1.0 -  S);
    q = V * (1.0 - (S * f));
    t = V * (1.0 - (S * (1.0 - f)));
    switch (i) 
    {
     case 0:  r = V;  g = t;  b = p;  break;
     case 1:  r = q;  g = V;  b = p;  break;
     case 2:  r = p;  g = V;  b = t;  break;
     case 3:  r = p;  g = q;  b = V;  break;
     case 4:  r = t;  g = p;  b = V;  break;
     default: r = V;  g = p;  b = q;  break;
    }
  }
return FuncCRGBW((int)(r*255), (int)(g*255), (int)(b*255), 0 );                                // R, G, B, W 
}
//--------------------------------------------                                                //
//  LED function to make RGBW color
//--------------------------------------------
uint32_t FuncCRGBW( uint32_t Red, uint32_t Green, uint32_t Blue, uint32_t White)
{ 
 return ( (White<<24) + (Red << 16) + (Green << 8) + Blue );
}
//--------------------------------------------                                                //
//  LED functions to extract RGBW colors
//--------------------------------------------
 uint8_t Cwhite(uint32_t c) { return (c >> 24);}
 uint8_t Cred(  uint32_t c) { return (c >> 16);}
 uint8_t Cgreen(uint32_t c) { return (c >> 8); }
 uint8_t Cblue( uint32_t c) { return (c);      }

//--------------------------------------------                                                //
// CLOCK Say the time and load the LEDs 
// with the proper colour and intensity
// If ClearLEDstrip is true a new minute starts
//--------------------------------------------
void Displaytime(void)
{ 
  if (Mem.FiboChrono) LEDsetTime(timeinfo.tm_hour , timeinfo.tm_min);                         // Fibonacci display   
  else            MakeChronoList(timeinfo.tm_hour , timeinfo.tm_min, timeinfo.tm_sec);        // Chrono (clock display) display
}

//--------------------------------------------                                                //
//  LED Dim the leds measured by the LDR and print values
// LDR reading are between 0 and 255. The Brightness send to the LEDs is between 0 and 255
//--------------------------------------------
void DimLeds(bool print) 
{ 
  int LDRread = ReadLDR();                                                                    // ESP32 analoge read is between 0 - 4096, reduce it to 0-1024                                                                                                   
  int LDRavgread = (4 * Previous_LDR_read + LDRread ) / 5;                                    // Read lightsensor and avoid rapid light intensity changes
  Previous_LDR_read = LDRavgread;                                                             // by using the previous reads
  OutPhotocell = (uint32_t)((Mem.LightReducer * sqrt(255*LDRavgread))/100);                   // Linear --> hyperbolic with sqrt. Result is between 0-255
  MinPhotocell = min(MinPhotocell, LDRavgread);                                               // Lowest LDR measurement
  MaxPhotocell = max(MaxPhotocell, LDRavgread);                                               // Highest LDR measurement
  OutPhotocell = constrain(OutPhotocell, Mem.LowerBrightness, Mem.UpperBrightness);           // Keep result between lower and upper boundery en calc percentage
  SumLDRreadshour += LDRavgread;    NoofLDRreadshour++;                                       // For statistics LDR readings per hour
  if(print)
  {
  // sprintf(sptext,"LDR:%3d Avg:%3d (%3d-%3d) Out:%3d=%2d%% Loop(%ld) ",
  //      LDRread,LDRavgread,MinPhotocell,MaxPhotocell,OutPhotocell,(int)(OutPhotocell/2.55),Loopcounter);    
 if (Mem.UseDS3231)   sprintf(sptext,"LDR:%3d=%2d%% %5lld l/s %0.0fC ",
               LDRread,(int)(OutPhotocell*100/255),Loopcounter,RTCklok.getTemperature()); 
 else                 sprintf(sptext,"LDR:%3d=%2d%% %5lld l/s ",
               LDRread,(int)(OutPhotocell*100/255),Loopcounter);   
   Tekstprint(sptext);
   PrintTimeHMS();    
  }
 if(LEDsAreOff) OutPhotocell = 0;
 SetBrightnessLeds(OutPhotocell);     // values between 0 and 255
}
//--------------------------------------------                                                //
//  LED Turn On en Off the LED's
//--------------------------------------------
void Play_Lights()
{
 for(int i=0; i<NUM_LEDS; i++) { ColorLed(i,chromeyellow); delay(100); ShowLeds(); }
 WhiteOverRainbow(50, 50, 5 );
 WhiteOverRainbow(50,50, 5 );  // wait, whiteSpeed, whiteLength
 LedsOff();
}

//--------------------------------------------                                                //
//  DISPLAY
//  Clear the display
//--------------------------------------------
void ClearScreen(void)
{
 LedsOff();
}
//--------------------------------------------                                                //
//  LED Wheel
//  Input a value 0 to 255 to get a color value.
//  The colours are a transition r - g - b - back to r.
//--------------------------------------------
uint32_t Wheel(byte WheelPos) 
{
 WheelPos = 255 - WheelPos;
 if(WheelPos < 85)   { return FuncCRGBW( 255 - WheelPos * 3, 0, WheelPos * 3, 0);  }
 if(WheelPos < 170)  { WheelPos -= 85;  return FuncCRGBW( 0,  WheelPos * 3, 255 - WheelPos * 3, 0); }
 WheelPos -= 170;      
 return FuncCRGBW(WheelPos * 3, 255 - WheelPos * 3, 0, 0);
}

//--------------------------------------------                                                //
//  LED RainbowCycle
//--------------------------------------------
void StartLeds(void) 
{
 switch (Mem.LEDstrip)
    {
      case 0: LEDstrip = LED6812strip; 
              white  = 0xFF000000; 
              lgray  = 0x66000000;  
              gray   = 0x33000000;                                                            // The SK6812 LED has a white LED that is pure white
              dgray  = 0x22000000;
              wgray  = 0xAA000000;
              break;
      case 1: LEDstrip = LED2812strip; 
              white  = 0xFFFFFF;
              lgray  = 0x666666;                                                              // R, G and B on together gives white light
              gray   = 0x333333;
              dgray  = 0x222222;
              wgray  = 0xAAAAAA;         
              break;
     default: LEDstrip = LED6812strip;
              white  = 0xFF000000; 
              lgray  = 0x66000000;  
              gray   = 0x33000000;                                                            // The SK6812 LED has a white LED that is pure white
              dgray  = 0x22000000;
              wgray  = 0xAA000000;                    
                                           // Set initial brightness of LEDs   
    }
sprintf(sptext,"LED strip is %s", Mem.LEDstrip?"WS2812":"SK6812" ); Tekstprintln(sptext);
LEDstrip.setBrightness(16);  
LedsOff();                                                                                    // Set initial brightness of LEDs  (0-255)  
ShowLeds();
ColorLeds("",0,NUM_LEDS-1,green); ShowLeds();
// for(int i=0; i<10; i++)  ColorLeds("",i,i,Wheel(i * 25));
// ShowLeds();
}

//--------------------------------------------                                                //
//  LED RainbowCycle
//--------------------------------------------
void RainbowCycle(uint8_t wait) 
{
 uint16_t i, j;
 for(j=0; j<256 * 5; j++) 
   {                                                                                          // 5 cycles of all colors on wheel
    for(i=0; i< NUM_LEDS; i++) ColorLeds("",i,i,Wheel(((i * 256 / NUM_LEDS) + j) & 255));
    ShowLeds();
    delay(wait);
  }
}
//--------------------------------------------                                                //
//  LED WhiteOverRainbow
//--------------------------------------------
void WhiteOverRainbow(uint32_t wait, uint8_t whiteSpeed, uint32_t whiteLength ) 
{
 if(whiteLength >= NUM_LEDS) whiteLength = NUM_LEDS - 1;
 uint32_t head = whiteLength - 1;
 uint32_t tail = 0;
 uint32_t loops = 1;
 uint32_t loopNum = 0;
 static unsigned long lastTime = 0;
 while(true)
  {
    for(uint32_t j=0; j<256; j++) 
     {
      for(uint32_t i=0; i<=NUM_LEDS; i++) 
       {
        if((i >= tail && i <= head) || (tail > head && i >= tail) || (tail > head && i <= head) )
              ColorLeds("",i,i,0XFFFFFF );
        else  
              ColorLeds("",i,i,Wheel(((i * 256 / NUM_LEDS) + j) & 255));
       }
      if(millis() - lastTime > whiteSpeed) 
       {
        head++;        tail++;
        if(head == NUM_LEDS) loopNum++;
        lastTime = millis();
      }
      if(loopNum == loops) return;
      head %= NUM_LEDS;
      tail %= NUM_LEDS;
      ShowLeds();
      delay(wait);
    }
  }  // end while
}

//--------------------------------------------                                                //
//  LED In- or decrease light intensity value i.e. Slope
//--------------------------------------------
void WriteLightReducer(int amount)
{
 int value = Mem.LightReducer + amount;                                                       // Prevent byte overflow by making it an integer before adding
 Mem.LightReducer = (byte) min(value,  255);                                                         // May not be larger than 255
 sprintf(sptext,"Max brightness: %3d%%",Mem.LightReducer);
 Tekstprintln(sptext);
}

//--------------------------- Time functions --------------------------
    
//--------------------------------------------                                                //
// RTC Get time from NTP cq internal ESP32 RTC 
// and store it in timeinfo struct
// return local time in unix time format
//--------------------------------------------
time_t GetTijd(bool printit)
{
 time_t now;
 
 if (Mem.UseDS3231) GetDS3231Time(false);                                                     // If the DS3231 is attached and used get its time in timeinfo struct
 else
    { 
     if(Mem.NTPOn)  getLocalTime(&timeinfo);                                                  // If NTP is running get the local time
     else { time(&now); localtime_r(&now, &timeinfo);}                                        // Else get the time from the internal RTC and place it timeinfo
    }
 if (printit)  PrintRTCTime();                                                                // 
 localtime(&now);                                                                             // Get the actual time and
 return now;                                                                                  // Return the unixtime in seconds
}


//--------------------------------------------                                                //
// NTP print the NTP time for the timezone set 
//--------------------------------------------
void GetNTPtime(bool printit)
{
 NTP.getTime();                                                                               // Force a NTP time update 
 if(printit) PrintNTPtime();
}
//--------------------------------------------                                                //
// NTP print the NTP time for the timezone set 
//--------------------------------------------
void PrintNTPtime(void)
{
 sprintf(sptext,"%s  ", NTP.getTimeDateString());  
 Tekstprint(sptext);              // 17/10/2022 16:08:15
}

//--------------------------------------------                                                //
// NTP print the NTP UTC time 
//--------------------------------------------
void PrintUTCtime(void)
{
 time_t tmi;
 struct tm* UTCtime;
 time(&tmi);
 UTCtime = gmtime(&tmi);
 sprintf(sptext,"UTC: %02d:%02d:%02d %02d-%02d-%04d  ", 
     UTCtime->tm_hour,UTCtime->tm_min,UTCtime->tm_sec,
     UTCtime->tm_mday,UTCtime->tm_mon+1,UTCtime->tm_year+1900);
 Tekstprint(sptext);   
}
//--------------------------------------------                                                //
// Rotary encoder Init 
//--------------------------------------------
 void InitRotaryMod(void)
 {
 pinMode(encoderPinA,  INPUT_PULLUP);
 pinMode(encoderPinB,  INPUT_PULLUP);  
 pinMode(clearButton,  INPUT_PULLUP); 
 myEnc.write(0);                                                                              // Clear Rotary encode buffer
 sprintf(sptext,"Rotary %s used", Mem.UseRotary?"IS":"NOT");
 Tekstprintln(sptext);   
 } 

//--------------------------------------------                                                //
// DS3231 Init module
//--------------------------------------------
void InitDS3231Mod(void)
{
 DS3231Installed = IsDS3231I2Cconnected();                                                    // Check if DS3231 is connected and working   
 sprintf(sptext,"External RTC module %s found", DS3231Installed?"IS":"NOT");
 RTCklok.begin();     
 Tekstprintln(sptext);                                                                 
}
//--------------------------------------------                                                //
// DS3231 check for I2C connection
// DS3231_I2C_ADDRESS (= often 0X68) = DS3231 module
//--------------------------------------------
bool IsDS3231I2Cconnected(void)
 {
  bool DS3231Found = false;
  for (byte i = 1; i < 120; i++)
  {
   Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)                                                       
      {
      sprintf(sptext,"Found I2C address: 0X%02X", i); Tekstprintln(sptext);  
      if( i== DS3231_I2C_ADDRESS) DS3231Found = true;
      } 
  }
  return DS3231Found;   
  }
//--------------------------------------------                                                //
// DS3231 Get temperature from DS3231 module
//--------------------------------------------
float GetDS3231Temp(void)
{
 byte tMSB, tLSB;
 float temp3231;
 
  Wire.beginTransmission(DS3231_I2C_ADDRESS);                                                 // Temp registers (11h-12h) get updated automatically every 64s
  Wire.write(0x11);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 2);
 
  if(Wire.available()) 
  {
    tMSB = Wire.read();                                                                       // 2's complement int portion
    tLSB = Wire.read();                                                                       // fraction portion 
    temp3231 = (tMSB & 0b01111111);                                                           // do 2's math on Tmsb
    temp3231 += ( (tLSB >> 6) * 0.25 ) + 0.5;                                                 // only care about bits 7 & 8 and add 0.5 to round off to integer   
  }
  else   {temp3231 = -273; }  
  return (temp3231);
}

//--------------------------------------------                                                //
// DS3231 Set time in module DS3231
//--------------------------------------------
void SetDS3231Time(void)
{
RTCklok.adjust(DateTime(timeinfo.tm_year+1900, timeinfo.tm_mon+1, timeinfo.tm_mday, 
                        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
}

//--------------------------------------------                                                //
// DS3231 reads time in module DS3231
// and store it in Word clock time structure
//--------------------------------------------
void GetDS3231Time(bool printit)
{
 Inow             = RTCklok.now();                                                           // Be sure to get the lates DS3231 RTC clock time
 timeinfo.tm_hour = Inow.hour();
 timeinfo.tm_min  = Inow.minute();
 timeinfo.tm_sec  = Inow.second();
 timeinfo.tm_year = Inow.year() - 1900;                                                      // Inow.year() is years since 2000 tm_year is years since 1900
 timeinfo.tm_mon  = Inow.month() - 1;
 timeinfo.tm_mday = Inow.day();
 if (printit) PrintRTCTime(); 
}

//--------------------------------------------                                                //
// DS3231 prints time to serial
// reference https://adafruit.github.io/RTClib/html/class_r_t_c___d_s3231.html
//--------------------------------------------
void PrintDS3231Time(void)
{
 Inow = RTCklok.now();                                                                        // Be sure to get the lates DS3231 RTC clock time
 sprintf(sptext,"%02d/%02d/%04d %02d:%02d:%02d ", Inow.day(),Inow.month(),Inow.year(),
                                                  Inow.hour(),Inow.minute(),Inow.second());
 Tekstprint(sptext);
}

//--------------------------------------------                                                //
// RTC prints the ESP32 internal RTC time to serial
//--------------------------------------------
void PrintRTCTime(void)
{
 sprintf(sptext,"%02d/%02d/%04d %02d:%02d:%02d ", 
     timeinfo.tm_mday,timeinfo.tm_mon+1,timeinfo.tm_year+1900,
     timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
 Tekstprint(sptext);
}
//--------------------------------------------                                                //
// RTC Fill sptext with time
//--------------------------------------------
void PrintTimeHMS(){ PrintTimeHMS(2);}                                                        // print with linefeed
void PrintTimeHMS(byte format)
{
 sprintf(sptext,"%02d:%02d:%02d",timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
 switch (format)
 {
  case 0: break;
  case 1: Tekstprint(sptext); break;
  case 2: Tekstprintln(sptext); break;  
 }
}

//--------------------------------------------                                                //
// RTC Set time from global timeinfo struct
// Check if values are within range
//--------------------------------------------
void SetRTCTime(void) 
{ 
 time_t t = mktime(&timeinfo);                                                                // t= unixtime
 setRTCTime(t);
}  

void setRTCTime(time_t t)
{ 
 // time_t t = mktime(&timeinfo);  // t= unixtime
 sprintf(sptext, "Setting time: %s", asctime(&timeinfo));    Tekstprintln(sptext);
 struct timeval now = { .tv_sec = t , .tv_usec = 0};
 settimeofday(&now, NULL);
 GetTijd(false);                                                                                  // Synchronize time with RTC clock
 Displaytime();
 PrintTimeHMS();
}

//--------------------------------------------                                                //
// Print all the times available 
//--------------------------------------------
void PrintAllClockTimes(void)
{
 Tekstprint(" Clock time: ");
 PrintRTCTime();
 if(WiFi.localIP()[0] != 0)                                                                   // If no WIFI then no NTP time printed
   {
    Tekstprint("\n   NTP time: ");
    PrintNTPtime();
   }
 if(DS3231Installed)
   {
    Tekstprint("\nDS3231 time: ");
    PrintDS3231Time();
   }
 Tekstprintln(""); 
}
//                                                                                            //
// ------------------- End  Time functions 

//--------------------------------------------                                                //
//  CLOCK Convert Hex to uint32
//--------------------------------------------
uint32_t HexToDec(String hexString) 
{
 uint32_t decValue = 0;
 int nextInt;
 for (uint8_t i = 0; i < hexString.length(); i++) 
  {
   nextInt = int(hexString.charAt(i));
   if (nextInt >= 48 && nextInt <= 57)  nextInt = map(nextInt, 48, 57, 0, 9);
   if (nextInt >= 65 && nextInt <= 70)  nextInt = map(nextInt, 65, 70, 10, 15);
   if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
   nextInt = constrain(nextInt, 0, 15);
   decValue = (decValue * 16) + nextInt;
  }
 return decValue;
}
//------------------------------------------------------------------------------
// CLOCK Demo mode
//------------------------------------------------------------------------------
//                                                                                            //
void Demomode(void)
{
 if ( millis() - msTick == 10)  SetNanoLED13(0);                                              // Turn OFF the second on pin 13
 if ( millis() - msTick >= MilliSecondValue)                                                  // Flash the onboard Pin 13 Led so we know something is happening
 {    
  msTick = millis();                                                                          // second++; 
  SetNanoLED13(100);                                                                          // Turn ON the second on pin 13
  if( ++timeinfo.tm_sec >59) { timeinfo.tm_sec = 0; timeinfo.tm_min++; Displaytime();}
  if( timeinfo.tm_min >59) { timeinfo.tm_min = 0; timeinfo.tm_sec = 0; timeinfo.tm_hour++;}
  if( timeinfo.tm_hour >24) timeinfo.tm_hour = 0;                                             // If hour is after 12 o'clock 
  DimLeds(false);
  SerialCheck();
 }
}
//--------------------------------------------                                                //
// BLE 
// SendMessage by BLE Slow in packets of 20 chars
// or fast in one long string.
// Fast can be used in IOS app BLESerial Pro
//------------------------------
void SendMessageBLE(std::string Message)
{
 if(deviceConnected) 
   {
    if (Mem.UseBLELongString)                                                                 // If Fast transmission is possible
     {
      pTxCharacteristic->setValue(Message); 
      pTxCharacteristic->notify();
      delay(10);                                                                              // Bluetooth stack will go into congestion, if too many packets are sent
     } 
   else                                                                                       // Packets of max 20 bytes
     {   
      int parts = (Message.length()/20) + 1;
      for(int n=0;n<parts;n++)
        {   
         pTxCharacteristic->setValue(Message.substr(n*20, 20)); 
         pTxCharacteristic->notify();
         delay(10);                                                                           // Bluetooth stack will go into congestion, if too many packets are sent
        }
     }
   } 
}
//-----------------------------
// BLE Start BLE Classes
//------------------------------
class MyServerCallbacks: public BLEServerCallbacks 
{
 void onConnect(BLEServer* pServer) {deviceConnected = true; };
 void onDisconnect(BLEServer* pServer) {deviceConnected = false;}
};

class MyCallbacks: public BLECharacteristicCallbacks 
{
 void onWrite(BLECharacteristic *pCharacteristic) 
  {
   std::string rxValue = pCharacteristic->getValue();
   ReceivedMessageBLE = rxValue + "\n";
//   if (rxValue.length() > 0) {for (int i = 0; i < rxValue.length(); i++) printf("%c",rxValue[i]); }
//   printf("\n");
  }  
};
//--------------------------------------------                                                //
// BLE Start BLE Service
//------------------------------
void StartBLEService(void)
{
 BLEDevice::init(Mem.BLEbroadcastName);                                                       // Create the BLE Device
 pServer = BLEDevice::createServer();                                                         // Create the BLE Server
 pServer->setCallbacks(new MyServerCallbacks());
 BLEService *pService = pServer->createService(SERVICE_UUID);                                 // Create the BLE Service
 pTxCharacteristic                     =                                                      // Create a BLE Characteristic 
     pService->createCharacteristic(CHARACTERISTIC_UUID_TX, NIMBLE_PROPERTY::NOTIFY);                 
 BLECharacteristic * pRxCharacteristic = 
     pService->createCharacteristic(CHARACTERISTIC_UUID_RX, NIMBLE_PROPERTY::WRITE);
 pRxCharacteristic->setCallbacks(new MyCallbacks());
 pService->start(); 
 BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
 pAdvertising->addServiceUUID(SERVICE_UUID); 
 pServer->start();                                                                            // Start the server  Nodig??
 pServer->getAdvertising()->start();                                                          // Start advertising
 TekstSprint("BLE Waiting a client connection to notify ...\n"); 
}

//--------------------------------------------                                                //
// BLE  CheckBLE
//------------------------------
void CheckBLE(void)
{
 if(!deviceConnected && oldDeviceConnected)                                                   // Disconnecting
   {
    delay(300);                                                                               // Give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();                                                              // Restart advertising
    TekstSprint("Start advertising\n");
    oldDeviceConnected = deviceConnected;
   }
 if(deviceConnected && !oldDeviceConnected)                                                   // Connecting
   { 
    oldDeviceConnected = deviceConnected;
    SWversion();
   }
 if(ReceivedMessageBLE.length()>0)
   {
    SendMessageBLE(ReceivedMessageBLE);
    String BLEtext = ReceivedMessageBLE.c_str();
    ReceivedMessageBLE = "";
    ReworkInputString(BLEtext); 
   }
}

//--------------------------------------------                                                //
// WIFI WIFIEvents
//--------------------------------------------
void WiFiEvent(WiFiEvent_t event)
{
  sprintf(sptext,"[WiFi-event] event: %d  : ", event); 
  Tekstprint(sptext);
  WiFiEventInfo_t info;
    switch (event) 
     {
        case ARDUINO_EVENT_WIFI_READY: 
            Tekstprintln("WiFi interface ready");
            break;
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
            Tekstprintln("Completed scan for access points");
            break;
        case ARDUINO_EVENT_WIFI_STA_START:
            Tekstprintln("WiFi client started");
            break;
        case ARDUINO_EVENT_WIFI_STA_STOP:
            Tekstprintln("WiFi clients stopped");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Tekstprintln("Connected to access point");
            break;
       case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            sprintf(sptext,"WiFi lost connection.");                                          // Reason: %d",info.wifi_sta_disconnected.reason); 
            Tekstprintln(sptext);
              // WiFi.disconnect();
              // WIFIwasConnected = false; 
//            WiFi.reconnect();
            break;
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            Tekstprintln("Authentication mode of access point has changed");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            sprintf(sptext, "Obtained IP address: %d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
            Tekstprintln(sptext);
//            WiFiGotIP(event,info);
            break;
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            Tekstprintln("Lost IP address and IP address is reset to 0");
            WIFIwasConnected = false;
            break;
        case ARDUINO_EVENT_WPS_ER_SUCCESS:
     //      txtstr = WiFi.SSID().c_str();
           sprintf(sptext, "WPS Successfull, stopping WPS and connecting to: %s: ", WiFi.SSID().c_str());
           Tekstprintln(sptext);
//            wpsStop();
//            delay(10);
//            WiFi.begin();
            break;
        case ARDUINO_EVENT_WPS_ER_FAILED:
            Tekstprintln("WPS Failed, retrying");
//            wpsStop();
//            wpsStart();
            break;
        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            Tekstprintln("WPS Timedout, retrying");
//            wpsStop();
//            wpsStart();
            break;
        case ARDUINO_EVENT_WPS_ER_PIN:
 //           txtstr = wpspin2string(info.wps_er_pin.pin_code).c_str();
//            sprintf(sptext,"WPS_PIN = %s",txtstr);
//            Tekstprintln(sptext);
            break;
        case ARDUINO_EVENT_WIFI_AP_START:
            Tekstprintln("WiFi access point started");
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            Tekstprintln("WiFi access point  stopped");
            break;
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
            Tekstprintln("Client connected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
            sprintf(sptext,"Client disconnected.");                                            // Reason: %d",info.wifi_ap_stadisconnected.reason); 
            Tekstprintln(sptext);
            break;
        case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
            Tekstprintln("Assigned IP address to client");
            break;
        case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
            Tekstprintln("Received probe request");
            break;
        case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
            Tekstprintln("AP IPv6 is preferred");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            Tekstprintln("STA IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP6:
            Tekstprintln("Ethernet IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_START:
            Tekstprintln("Ethernet started");
            break;
        case ARDUINO_EVENT_ETH_STOP:
            Tekstprintln("Ethernet stopped");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            Tekstprintln("Ethernet connected");
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            // WiFi.scanNetworks will return the number of networks found("Ethernet disconnected");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            Tekstprintln("Obtained IP address");
            WiFiGotIP(event,info);
            break;
        default: break;
    }
}
//--------------------------------------------                                                //
// NTP Check NTP SyncEvent 
//--------------------------------------------
void NTPsyncEvent(void)
{
    if (syncEventTriggered) 
    {
      syncEventTriggered = false;
      processSyncEvent (ntpEvent);
    }
}
//--------------------------------------------                                                //
// NTP processSyncEvent 
//--------------------------------------------
void processSyncEvent (NTPEvent_t ntpEvent) 
{
 switch (ntpEvent.event) 
    {
        case timeSyncd:
        case partlySync:
        case syncNotNeeded:
        case accuracyError:
            sprintf(sptext,"[NTP-event] %s", NTP.ntpEvent2str (ntpEvent));
            Tekstprintln(sptext);
            break;
        default:
            break;
    }
}
//--------------------------------------------                                                //
// WIFI GOT IP address Used with WiFiEvent()
//--------------------------------------------
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
 WIFIwasConnected = true;
 if(Mem.WIFIOn) WebPage();                                                                    // Show the web page if WIFI is on
 if(Mem.NTPOn)
   {
    NTP.setTimeZone(Mem.Timezone);                                                            // TZ_Europe_Amsterdam); //\TZ_Etc_GMTp1); // TZ_Etc_UTC 
    NTP.begin();                                                                              // https://raw.githubusercontent.com/nayarsystems/posix_tz_db/master/zones.csv
    Tekstprintln("NTP is On"); 
   } 
}


//--------------------------------------------                                                //
// WIFI Check for WIFI Network 
// Check if WIFI network to connect to is available
//--------------------------------------------
 bool CheckforWIFINetwork(void)         { return CheckforWIFINetwork(true);}
 bool CheckforWIFINetwork(bool PrintIt)
 {
  WiFi.mode(WIFI_STA);                                                                         // Set WiFi to station mode and disconnect from an AP if it was previously connected.
  WiFi.disconnect(); 
  WIFIwasConnected = false;
  int n = WiFi.scanNetworks();                                                                // WiFi.scanNetworks will return the number of networks found
  SSIDandNetworkfound = false;
  Tekstprintln("Scanning for networks");
  if (n == 0) { Tekstprintln("no networks found"); return false;} 
  else 
    { 
     sprintf(sptext,"%d: networks found",n);    Tekstprintln(sptext);
     for (int i = 0; i < n; ++i)                                                              // Print SSID and RSSI for each network found
      {
        sprintf(sptext,"%2d: %20s %3d %1s",i+1,WiFi.SSID(i).c_str(),WiFi.RSSI(i),(WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
        if (strcmp(WiFi.SSID(i).c_str(), Mem.SSID)==0) 
           {
            strcat(sptext, " -- Will connect to");
            SSIDandNetworkfound = true;
           }
        if(PrintIt) Tekstprintln(sptext);
      }
    }
 return SSIDandNetworkfound; 
 }

//--------------------------------------------                                                //
// WIFI Scan for WIFI stations
//--------------------------------------------
void ScanWIFI(void)
{
 WiFi.mode(WIFI_STA);                                                                         // Set WiFi to station mode and disconnect from an AP if it was previously connected.
 WiFi.disconnect();
 WIFIwasConnected = false;
 delay(100);
 int n = WiFi.scanNetworks();                                                                 // WiFi.scanNetworks will return the number of networks found.
 if (n == 0)  { Tekstprintln("no networks found");  } 
 else 
   {
    sprintf(sptext,"%d networks found",n);   Tekstprintln(sptext);
    Tekstprintln("Nr | SSID                             | RSSI | CH | Encryption");
    for(int i = 0; i < n; ++i) 
      {
       sprintf(sptext,"%2d | %-32.32s | %4d | %2d | ",i + 1, 
                       WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i));                  // Print SSID and RSSI for each network found
       Tekstprint(sptext);
       switch (WiFi.encryptionType(i))
           {
            case WIFI_AUTH_OPEN:
                Tekstprint("open");
                break;
            case WIFI_AUTH_WEP:
                Tekstprint("WEP");
                break;
            case WIFI_AUTH_WPA_PSK:
                Tekstprint("WPA");
                break;
            case WIFI_AUTH_WPA2_PSK:
                Tekstprint("WPA2");
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                Tekstprint("WPA+WPA2");
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                Tekstprint("WPA2-EAP");
                break;
            case WIFI_AUTH_WPA3_PSK:
                Tekstprint("WPA3");
                break;
            case WIFI_AUTH_WPA2_WPA3_PSK:
                Tekstprint("WPA2+WPA3");
                break;
            case WIFI_AUTH_WAPI_PSK:
                Tekstprint("WAPI");
                break;
            default:
                Tekstprint("unknown");
            }
            Tekstprintln("");
            delay(10);
        }
   }
Tekstprintln("");
WiFi.scanDelete();                                                                            // Delete the scan result to free memory for code below.
}


//--------------------------------------------                                                //
// WIFI WEBPAGE 
//--------------------------------------------
void StartWIFI_NTP(void)
{
  WiFi.disconnect(true,true);                                                                 // remove all previous settings and entered SSID and password
 delay(100);
 WiFi.setHostname(Mem.BLEbroadcastName);                                                      // Set the host name in the WIFI router instead of a cryptic esp32 name
 WiFi.mode(WIFI_STA);  
 WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
 WIFIwasConnected = false;
 WiFi.onEvent(WiFiEvent);                                                                     // Using WiFi.onEvent interrupts and crashes IL9341 screen display while writing the screen
 WiFi.begin(Mem.SSID, Mem.Password);
 MDNS.begin(Mem.BLEbroadcastName);                                                            // After reset http://FiboESP32.local 
 
 int tryDelay = 5000;                                                                         // Will try for about 50 seconds (20x 10,000ms)
 int numberOfTries = 10;
 while (true)                                                                                 // Wait for the WiFi event
  {
    switch(WiFi.status()) 
    {
     case WL_NO_SSID_AVAIL:
          Tekstprintln("[WiFi] SSID not found");
          break;
     case WL_CONNECT_FAILED:
          Tekstprint("[WiFi] Failed - WiFi not connected! Reason: ");
          return;
          break;
     case WL_CONNECTION_LOST:
          Tekstprintln("[WiFi] Connection was lost");
          break;
     case WL_SCAN_COMPLETED:
          Tekstprintln("[WiFi] Scan is completed");
          break;
     case WL_DISCONNECTED:
          Tekstprintln("[WiFi] WiFi is disconnected, will reconnect");
          WiFi.reconnect();
          break;
     case WL_CONNECTED:
          sprintf(sptext, "IP Address: %d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
          Tekstprintln(sptext); 
          WIFIwasConnected = true;
          break;
     default:
          Serial.print("[WiFi] WiFi Status: ");
          Serial.println(WiFi.status());
          break;
    } 
  ColorLed(ProgressLedNr++,frenchviolet); ShowLeds();
  if (WIFIwasConnected) break;       
  if(numberOfTries <= 0)
      {
       Tekstprint("[WiFi] Failed to connect to WiFi! Check your password, SSID and router");
       WiFi.disconnect();                                                                    // Use disconnect function to force stop trying to connect
       WIFIwasConnected = false;
       return;
      } 
  else 
      { 
       delay(tryDelay);  
       numberOfTries--;    
      }
  }
if ( !WIFIwasConnected) return;                                                               // If WIFI connection fails -> returm
//  sprintf(sptext, "IP Address: %d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );  Tekstprintln(sptext); 
ElegantOTA.begin(&server);                                                                    // Start ElegantOTA  new version in 2023  
       // if compile error see here :https://docs.elegantota.pro/async-mode/                                                                                           // Open ElegantOTA folder and then open src folder
                                                                                              // Locate the ELEGANTOTA_USE_ASYNC_WEBSERVER macro in the ElegantOTA.h file, and set it to 1:
                                                                                              // #define ELEGANTOTA_USE_ASYNC_WEBSERVER 1ColorLed(ProgressLedNr++,frenchviolet); ShowLeds();                                           // Save the changes to the ElegantOTA.h file.   
if(Mem.NTPOn )
  {
   NTP.setTimeZone(Mem.Timezone);                                                             // TZ_Europe_Amsterdam); //\TZ_Etc_GMTp1); // TZ_Etc_UTC 
   NTP.begin();                                                                               // https://raw.githubusercontent.com/nayarsystems/posix_tz_db/master/zones.csv
   NTP.onNTPSyncEvent([](NTPEvent_t event){ntpEvent=event; syncEventTriggered=true;});
   Tekstprintln("NTP is On"); 
   }   
 ColorLed(ProgressLedNr++,frenchviolet); ShowLeds();
 if(Mem.WIFIOn) WebPage();                                                                    // Show the web page if WIFI is on
 
 Tekstprint("Web page started\n");
}


//--------------------------------------------                                                //
// WIFI WEBPAGE 
//--------------------------------------------
void WebPage(void) 
{
 server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)                                  // Send web page with input fields to client
          { request->send(200, "text/html", index_html);  }    );
 server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request)                              // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
       { 
        String inputMessage;    String inputParam;
        if (request->hasParam(PARAM_INPUT_1))                                                 // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
           {
            inputMessage = request->getParam(PARAM_INPUT_1)->value();
            inputParam = PARAM_INPUT_1;
           }
        else 
           {
            inputMessage = "";    //inputMessage = "No message sent";
            inputParam = "none";
           }  
 //     sprintf(sptext,"%s",inputMessage.c_str());    Tekstprintln(sptext); 
        ReworkInputString(inputMessage);
        request->send(200, "text/html", index_html);
       }   );   
 server.onNotFound(notFound);
 server.begin();
}

//--------------------------------------------                                                //
// WIFI WEBPAGE 
//--------------------------------------------
void notFound(AsyncWebServerRequest *request) 
{
  request->send(404, "text/plain", "Not found");
}

                                  #ifdef ONEWIREKEYPAD3x4
//--------------------------------------------                                                //
// KEYPAD check for Onewire Keypad input
//--------------------------------------------
void OnewireKeypad3x4Check(void)
{
 int keyvalue=99;;
 int Key=0;
 int sensorValue = analogRead(OneWirePin); // read the value from the sensor:
 switch(sensorValue)
  {
    case   0 ... 100:  keyvalue = 13; break;   // noise
    case 101 ... 132:  keyvalue = 12; Key = '*'; break;   // * 
    case 133 ... 154:  keyvalue =  0; Key = '0'; break;   // 0 
    case 155 ... 216:  keyvalue = 11; Key = '#'; break;   // # 
    case 217 ... 281:  keyvalue =  7; Key = '7'; break;   // 7 
    case 282 ... 318:  keyvalue =  4; Key = '4'; break;   // 4 
    case 319 ... 349:  keyvalue =  1; Key = '1'; break;   // 1 
    case 350 ... 390:  keyvalue =  8; Key = '8'; break;   // 8 
    case 391 ... 463:  keyvalue =  5; Key = '5'; break;   // 5 
    case 464 ... 519:  keyvalue =  2; Key = '2'; break;   // 2 
    case 520 ... 619:  keyvalue =  9; Key = '9'; break;   // 9 
    case 620 ... 848:  keyvalue =  6; Key = '6'; break;   // 6 
    case 849 ... 1023: keyvalue =  3; Key = '3'; break;   // 3
  }
 if(keyvalue<13) { Serial.println(Key); delay(300); }
  if (Key == 12)   // *                                                                       // Pressing a * activates the keyboard input. 
   { 
    KeyInputactivated = true;
    KeyLooptime = millis();
    KeypadString ="";
    ColorLeds("",0,NUM_LEDS-1,0x00FF00);                                                      // Turn all LEDs green
    ShowLeds();                                                                               // Push data in LED strip to commit the changes
    Serial.println(F("Key entry activated"));
   }
 if (KeyInputactivated && (Key>=0 && Key<10))
   {
    delay(20); 
    KeypadString += Key;                                                                      // Digit keys 0 - 9
    ColorLeds("",0,Key-48,0xFF0000);                                                          // Turn all LEDs red
    ShowLeds();                                                                               // Push data in LED strip to commit the changes
 //   Serial.println(KeypadString);
   }
 if (KeypadString.length()>5)                                                                 // If six numbers are entered rework this to a time hhmmss
   {       
   if(KeypadString=="999999")
     { 
      KeypadString = "";   
      Reset();
      Serial.println(F("Settings reset"));   
     }
    else 
     {      
      ReworkInputString(KeypadString);                                                        // Rework ReworkInputString();
      KeypadString = "";
      Serial.println(F("Time changed"));
     }    
   }
 if ( KeyInputactivated && ((millis() - KeyLooptime) > 30000) ) 
   {  
    KeyInputactivated = false;                                                                // Stop data entry after 30 seconds. This avoids unintended entry 
    KeypadString ="";
    Serial.println(F("Keyboard entry stopped"));
  }
}
                                  #endif  //ONEWIREKEYPAD3x4  
                                  #ifdef ONEWIREKEYPAD3x1
//--------------------------------------------                                                //
// KEYPAD check for Onewire Keypad input with 5V and 1.1, 4.7, 4.7, 4.7 kOhm resistors
//--------------------------------------------
void OnewireKeypad3x1Check(void)
{
 int8_t keyvalue = 99;
 int8_t Key;
 int16_t sensorValue = analogRead(OneWirePin);                                                    // Read the value from the sensor:
 switch(sensorValue)
   {
    case   0 ... 385:  keyvalue = 99;            break;                                       // Noise
    case 386 ... 635:  keyvalue = -1; Key = 'G'; break;                                       // G 
    case 636 ... 910:  keyvalue =  0; Key = 'Y'; break;                                       // Y 
    case 911 ... 1024: keyvalue =  1; Key = 'R'; break;                                       // R 
   }
 if(keyvalue<2) 
    { 
//     Serial.print(sensorValue); Serial.println(Key); 
     if (Key == 'R') ProcessKeyPressTurn(1);                                                  // Pressing Red increases hour or minute. 
     if (Key == 'G') ProcessKeyPressTurn(-1);                                                 // Pressing Green decreases hour or minute. 
     if (Key == 'Y') ProcessKeyPressTurn(0);                                                  // Pressing Yellow activates the keyboard input. 
     delay(200);     
    }
}
                                  #endif //ONEWIREKEYPAD3x1

//--------------------------------------------                                                //
// KY-040 ROTARY check if the rotary is moving
//--------------------------------------------
void RotaryEncoderCheck(void)
{
 int ActionPress = 999;
 if (digitalRead(clearButton) == LOW )          ProcessKeyPressTurn(0);                       // Set the time by pressing rotary button
 else if (ChangeTime)    
  {   
   ActionPress = myEnc.read();                                                                // If the knob is turned store the direction (-1 or 1)
   if (ActionPress == 0) {  ActionPress = 999;  ProcessKeyPressTurn(ActionPress);  }          // Sent 999 = nop (no operation) 
   if (ActionPress == 1 || ActionPress == -1 )  ProcessKeyPressTurn(ActionPress);             // Process the ActionPress
  } 
 myEnc.write(0);                                                                              // Set encoder pos back to 0
}

//--------------------------------------------                                                //
// CLOCK
// KY-040 or Membrane 3x1 processing input
// encoderPos < 1 left minus 
// encoderPos = 0 attention and selection choice
// encoderPos > 1 right plus
//--------------------------------------------
void ProcessKeyPressTurn(int encoderPos)
{
if ((unsigned long) (millis() - RotaryPressTimer) > 60000)                                    // After 60 sec after shaft is pressed time of light intensity can not be changed 
   {
    if (ChangeTime || ChangeLightIntensity)                         
      {
        Tekstprintln("<-- Changing time is over -->");
        NoofRotaryPressed = 0;
      }
    ChangeTime            = false;
    ChangeLightIntensity  = false;
   }  
 if (ChangeTime || ChangeLightIntensity)                                                      // If shaft is pressed time of light intensity can be changed
   {
    if ( encoderPos!=999 && ( (millis() - Looptime) > 250))                                   // If rotary turned avoid debounce within 0.25 sec
     {   
     Serial.print(F("----> Index:"));   Serial.println(encoderPos);
     if (encoderPos == 1)                                                                     // Increase  
       {     
        if (ChangeLightIntensity)  { WriteLightReducer(5); }                                  // If time < 60 sec then adjust light intensity factor
        if (ChangeTime) 
          {
           if (NoofRotaryPressed == 1)                                                        // Change hours
              {if( ++timeinfo.tm_hour >23) { timeinfo.tm_hour = 0; } }      
           if (NoofRotaryPressed == 2)                                                        // Change minutes
              {  timeinfo.tm_sec = 0;
               if( ++timeinfo.tm_min >59) { timeinfo.tm_min = 0; if( ++timeinfo.tm_hour >23) { timeinfo.tm_hour = 0; } }   }
           } 
        }    
      if (encoderPos == -1)                                                                   // Decrease
       {
       if (ChangeLightIntensity)   { WriteLightReducer(-5); }    // If time < 60 sec then adjust light intensity factor
       if (ChangeTime)     
          {
           if (NoofRotaryPressed == 1)                                                        // Change hours
            { if( timeinfo.tm_hour-- ==0) { timeinfo.tm_hour = 23; } }      
           if (NoofRotaryPressed == 2)                                                        // Change minutes
            { timeinfo.tm_sec = 0;
             if( timeinfo.tm_min-- == 0) { timeinfo.tm_min = 59; if( timeinfo.tm_hour-- == 0) { timeinfo.tm_hour = 23; } }  }
          }          
        } 
      SetDS3231Time();  
      PrintDS3231Time();
      Looptime = millis();       
     }                                                
   }
 if (encoderPos == 0 )                                                                        // Set the time by pressing rotary button
   { 
    delay(250);
    ChangeTime            = false;
    ChangeLightIntensity  = false;
    RotaryPressTimer      = millis();                                                         // Record the time the shaft was pressed.
    if(++NoofRotaryPressed >6 ) NoofRotaryPressed = 0;
    switch (NoofRotaryPressed)                                                                // No of times the rotary is pressed
      {
       case 1:  ChangeTime = true;               ColorLeds("",0,NUM_LEDS-1,0x00FFFF); break;  // Turn all LEDs yellow Change the hours
       case 2:  ChangeTime = true;               ColorLeds("",0,NUM_LEDS-1,0x00FF00); break;  // Turn all LEDs green  Change the hours        
       case 3:  ChangeLightIntensity = true;     ColorLeds("",0,NUM_LEDS-1,0xFF0000); break;  // Turn all LEDs red    Change intensity 
       case 4:                                                      break;                    // Shows the DCF-signal in the display                               
       case 5:                                                      break;
       case 6:  Reset();                                            break;                    // Reset all settings                                                                  
      default:                                                      break;                     
      }
    Serial.print(F("NoofRotaryPressed: "));   Serial.println(NoofRotaryPressed);   
    Looptime = millis();     
    Displaytime();                                                                            // Turn on the LEDs with proper time
   }
 }

//                                                                                            //
//--------------------------------------------
// FIBONACCI Set the color and strip based on the time
//--------------------------------------------
void LEDsetTime(byte hours, byte minutes)
{ 
// hours %=12;                                                                                // Keep the hours between 0 and 12
 if (!(hours==12 && minutes<5)) hours %=12;                                                     // Keep the hours between 0 and 12 but display red when 12:00-12:04
 for(int i=0; i<CLOCK_PIXELS; i++) { bits[i] = 0; BitSet[i] = 0; }                            // Clear all bits  
 MakeFibonacciList(hours);
 for(int i=0; i<CLOCK_PIXELS; i++) if(BitSet[i]) bits[i] +=1;                                 // If hour must be lit add 1
 for(int i=0; i<CLOCK_PIXELS; i++)    BitSet[i] = 0;                                          // Clear  bits  
 MakeFibonacciList(minutes/5);                                                                // Block is 5 minutes  
  for(int i=0; i<CLOCK_PIXELS; i++)  
 {
   if( BitSet[i]) bits[i] +=2;    
   setPixel(i, colors[Mem.DisplayChoice][bits[i]]); 
  // Serial.print(hours); Serial.print(":"); Serial.print(minutes);  Serial.print(" ");  
  // Serial.println(colors[Mem.DisplayChoice][bits[i]],HEX);
  }
}

//--------------------------------------------                                                //
// FIBONACCI Calculate the proper Fibonacci-numbers (Pixels)
//--------------------------------------------
void MakeFibonacciList(byte Value)
{
 byte CalcValue = 0;
 byte pos = CLOCK_PIXELS;
 while (Value != CalcValue  )
  {
   byte Fibonaccireeks[] = {1,1,2,3,5,0};                          // Set up Fibonacci array with 6 numbers.
   for(int i=0; i<=CLOCK_PIXELS; i++) BitSet[i] = 0;               // Clear all bits. NB CLOCK_PIXELS is noof cells / strips in the clock
   CalcValue = 0;
   while ( (Value != CalcValue) &&  CalcValue <=  Value)   
    {
     do { pos = random(CLOCK_PIXELS); } while(Fibonaccireeks[pos] == 0 );   
     CalcValue += Fibonaccireeks[pos];
     BitSet[pos] = 1;                                              // Set pos in array for valid number    
     Fibonaccireeks[pos] = 0;                                      // Number taken from array 
    }
  }
}

//--------------------------------------------                                                //
// FIBONACCI Calculate the proper chronological numbers (Pixels)
//--------------------------------------------
void MakeChronoList(byte Hours, byte Minutes,byte Seconds)
{
 Hours %=12;                                                                                  // Keep the hours between 0 and 12
 byte Secsegment = Seconds / 5;
 byte Minsegment = Minutes / 5;
 byte Bit;
 uint32_t Kleur;                                                                              // Color
 for(int i=0; i<12; i++)
  {
   Bit = 0;
   if(i < Hours)        Bit+= 1;                                                              // If hours use the second colour
   if(i < Minsegment)   Bit+= 2;                                                              // If minute use the third colour. If hours was set the fourth colour is displayed 
   if(Mem.NoExUl>0 && i == Secsegment)  Bit = 4;                                              // If second use the fifth colour to display 
   Kleur = colors[Mem.DisplayChoice][Bit];
   if(Mem.NoExUl>1) {if(i<Minutes%5 && Seconds%5<1) Kleur=purple;}                            // If in Ultimate mode
   ColorLed(i,Kleur); 
   }
}
//                                            
//--------------------------------------------                                                //
// FIBONACCI Turn on the right pixels and colours for 24 hour 
//--------------------------------------------
void setPixel(byte pixel, uint32_t kleur)
{
 switch(NUM_LEDS)
 {
  case 12:
        switch(pixel)                                                                         // 12 LEDs 
          {
            case 0:      ColorLeds("", 0, 0,kleur); break;
            case 1:      ColorLeds("", 1, 1,kleur); break;
            case 2:      ColorLeds("", 2, 3,kleur); break;
            case 3:      ColorLeds("", 4, 6,kleur); break;
            case 4:      ColorLeds("", 7,11,kleur); break;
          }
          break;
  case 14:
           switch(pixel)                                                                      // 14 LEDs    
           {
            case 0:      ColorLeds("", 0, 0,kleur); break;
            case 1:      ColorLeds("", 1, 1,kleur); break;
            case 2:      ColorLeds("", 2, 3,kleur); break;
            case 3:      ColorLeds("", 4, 7,kleur); break;
            case 4:      ColorLeds("", 8,13,kleur); break;
           }
          break;
  case 24:
         switch(pixel)                                                                        // 24 LEDs    
          {
            case 0:      ColorLeds("", 0, 1,kleur); break;
            case 1:      ColorLeds("", 2, 3,kleur); break;
            case 2:      ColorLeds("", 4, 7,kleur); break;
            case 3:      ColorLeds("", 8, 13,kleur); break;
            case 4:      ColorLeds("", 14,23,kleur); break;
          }
          break;    
 case 32:
  switch(pixel)                                                                               // for 32 LEDs, 4 strips of 8 LEDs
           {
             case 0:      ColorLeds("", 2, 3,kleur);                            break;
             case 1:      ColorLeds("",12,13,kleur);                            break;
             case 2:      ColorLeds("", 0, 1,kleur); ColorLeds("",14,15,kleur); break;
             case 3:      ColorLeds("",16,19,kleur); ColorLeds("",28,31,kleur); break;
             case 4:      ColorLeds("", 4,11,kleur); ColorLeds("",20,27,kleur); break;
           }
         break;
  case 36:                                                                                      // For case with 3 x 12 LEDs clock 
           switch(pixel)  
          {
            case 0:      ColorLeds("", 0,   2,kleur); break;
            case 1:      ColorLeds("", 3,   5,kleur); break;
            case 2:      ColorLeds("", 6,  11,kleur); break;
            case 3:      ColorLeds("", 12, 20,kleur); break;
            case 4:      ColorLeds("", 21, 35,kleur); break; 
          }
         break;
 case 174:                                                                                     // For 50x50 cm case with 174 LEDs    
         switch(pixel)  
          {
            case 0:      ColorLeds("",  0,   15,kleur); break;
            case 1:      ColorLeds("", 16,   31,kleur); break;
            case 2:      ColorLeds("", 32,   63,kleur); break;
            case 3:      ColorLeds("", 64,  103,kleur); break;
            case 4:      ColorLeds("", 104, 173,kleur); break; 
          }
         break;         
  default:
           switch(pixel)                                                                      // 14 LEDs    
           {
            case 0:      ColorLeds("", 0, 0,kleur); break;
            case 1:      ColorLeds("", 1, 1,kleur); break;
            case 2:      ColorLeds("", 2, 3,kleur); break;
            case 3:      ColorLeds("", 4, 7,kleur); break;
            case 4:      ColorLeds("", 8,13,kleur); break;
           }
          break;
 }  
}

//                                                                                            //
