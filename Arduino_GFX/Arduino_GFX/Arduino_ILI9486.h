/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 */
#ifndef _ARDUINO_ILI9486_H_
#define _ARDUINO_ILI9486_H_

#include "Arduino.h"
#include "Print.h"
#include "Arduino_GFX.h"
#include "Arduino_TFT.h"
#include "Arduino_TFT_18bit.h"

#define ILI9486_TFTWIDTH 320  ///< ILI9486 max TFT width
#define ILI9486_TFTHEIGHT 480 ///< ILI9486 max TFT height

#define ILI9486_RST_DELAY 120    ///< delay ms wait for reset finish
#define ILI9486_SLPIN_DELAY 120  ///< delay ms wait for sleep in finish
#define ILI9486_SLPOUT_DELAY 120 ///< delay ms wait for sleep out finish

// Generic commands used by ILI9486_eSPI.cpp
#define ILI9486_NOP     0x00
#define ILI9486_SWRST   0x01

#define ILI9486_SLPIN   0x10
#define ILI9486_SLPOUT  0x11

#define ILI9486_INVOFF  0x20
#define ILI9486_INVON   0x21

#define ILI9486_DISPOFF 0x28
#define ILI9486_DISPON  0x29

#define ILI9486_CASET   0x2A
#define ILI9486_PASET   0x2B
#define ILI9486_RAMWR   0x2C

#define ILI9486_RAMRD   0x2E

#define ILI9486_MADCTL  0x36

#define ILI9486_MAD_MY  0x80
#define ILI9486_MAD_MX  0x40
#define ILI9486_MAD_MV  0x20
#define ILI9486_MAD_ML  0x10
#define ILI9486_MAD_RGB 0x00
#define ILI9486_MAD_BGR 0x08
#define ILI9486_MAD_MH  0x04
#define ILI9486_MAD_SS  0x02
#define ILI9486_MAD_GS  0x01

class Arduino_ILI9486 : public Arduino_TFT_18bit
{
public:
  Arduino_ILI9486(Arduino_DataBus *bus, int8_t rst = -1, uint8_t r = 0);

  virtual void begin(uint32_t speed = 0);
  virtual void writeAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
  virtual void setRotation(uint8_t r);
  virtual void invertDisplay(bool);
  virtual void displayOn();
  virtual void displayOff();

protected:
  virtual void tftInit();

private:
};

#endif
