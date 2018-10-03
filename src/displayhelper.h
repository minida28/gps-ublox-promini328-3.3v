#ifndef displayhelper_h
#define displayhelper_h

#include <Arduino.h>
// #include <U8g2lib.h>
#include <U8x8lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

extern U8X8_SH1106_128X64_NONAME_HW_I2C u8x8;

void DisplaySetup();
void DisplayLoop();

#endif