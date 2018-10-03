#ifndef _gpshelper_h
#define _gpshelper_h

#include <Arduino.h>
// #include <NMEAGPS.h>
// #include <GPSport.h>
// #include <Streamers.h>
#include <NeoTeeStream.h>

// #include "displayhelper.h"

//======================================================================
//  Program: NMEAsimple.ino
//
//  Description:  This program shows simple usage of NeoGPS
//
//  Prerequisites:
//     1) NMEA.ino works with your device (correct TX/RX pins and baud rate)
//     2) At least one of the RMC, GGA or GLL sentences have been enabled in NMEAGPS_cfg.h.
//     3) Your device at least one of those sentences (use NMEAorder.ino to confirm).
//     4) LAST_SENTENCE_IN_INTERVAL has been set to one of those sentences in NMEAGPS_cfg.h (use NMEAorder.ino).
//     5) LOCATION and ALTITUDE have been enabled in GPSfix_cfg.h
//
//  'Serial' is for debug output to the Serial Monitor window.
//
//  License:
//    Copyright (C) 2014-2017, SlashDevin
//
//    This file is part of NeoGPS
//
//    NeoGPS is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    NeoGPS is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with NeoGPS.  If not, see <http://www.gnu.org/licenses/>.
//
//======================================================================

//    Enter the following commands through the Serial Monitor window:
//
//   '?'  - dislay this help message
//   '0'  - send NMEA PUBX text command to disable all sentences
//   '1'  - send NMEA PUBX text command to enable all sentences
//   'd'  - send UBX binary command to disable all sentences except GLL
//   'r1' - send UBX binary command to set update rate to 1Hz
//   'r2' - send UBX binary command to set update rate to 2Hz
//   'r3' - send UBX binary command to set update rate to 5Hz
//   're' - send UBX binary command to reset the GPS device (cold start)
//   '5'  - send NMEA PUBX text command to set baud rate to 115200
//   '7'  - send NMEA PUBX text command to set baud rate to 57600
//   '3'  - send NMEA PUBX text command to set baud rate to 38400
//   '9'  - send NMEA PUBX text command to set baud rate to 9600
//   'e'  - toggle echo of all characters received from GPS device.
//   't'  - toggle tracing of parsed GPS fields.


// U-blox UBX binary commands

const unsigned char ubxRate1Hz[] PROGMEM =
    {0x06, 0x08, 0x06, 0x00, 0xE8, 0x03, 0x01, 0x00, 0x01, 0x00};
const unsigned char ubxRate2Hz[] PROGMEM =
    {0x06, 0x08, 0x06, 0x00, 0xF4, 0x01, 0x01, 0x00, 0x01, 0x00};
const unsigned char ubxRate5Hz[] PROGMEM =
    {0x06, 0x08, 0x06, 0x00, 0xC8, 0x00, 0x01, 0x00, 0x01, 0x00};

// Disable specific NMEA sentences
const unsigned char ubxDisableGGA[] PROGMEM =
    {0x06, 0x01, 0x08, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
const unsigned char ubxDisableGLL[] PROGMEM =
    {0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
const unsigned char ubxDisableGSA[] PROGMEM =
    {0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
const unsigned char ubxDisableGSV[] PROGMEM =
    {0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
const unsigned char ubxDisableRMC[] PROGMEM =
    {0x06, 0x01, 0x08, 0x00, 0xF0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
const unsigned char ubxDisableVTG[] PROGMEM =
    {0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
const unsigned char ubxDisableZDA[] PROGMEM =
    {0x06, 0x01, 0x08, 0x00, 0xF0, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

//-------------------------------------------
// U-blox NMEA text commands

const char baud9600[] PROGMEM = "PUBX,41,1,3,3,9600,0";
const char baud38400[] PROGMEM = "PUBX,41,1,3,3,38400,0";
const char baud57600[] PROGMEM = "PUBX,41,1,3,3,57600,0";
const char baud115200[] PROGMEM = "PUBX,41,1,3,3,115200,0";

//--------------------------

// using namespace NeoGPS;


// extern NMEAGPS gps;                           // This parses the GPS characters
// extern gps_fix fix;                           // This holds on to the latest values
extern uint8_t LastSentenceInInterval; // storage for the run-time selection

//  Use NeoTee to echo the NMEA text commands to the Serial Monitor window
// extern Stream *both[];
// static Stream *both[2] = {&DEBUG_PORT, &gpsPort};
// extern NeoTeeStream tee;


extern char lastChar; // last command char
extern bool echoing;
extern bool tracing;

// extern uint8_t GPSFixStatus;
extern bool validGPSTimeFlag;
extern bool validGPSDateFlag;
extern bool validGPSLocationFlag;
extern bool validGPSAltitudeFlag;
extern bool validGPSSpeedFlag;
extern bool parseGPSCompleted;

extern uint32_t GPStimestamp;

void sendUBX(const unsigned char *progmemBytes, size_t len);
void changeBaud(const char *textCommand, unsigned long baud);
uint8_t GPSFixStatus();
// uint8_t GPSSatellites();
// uint8_t GPSSatellitesCount();
float GPSLatitude();
float GPSLongitude();
float GPSSpeedKPH();
void doSomeWork();
void GPSSetup();
void GPSLoop();

#endif