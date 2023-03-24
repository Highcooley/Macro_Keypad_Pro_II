// Macro Keypad Pro by Hayri 27.07.2020
// edited to latest PCB version 30.10.2020
// edited to have more than two layers by HighC 24.03.2023
// USE THIS FROM NOW ON!!!

#include <SPI.h>
#include <Wire.h>
#include "bitmaps.h"
#include <TimerOne.h>
#include <HID-Project.h>
#include <Adafruit_GFX.h>
#include <ClickEncoder.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino_ST7789_Fast.h>

#define TFT_CS    A4
#define TFT_DC     2
#define TFT_RST   A3
#define SCR_WD   240
#define SCR_HT   240

int sw_detect;
int sw_old;
int page;  

Arduino_ST7789 display = Arduino_ST7789(TFT_DC, TFT_RST, TFT_CS);

#define ORANGE      RGBto565( 255, 128,  64)
#define GREY        RGBto565( 127, 127, 127)
#define DARKGREY    RGBto565(  64,  64,  64)
#define TURQUOISE   RGBto565(   0, 128, 128)  // TOO DARK
#define PINK2       RGBto565( 255, 128, 192)  // IS BRIGHT
#define PINK        RGBto565( 255,   0, 100)  // my version of it :-)
#define OLIVE       RGBto565( 128, 128,   0)  // NOT BAD
#define PURPLE      RGBto565( 255,   0, 128)  // edited, was 128, 0, 128 // DARK BUT NICE
#define AZURE       RGBto565(   0, 128, 255)  // VERY NICE, GOES PERFECT WIT BLUE
#define BROWN       RGBto565( 120,  60,  30)

#define KEYCOLOR1   RGBto565( 255,   0, 228)
#define KEYCOLOR2   RGBto565(   0, 255, 255)
#define KEYCOLOR3   RGBto565( 255, 255,   0)
#define KEYCOLOR4   RGBto565(   0, 255,   0)
#define KEYCOLOR5   RGBto565(  10,  10,  10)
#define KEYCOLOR6   RGBto565(   0, 255, 255)

const int analogInPin = A5;                   // Analog input pin that the LDR is attached to
const int analogOutPin = 3;                   // Analog/PWM output pin that the Transistor is attached to

int ledPin = 3;

template<typename T>
struct TimeStampedValue {
    explicit TimeStampedValue(T value) : _value(value), _timeStamp(0) {}
    void set(const T& value) {
      _value = value;
      touch();
    }
    operator const T&() const {
      return _value;
    }
    void touch() {
      _timeStamp = millis();
    }
    unsigned long getTimeStamp() const {
      return _timeStamp;
    }

  private:
    T _value;
    unsigned long _timeStamp;
};

#define LUp 4
#define Up 5
#define RUp 6
#define Left 7
#define Down 8
#define Right 13
#define Switch 0

#define TIMEOUT_VIBRA_MS   30
#define TIMEOUT_LIGHTS_MS 600

Adafruit_NeoPixel strip  ( 6, 12, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_a(15, A1, NEO_GRB + NEO_KHZ800);

#define PIN              12
#define PIN_a            A1
#define NUMPIXELS         6
#define NUMPIXELS_a      15
Adafruit_NeoPixel pixels   = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_a = Adafruit_NeoPixel(NUMPIXELS_a, PIN_a, NEO_GRB + NEO_KHZ800);

const uint32_t REP  = strip_a.Color(120,    0, 120);
const uint32_t BLUP = strip_a.Color(120,  120,   0);
const uint32_t GREE = strip_a.Color(  0,  120, 120);
const uint32_t OFF  = strip_a.Color( 20,    0,  20);

ClickEncoder encoder(10, 11, A0);
TimeStampedValue<int16_t> value(0);
int16_t current = 0;
int16_t intensity = 0;

void timerIsr() {
  encoder.service();
}

boolean OledClear = true;

long lastDebounceTime = 0;
long debounceDelay = 1000; // 3000

boolean activeConn = false;
long lastActiveConn = 0;
#define lastActiveDelay 3000 // 10000
boolean bootMode = true;

String inputString = "";
boolean stringComplete = false;

int delayval = 80;
int delayval1 = 30;

const int pinButton = 4;
#define COUNT 15

//#define NOTE_C6  1047
//#define NOTE_CS6 1109
//#define NOTE_D6  1175
//#define NOTE_DS6 1245
//#define NOTE_E6  1319
//#define NOTE_F6  1397
//#define NOTE_FS6 1480
//#define NOTE_G6  1568
//#define NOTE_GS6 1661
//#define NOTE_A6  1760
//#define NOTE_AS6 1865
//#define NOTE_B6  1976

void setup() {
  Serial.begin(9600);
  inputString.reserve(200);

  pinMode(15, OUTPUT);
  pinMode(analogOutPin, OUTPUT);
  digitalWrite(analogOutPin, LOW);

  pinMode (   LUp, INPUT_PULLUP);
  pinMode (    Up, INPUT_PULLUP);
  pinMode (   RUp, INPUT_PULLUP);
  pinMode (  Left, INPUT_PULLUP);
  pinMode (  Down, INPUT_PULLUP);
  pinMode ( Right, INPUT_PULLUP);
  pinMode (Switch, INPUT_PULLUP);

  strip.begin();
  strip.show();

  strip_a.begin();
  strip_a.show();

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 128, 128));
    pixels.show();
    delay(delayval);
  }
  for (int i = 0; i < NUMPIXELS_a; i++) {
    pixels_a.setPixelColor(i, pixels_a.Color(20, 0, 20));
    pixels_a.show();
    delay(delayval1);
  }

  Keyboard.begin();
  Consumer.begin();
  BootKeyboard.begin();

  display.init(SCR_WD, SCR_HT);
  display.fillScreen(BLACK);

  display.setTextColor(YELLOW, BLACK);
  display.setTextSize(2);
  display.setCursor( 122, 16);
  display.print("TOUCH");
  display.setCursor( 130, 36);
  display.print("HERE");
  display.setTextColor(KEYCOLOR6, BLACK);
  display.setCursor( 76, 80);
  display.print(" MACRO ");
  display.setCursor( 72, 100);
  display.print(" Keypad ");
  display.setCursor( 72, 120);
  display.print(" Pro II ");
  display.setCursor( 72, 160);
  display.print("by Hayri");
  display.setCursor( 78, 190);
  display.print("10.2020");

  display.drawBitmap( 51,   3, KEYPADTOP, 138, 6, WHITE);
  display.drawBitmap(129,   3, TOUCH, 46, 3, RED);
  display.drawBitmap( 51,  6, KEYPADSIDE, 3, 167, WHITE);
  display.drawBitmap(186,  6, KEYPADSIDE, 3, 167, WHITE);
  display.drawBitmap( 51, 172, KEYPAD, 138, 64, WHITE);

  // fade in from min to max in increments of 5 points:
  for (int fadeValue = 0 ; fadeValue <= 255; fadeValue += 5) {
    // sets the value (range from 0 to 255):
    analogWrite(3, fadeValue);
    // wait xx amount milliseconds to see the dimming effect
    delay(5);
  }
  delay(6000);
  display.fillScreen(BLACK);
}

void loop() {

  display.setTextSize(2);
  display.setCursor(4, 10);
  display.print("Macro Keypad Pro II");

  (void) LightSensor();

//  display.setTextSize(2);
//  display.setTextColor(WHITE, BLACK);
//  display.setCursor( 182, 38);
//  display.print(LightSensor());
//  display.print(" ");

  //////////////////////////////////////////////////////////////////////////////////
  serialEvent();
  activityChecker();

  /* OLED DRAW STATS */
  if (stringComplete) {

    if (bootMode) {
      bootMode = false;
    }

    lastActiveConn = millis();

    //    pixels.setPixelColor(0, GREEN);  // NeoPixel Idle colour
    //    pixels.show(); // This sends the updated pixel color to the hardware

    display.setTextSize(2);
    display.setCursor(10, 38);
    display.print("CPU");
    display.setCursor(10, 60);
    display.print("SYSRAM");

    //------------------------------------------ CPU Load/Temp -------------------------------------------------
    /*CPU Display String*/
    int cpuStringStart = inputString.indexOf("C");
    int cpuDegree = inputString.indexOf("c");
    int cpuStringLimit = inputString.indexOf("|");
    String cpuString1 = inputString.substring(cpuStringStart + 1, cpuDegree);
    String cpuString2 = inputString.substring(cpuDegree + 1, cpuStringLimit - 1);
    /*CPU LOAD, ALL CORES*/
    display.setTextSize(2);
    display.setCursor(88, 38);
    display.print(cpuString2);
    display.print("  ");
    display.setCursor(136, 38);
    display.print("%");    // Small Percent Symbol
    //------------------------------------------ GPU Load/Temp -------------------------------------------------
    /*GPU Display String*/
    int gpuStringStart = inputString.indexOf("G", cpuStringLimit);  // "G"
    int gpuDegree      = inputString.indexOf("c", gpuStringStart);
    int gpuStringLimit = inputString.indexOf("|", gpuStringStart);
    String gpuString1  = inputString.substring(gpuStringStart + 1, gpuDegree);
    String gpuString2  = inputString.substring(gpuDegree + 1, gpuStringLimit - 1);
    //----------------------------------------SYSTEM RAM USAGE---------------------------------------------------
    /*SYSTEM RAM String*/
    int ramStringStart = inputString.indexOf("R", gpuStringLimit);
    int ramStringLimit = inputString.indexOf("|", ramStringStart);
    String ramString = inputString.substring(ramStringStart + 1 , ramStringLimit);
    /*RAM USAGE*/
    display.setTextSize(2); //set background txt font size
    display.setCursor(100, 60);
    display.println(ramString);

    inputString = "";
    stringComplete = false;
  }
  /////////////////////////////////////////////////////////////////////////////////////////

  current += encoder.getValue();
  auto diff = current - value;
  if (diff != 0) {

    if (diff < 0) {
      intensity = max(1, min(intensity + 1, 10));
      volumeChange(MEDIA_VOL_UP, REP);
      //  font.printStr(ALIGN_CENTER, 44, (char *) "VOLUME UP");
    }
    else {
      intensity = min(-1, max(intensity - 1, -10));
      volumeChange(MEDIA_VOL_DOWN, GREE);
      //  font.printStr(ALIGN_CENTER, 44, (char *) "VOLUME DOWN");
    }
    value.set(current);
  }

  ClickEncoder::Button b = encoder.getButton();
  if (b != ClickEncoder::Open) {
    switch (b) {
      case ClickEncoder::Clicked:
        intensity = 9;
        volumeChange(MEDIA_VOL_MUTE, BLUP);
        value.touch();
        //  font.printStr(ALIGN_CENTER,30, (char *) "MUTE");
        //  display.drawBitmap( 104,   32, MUTE, 35, 46, RED);
        break;
    }
  }
  else {

    if (millis() - value.getTimeStamp() > TIMEOUT_VIBRA_MS) {

    }
    if (millis() - value.getTimeStamp() > TIMEOUT_LIGHTS_MS) {
      setColor(OFF);
      intensity = 1;
    }
  }
}

void volumeChange(uint16_t key, uint32_t color) {
  setColor(color);
  Consumer.write(key);
}

void setColor(uint32_t c) {
  strip_a.setBrightness(abs(intensity) * 255 / 10);
  for (uint16_t i = 0; i < strip_a.numPixels(); i++) {
    strip_a.setPixelColor(i, c);
  }
  strip_a.show();

  // Toggle through pages
  sw_detect = digitalRead(Switch);
  if (sw_detect != sw_old)
  {
    sw_old = sw_detect;
    if (page >= 2)
    {
      page = 0;
    }
    else
    {
      page += 1;
    }
    OledClear = true;
  }
  
  if (page == 0)
  {
    //  strip.setPixelColor(0, 128,   0,  64);  // 1  clockwise counting !!!
    strip.setPixelColor(1,   0, 128, 128);  // 2
    strip.setPixelColor(2, 128,   0,  64);  // 3
    strip.setPixelColor(3,   0, 128, 128);  // 4
    strip.setPixelColor(4,   0, 128, 128);  // 5
    strip.setPixelColor(5,   0, 128, 128);  // 6

    strip.show();
    // Boolean Checks to make sure that we haven't done this before
    if (OledClear == true) {

      Keys();      // clears the text or bitmap on the keys :-)

      display.drawBitmap(   0,   80, KEYCAP, 76, 76, KEYCOLOR1);
      display.drawBitmap(  80,   80, KEYCAP, 76, 76, KEYCOLOR2);
      display.drawBitmap( 160,   80, KEYCAP, 76, 76, KEYCOLOR1);
      display.drawBitmap(   0,  160, KEYCAP, 76, 76, KEYCOLOR2);
      display.drawBitmap(  80,  160, KEYCAP, 76, 76, KEYCOLOR2);
      display.drawBitmap( 160,  160, KEYCAP, 76, 76, KEYCOLOR2);

      display.setTextColor(WHITE, BLACK);
      display.setCursor( 14, 99);
      display.print("Caps");
      display.setCursor( 14, 121);
      display.print("Lock");
      display.drawBitmap(106,  104,   ARROWUP,  24, 29, WHITE);
      display.drawBitmap(191,  106,    PAUSE2,  14, 24, WHITE);
      display.drawBitmap( 23,  186, ARROWLEFT,  29, 24, WHITE);
      display.drawBitmap(106,  185, ARROWDOWN,  24, 29, WHITE);
      display.drawBitmap(185,  186, ARROWRIGHT, 29, 24, WHITE);

      OledClear = false;
    }
    //Up Arrow//
    if (digitalRead(Up) == LOW)
    {
      Keyboard.press(KEY_UP_ARROW);
    //  tone(9, NOTE_A6);
    }
    if (digitalRead(Up) == HIGH)
    {
      Keyboard.release(KEY_UP_ARROW);
    //  noTone(9);
    }

    //Down Arrow//
    if (digitalRead(Down) == LOW)
    {
      Keyboard.press(KEY_DOWN_ARROW);
    //  tone(9, NOTE_E6);
    }
    if (digitalRead(Down) == HIGH)
    {
      Keyboard.release(KEY_DOWN_ARROW);
      //  noTone(9);
    }

    //Right Arrow//
    if (digitalRead(Right) == LOW)
    {
      Keyboard.press(KEY_RIGHT_ARROW);
    //  tone(9, NOTE_D6);
    }
    if (digitalRead(Right) == HIGH)
    {
      Keyboard.release(KEY_RIGHT_ARROW);
      //  noTone(9);
    }

    //Left Arrow//
    if (digitalRead(Left) == LOW)
    {
      Keyboard.press(KEY_LEFT_ARROW);
    //  tone(9, NOTE_F6);
    }
    if (digitalRead(Left) == HIGH)
    {
      Keyboard.release(KEY_LEFT_ARROW);
      //  noTone(9);
    }

    //Top Right Button//
    //      if (digitalRead(RUp) == LOW)
    //      {
    //      //  Keyboard.press(KEY_BACKSPACE);
    //        Consumer.write(MEDIA_PLAY_PAUSE);
    //        display.drawBitmap( 170,   32, PAUSE, 50, 50, RED);
    //        delay(300);
    //      }
    //      if (digitalRead(RUp) == HIGH)
    //      {
    //      //  Keyboard.release(KEY_BACKSPACE);
    //        Keyboard.releaseAll();
    //        display.drawBitmap(   170,   32, PAUSE, 50, 50, BLACK);
    //      }

    if (!digitalRead(6)) {
      //   digitalWrite(pinLed, HIGH);

      Consumer.write(MEDIA_PLAY_PAUSE);
      display.drawBitmap( 191, 106, PAUSE2, 14, 24, RED);
      delay(300);

      //   digitalWrite(pinLed, LOW);
      display.drawBitmap( 191, 106, PAUSE2, 14, 24, WHITE);
    }

    //Top Left Button//
    //      if (digitalRead(LUp) == LOW)
    //      {
    //        Keyboard.press(KEY_DELETE);
    //      }
    //      if (digitalRead(LUp) == HIGH)
    //      {
    //        Keyboard.release(KEY_DELETE);
    //      }

    if (BootKeyboard.getLeds() & LED_CAPS_LOCK) {
      strip.setPixelColor(0, 255,   0,  0);  // 1
      //      strip.setPixelColor(1, 255,   0,  0);  // 2
      //      strip.setPixelColor(2, 255,   0,  0);  // 3
      //      strip.setPixelColor(3, 255,   0,  0);  // 4
      //      strip.setPixelColor(4, 255,   0,  0);  // 5
      //      strip.setPixelColor(5, 255,   0,  0);  // 6
      strip.show();
      //      display.setTextColor(RED);
      //      display.setCursor( 14, 103);
      //      display.print("Caps");
      //      display.setCursor( 14, 125);
      //      display.print("Lock");
    }
    else

      strip.setPixelColor(0, 128,   0,  64);  // 1
    //      strip.setPixelColor(1,   0, 128, 128);  // 2
    //      strip.setPixelColor(2, 128,   0,  64);  // 3
    //      strip.setPixelColor(3,   0, 128, 128);  // 4
    //      strip.setPixelColor(4,   0, 128, 128);  // 5
    //      strip.setPixelColor(5,   0, 128, 128);  // 6
    strip.show();

    // Trigger caps lock manually via button
    if (!digitalRead(pinButton)) {
      BootKeyboard.write(KEY_CAPS_LOCK);
      //

      delay(300);    // Simple debounce
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////

  // Second Layer Programming

  else if (page == 1)
  {
    strip.setPixelColor(0, 128, 128, 0);
    strip.setPixelColor(1, 128, 128, 0);
    strip.setPixelColor(2, 128, 128, 0);
    strip.setPixelColor(3, 128, 0, 128);
    strip.setPixelColor(4, 128, 0, 128);
    strip.setPixelColor(5, 128, 0, 128);

    strip.show();

    // Make sure that we aren't constantly clearing the display
    if (OledClear == true)
    {
      Keys();

      display.drawBitmap(   0,   80, KEYCAP, 76, 76, KEYCOLOR3);
      display.drawBitmap(  80,   80, KEYCAP, 76, 76, KEYCOLOR3);
      display.drawBitmap( 160,   80, KEYCAP, 76, 76, KEYCOLOR3);
      display.drawBitmap(   0,  160, KEYCAP, 76, 76, KEYCOLOR1);
      display.drawBitmap(  80,  160, KEYCAP, 76, 76, KEYCOLOR1);
      display.drawBitmap( 160,  160, KEYCAP, 76, 76, KEYCOLOR1);

      display.setCursor( 21, 111);
      display.print("ESC");
      display.setCursor(94, 111);
      display.print("HOME");
      display.setCursor(174, 111);  // 185, 108
      display.print("PRSC");
      display.setCursor( 14, 191);
      display.print("DSLA");
      display.setCursor(101, 191);
      display.print("END");
      display.setCursor(174, 191);
      display.print("SAVE");

      OledClear = false;
    }

    //Up Arrow//
    if (digitalRead(Up) == LOW)
    {
      Keyboard.press(KEY_HOME);
    }
    if (digitalRead(Up) == HIGH)
    {
      Keyboard.release(KEY_HOME);
    }

    //Down Arrow//
    if (digitalRead(Down) == LOW)
    {
      Keyboard.press(KEY_END);
    }
    if (digitalRead(Down) == HIGH)
    {
      Keyboard.release(KEY_END);
    }

    //Right Arrow//
    if (digitalRead(Right) == LOW)
    {
      Keyboard.press(KEY_RIGHT_CTRL);
      Keyboard.press('s');
    }
    if (digitalRead(Right) == HIGH)
    {
      Keyboard.releaseAll();
    }

    //Left Arrow//
    if (digitalRead(Left) == LOW)
    {
      Keyboard.print("display last");
      delay(1000);
    }
    if (digitalRead(Left) == HIGH)
    {
      Keyboard.releaseAll();
    }

    //Top Right Button//
    if (digitalRead(RUp) == LOW)
    {
      Keyboard.press(KEY_PRINTSCREEN);  // print screen, finally working AGAIN :-)))))
    }
    if (digitalRead(RUp) == HIGH)
    {
      Keyboard.release(KEY_PRINTSCREEN);
    }

    //Top Left Button//
    if (digitalRead(LUp) == LOW)
    {
      Keyboard.press(KEY_ESC);
    }
    if (digitalRead(LUp) == HIGH)
    {
      Keyboard.release(KEY_ESC);
    }
  }
  
  ////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////

  // Third Layer Programming
  else
  {
    strip.setPixelColor(0, 128, 0, 128);
    strip.setPixelColor(1, 0, 128, 0);
    strip.setPixelColor(2, 128, 0, 128);
    strip.setPixelColor(3, 0, 128, 0);
    strip.setPixelColor(4, 0, 128, 0);
    strip.setPixelColor(5, 0, 128, 0);

    strip.show();

    // Make sure that we aren't constantly clearing the display
    if (OledClear == true)
    {
      Keys();

      display.drawBitmap(   0,   80, KEYCAP, 76, 76, KEYCOLOR1);
      display.drawBitmap(  80,   80, KEYCAP, 76, 76, KEYCOLOR4);
      display.drawBitmap( 160,   80, KEYCAP, 76, 76, KEYCOLOR1);
      display.drawBitmap(   0,  160, KEYCAP, 76, 76, KEYCOLOR4);
      display.drawBitmap(  80,  160, KEYCAP, 76, 76, KEYCOLOR4);
      display.drawBitmap( 160,  160, KEYCAP, 76, 76, KEYCOLOR4);

      display.setCursor( 33, 111);
      display.print("Q");
      display.setCursor(113, 111);
      display.print("W");
      display.setCursor(193, 111);
      display.print("E");
      display.setCursor( 33, 191);
      display.print("A");
      display.setCursor(113, 191);
      display.print("S");
      display.setCursor(193, 191);
      display.print("D");

      OledClear = false;
    }

    //Up Arrow//
    if (digitalRead(Up) == LOW)
    {
      Keyboard.press(KEY_W);
    }
    if (digitalRead(Up) == HIGH)
    {
      Keyboard.release(KEY_W);
    }

    //Down Arrow//
    if (digitalRead(Down) == LOW)
    {
      Keyboard.press(KEY_S);
    }
    if (digitalRead(Down) == HIGH)
    {
      Keyboard.release(KEY_S);
    }

    //Right Arrow//
    if (digitalRead(Right) == LOW)
    {
      Keyboard.press(KEY_D);
    }
    if (digitalRead(Right) == HIGH)
    {
      Keyboard.release(KEY_D);
    }

    //Left Arrow//
    if (digitalRead(Left) == LOW)
    {
      Keyboard.press(KEY_A);
    }
    if (digitalRead(Left) == HIGH)
    {
      Keyboard.release(KEY_A);
    }

    //Top Right Button//
    if (digitalRead(RUp) == LOW)
    {
      Keyboard.press(KEY_E);
    }
    if (digitalRead(RUp) == HIGH)
    {
      Keyboard.release(KEY_E);
    }

    //Top Left Button//
    if (digitalRead(LUp) == LOW)
    {
      Keyboard.press(KEY_Q);
    }
    if (digitalRead(LUp) == HIGH)
    {
      Keyboard.release(KEY_Q);
    }
  }
}

void serialEvent() {

  while (Serial.available()) {          //  32u4 USB Serial Available?
    char inChar = (char)Serial.read();  // Read 32u4 USB Serial

    inputString += inChar;
    if (inChar == '|') {
      stringComplete = true;
    }
  }
}
void activityChecker() {
  if (millis() - lastActiveConn > lastActiveDelay)

    activeConn = false;
  else
    activeConn = true;
  if (!activeConn) {

    //Turn off NeoPixel when there is no activity
    //    pixels.setPixelColor(0, BLACK);
    //    pixels.show();

  }
}

void  Keys() {
  display.fillRect( 14,  95, 48, 46, BLACK);
  display.fillRect( 94,  95, 48, 46, BLACK);
  display.fillRect(174,  95, 48, 46, BLACK);
  display.fillRect( 14, 175, 48, 46, BLACK);
  display.fillRect( 94, 175, 48, 46, BLACK);
  display.fillRect(174, 175, 48, 46, BLACK);
}

//**************************************************************************** lightSensor()
int LightSensor() {
  int sensorValue = analogRead(analogInPin);
  int outputValue = map(sensorValue, 0, 1023, 10, 255);
  analogWrite(analogOutPin, outputValue);

  //  // if (analogRead(A0) >= 0) {
  //  int NeoBrightness = map(sensorValue, 0, 1023, 10, 250);
  //  strip.setBrightness(NeoBrightness);
  //
  //  Red       = 24;
  //  Green     = 14;
  //  Blue      = 24;

  return sensorValue;
}
