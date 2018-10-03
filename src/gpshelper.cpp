#include "gpshelper.h"
#include <NMEAGPS.h>
#include <GPSport.h>

#define RELEASE

#ifndef RELEASE
#define DEBUGLOG(fmt, ...)                      \
    {                                           \
        static const char pfmt[] PROGMEM = fmt; \
        printf_P(pfmt, ##__VA_ARGS__);          \
    }
#else
#define DEBUGLOG(...)
#endif

NMEAGPS gps;                           // This parses the GPS characters
gps_fix fix;                           // This holds on to the latest values
uint8_t LastSentenceInInterval = 0xFF; // storage for the run-time selection

char lastChar; // last command char
bool echoing = false;
bool tracing = true;

// uint8_t GPSFixStatus = 0;
bool validGPSTimeFlag = false;
bool validGPSDateFlag = false;
bool validGPSLocationFlag = false;
bool validGPSAltitudeFlag = false;
bool validGPSSpeedFlag = false;
bool parseGPSCompleted = false;

uint32_t GPStimestamp = 0;

//  Use NeoTee to echo the NMEA text commands to the Serial Monitor window
Stream *both[2] = {&DEBUG_PORT, &gpsPort};
NeoTeeStream tee(both, sizeof(both) / sizeof(both[0]));

//-------------------------------------------

void sendUBX(const unsigned char *progmemBytes, size_t len)
{
    gpsPort.write(0xB5); // SYNC1
    gpsPort.write(0x62); // SYNC2

    uint8_t a = 0, b = 0;
    while (len-- > 0)
    {
        uint8_t c = pgm_read_byte(progmemBytes++);
        a += c;
        b += a;
        gpsPort.write(c);
    }

    gpsPort.write(a); // CHECKSUM A
    gpsPort.write(b); // CHECKSUM B

} // sendUBX

const uint32_t COMMAND_DELAY = 250;

void changeBaud(const char *textCommand, unsigned long baud)
{
    gps.send_P(&tee, (const __FlashStringHelper *)textCommand);
    gpsPort.flush();
    gpsPort.end();

    // DEBUG_PORT.print(F("Baud rate change to "));
    DEBUGLOG("Baud rate change to ");
    DEBUGLOG("%d\r\n", baud);
    delay(500);
    gpsPort.begin(baud);

} // changeBaud

//--------------------------

void doSomeWork()
{
    // Print all the things!

    if (tracing)
    {
        // trace_all(DEBUG_PORT, gps, fix);
    }

} // doSomeWork

uint8_t GPSFixStatus()
{
    return fix.status;
}

// uint8_t GPSSatellites()
// {
//     return fix.satellites;
// }

// uint8_t GPSSatellitesCount()
// {
//     return gps.sat_count;
// }

float GPSLatitude()
{
    return fix.latitude();
}

float GPSLongitude()
{
    return fix.longitude();
}

float GPSSpeedKPH()
{
    return fix.speed_kph();
}


void GPSSetup()
{
    tracing = false;
    echoing = 0;
    LastSentenceInInterval = NMEAGPS::NMEA_GLL;

    // trace_header(DEBUG_PORT);
    DEBUG_PORT.flush();

    gpsPort.begin(38400);
}

void GPSLoop()
{
    if (DEBUG_PORT.available())
    {
        char c = DEBUG_PORT.read();

        switch (c)
        {
        case '?':
            echoing = false;
            tracing = false;
            DEBUG_PORT.print(F("Enter command> "));
            break;

        case '0':
            // LastSentenceInInterval = NMEAGPS::NMEA_GLL;
            break;

        case '1':
            if (lastChar == 'b')
            {
                changeBaud(baud9600, 9600UL);
            }
            else if (lastChar == 'r')
            {
                sendUBX(ubxRate1Hz, sizeof(ubxRate1Hz));
            }
            else
            {
                // LastSentenceInInterval = NMEAGPS::NMEA_GLL;
            }
            break;

        case '2':
            if (lastChar == 'b')
            {
                changeBaud(baud38400, 38400UL);
            }
            else if (lastChar == 'r')
            {
                sendUBX(ubxRate2Hz, sizeof(ubxRate2Hz));
            }
            else
            {
                // LastSentenceInInterval = NMEAGPS::NMEA_GLL;
            }
            break;

        case '3':
            if (lastChar == 'b')
            {
                changeBaud(baud57600, 57600UL);
            }
            else if (lastChar == 'r')
            {
                sendUBX(ubxRate5Hz, sizeof(ubxRate5Hz));
            }
            else
            {
                // LastSentenceInInterval = NMEAGPS::NMEA_GLL;
            }
            break;

        case 'd':
            LastSentenceInInterval = NMEAGPS::NMEA_GLL;
            break;

        case 'e':
            if (lastChar == 'r')
            {
                // resetGPS();
            }
            else
            {
                echoing = !echoing;
            }
            break;

        case 't':
            tracing = !tracing;
            break;

        default:
            break;
        }
        lastChar = c;
    }

    //  Check for GPS data

    static bool displayingHex = false;

    if (echoing)
    {
        // Use advanced character-oriented methods to echo received characters to
        //    the Serial Monitor window.
        // if (gpsPort.available())
        while (gpsPort.available())
        {

            char c = gpsPort.read();

            if (((' ' <= c) && (c <= '~')) || (c == '\r') || (c == '\n'))
            {
                DEBUG_PORT.write(c);
                displayingHex = false;
            }
            else
            {
                if (!displayingHex)
                {
                    displayingHex = true;
                    DEBUG_PORT.print(F("0x"));
                }
                if (c < 0x10)
                    DEBUG_PORT.write('0');
                DEBUG_PORT.print((uint8_t)c, HEX);
                DEBUG_PORT.write(' ');
            }

            gps.handle(c);
            // if (gps.available())
            while (gps.available())
            {
                fix = gps.read();

                if (displayingHex)
                    displayingHex = false;
                DEBUG_PORT.println();

                parseGPSCompleted = true;

                if (fix.valid.time && fix.valid.date)
                {
                    GPStimestamp = (NeoGPS::clock_t)fix.dateTime;
                    Serial.println(GPStimestamp);
                }

                // GPSFixStatus = fix.status;
                validGPSTimeFlag = fix.valid.time;
                validGPSDateFlag = fix.valid.date;
                validGPSLocationFlag = fix.valid.location;
                // validGPSAltitudeFlag = fix.valid.altitude;
                validGPSSpeedFlag = fix.valid.speed;

                doSomeWork();
            }
        }
    }
    else
    {

        // Use the normal fix-oriented methods to display fixes
        // if (gps.available(gpsPort))
        while (gps.available(gpsPort))
        {
            fix = gps.read();
            doSomeWork();

            parseGPSCompleted = true;

            if (fix.valid.time && fix.valid.date)
            {
                GPStimestamp = (NeoGPS::clock_t)fix.dateTime;
            }

            // GPSFixStatus = fix.status;
            validGPSTimeFlag = fix.valid.time;
            validGPSDateFlag = fix.valid.date;
            validGPSLocationFlag = fix.valid.location;
            // validGPSAltitudeFlag = fix.valid.altitude;
            validGPSSpeedFlag = fix.valid.speed;

            displayingHex = false;
        }
    }
}