#include "displayhelper.h"
// #include "timehelper.h"
// #include "sholathelper.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define DEBUGPORT Serial

#define RELEASE

#ifndef RELEASE
#define DEBUGLOG(fmt, ...)                       \
    {                                            \
        static const char pfmt[] PROGMEM = fmt;  \
        DEBUGPORT.printf_P(pfmt, ##__VA_ARGS__); \
    }
#else
#define DEBUGLOG(...)
#define DEBUGLOGLN(...)
#endif

U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE);

void DisplaySetup()
{
    u8x8.begin();
    u8x8.clear();
    u8x8.setPowerSave(0);
}

void DisplayLoop()
{

    // picture loop
    // must be after sholat time has been updated
    // u8g2.clearBuffer();

    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.drawString(0, 0, "Hello World!");
}