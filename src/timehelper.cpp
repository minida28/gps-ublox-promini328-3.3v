#include <Arduino.h>
#include <time.h>
#include "timehelper.h"
// #include "sntphelper.h"
// #include "rtchelper.h"

#define DEBUGPORT Serial

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

bool tick1000ms = false;

bool NTP_OK = false;

time_t utcTime, localTime;
time_t _lastSyncd; ///< Stored time of last successful sync
time_t _firstSync; ///< Stored time of first successful sync after boot
time_t _lastBoot;

uint16_t syncInterval;      ///< Interval to set periodic time sync
uint16_t shortSyncInterval; ///< Interval to set periodic time sync until first synchronization.
uint16_t longSyncInterval;  ///< Interval to set periodic time sync

strConfigTime configTime;
// TIMESOURCE _timeSource;

int32_t TimezoneMinutes()
{
  return configLocation.timezone * 60;
}

int32_t TimezoneSeconds()
{
  return TimezoneMinutes() * 60;
}

char *getDateStr(time_t rawtime) // Thu Aug 23 2001
{
    DEBUGLOG("%s\r\n", __PRETTY_FUNCTION__);
    static char buf[16];
    // time_t rawtime;
    // time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);
    strftime(buf, sizeof(buf), "%a %b %d %Y", timeinfo);

    return buf;
}

char *getTimeStr(time_t rawtime) 
{
    DEBUGLOG("%s\r\n", __PRETTY_FUNCTION__);
    static char buf[12];
    // time_t rawtime;
    // time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);
    strftime(buf, sizeof(buf), "%T", timeinfo); //02:55:02
    // strftime(buf, sizeof(buf), "%r", timeinfo); //02:55:02 pm

    return buf;
}

char *getUptimeStr()
{
    DEBUGLOG("%s\r\n", __PRETTY_FUNCTION__);
    //time_t uptime = utcTime - _lastBoot;
    time_t uptime = millis() / 1000;

    uint16_t days;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;

    struct tm *tm = gmtime(&uptime); // convert to broken down time
    days = tm->tm_yday;
    hours = tm->tm_hour;
    minutes = tm->tm_min;
    seconds = tm->tm_sec;

    static char buf[30];
    snprintf_P(buf, sizeof(buf), PSTR("%u days %02d:%02d:%02d"), days, hours, minutes, seconds);

    return buf;
}

char *getLastSyncStr()
{
    DEBUGLOG("%s\r\n", __PRETTY_FUNCTION__);

    time_t diff = time(nullptr) - _lastSyncd;

    uint16_t days;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;

    struct tm *tm = gmtime(&diff); // convert to broken down time
    days = tm->tm_yday;
    hours = tm->tm_hour;
    minutes = tm->tm_min;
    seconds = tm->tm_sec;

    static char buf[30];
    if (days > 0)
    {
        snprintf_P(buf, sizeof(buf), PSTR("%u day %d hr ago"), days, hours);
    }
    else if (hours > 0)
    {
        snprintf_P(buf, sizeof(buf), PSTR("%d hr %d min ago"), hours, minutes);
    }
    else if (minutes > 0)
    {
        snprintf_P(buf, sizeof(buf), PSTR("%d min ago"), minutes);
    }
    else
    {
        snprintf_P(buf, sizeof(buf), PSTR("%d sec ago"), seconds);
    }

    return buf;
}

char *getNextSyncStr()
{
    DEBUGLOG("%s\r\n", __PRETTY_FUNCTION__);

    time_t nextsync;

    nextsync = _lastSyncd - utcTime + syncInterval;

    uint16_t days;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;

    struct tm *tm = gmtime(&nextsync); // convert to broken down time
    days = tm->tm_yday;
    hours = tm->tm_hour;
    minutes = tm->tm_min;
    seconds = tm->tm_sec;

    static char buf[30];
    snprintf_P(buf, sizeof(buf), PSTR("%u days %02d:%02d:%02d"), days, hours, minutes, seconds);

    return buf;
}

void TimeSetup()
{
    // Synchronize time useing SNTP. This is necessary to verify that
    // the TLS certificates offered by the server are currently valid.
    //   Serial.print("Setting time using SNTP");

    //   configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");

    time_t now;
    struct tm *timeinfo;

    // time(&rawtime);
    // timeinfo = localtime (&rawtime);

    // time(&now); /* get current time; same as: now = time(NULL)  */
    now = time(nullptr);
    // timeinfo = gmtime(&now);
    timeinfo = localtime(&now);

    timeinfo->tm_year = 2018 - 1900;
    timeinfo->tm_mon = 9 - 1;
    timeinfo->tm_mday = 27;

    mktime(timeinfo);

    DEBUGLOG("Current time: ");
    Serial.println(asctime(timeinfo));
}