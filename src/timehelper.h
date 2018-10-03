#ifndef timehelper_h
#define timehelper_h

#include <Arduino.h>
// #include "gpshelper.h"
#include <time.h>
// #include <sys/time.h>  // struct timeval
// #include <coredecls.h> // settimeofday_cb()
// #include "rtchelper.h"
// #include <Ticker.h>
#include "locationhelper.h"

/*==============================================================================*/
/* Useful Constants */
#define SECS_PER_MIN  ((time_t)(60UL))
#define SECS_PER_HOUR ((time_t)(3600UL))
#define SECS_PER_DAY  ((time_t)(SECS_PER_HOUR * 24UL))
#define DAYS_PER_WEEK ((time_t)(7UL))
#define SECS_PER_WEEK ((time_t)(SECS_PER_DAY * DAYS_PER_WEEK))
#define SECS_PER_YEAR ((time_t)(SECS_PER_DAY * 365UL)) // TODO: ought to handle leap years
#define SECS_YR_2000  ((time_t)(946684800UL)) // the time at the start of y2k
 
/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) ((_time_) % SECS_PER_MIN)  
#define numberOfMinutes(_time_) (((_time_) / SECS_PER_MIN) % SECS_PER_MIN) 
#define numberOfHours(_time_) (((_time_) % SECS_PER_DAY) / SECS_PER_HOUR)
#define dayOfWeek(_time_) ((((_time_) / SECS_PER_DAY + 4)  % DAYS_PER_WEEK)+1) // 1 = Sunday
#define elapsedDays(_time_) ((_time_) / SECS_PER_DAY)  // this is number of days since Jan 1 1970
#define elapsedSecsToday(_time_) ((_time_) % SECS_PER_DAY)   // the number of seconds since last midnight 
// The following macros are used in calculating alarms and assume the clock is set to a date later than Jan 1 1971
// Always set the correct time before settting alarms
#define previousMidnight(_time_) (((_time_) / SECS_PER_DAY) * SECS_PER_DAY)  // time at the start of the given day
#define nextMidnight(_time_) (previousMidnight(_time_)  + SECS_PER_DAY)   // time at the end of the given day 
#define elapsedSecsThisWeek(_time_) (elapsedSecsToday(_time_) +  ((dayOfWeek(_time_)-1) * SECS_PER_DAY))   // note that week starts on day 1
#define previousSunday(_time_) ((_time_) - elapsedSecsThisWeek(_time_))      // time at the start of the week for the given time
#define nextSunday(_time_) (previousSunday(_time_)+SECS_PER_WEEK)          // time at the end of the week for the given time

// for testing purpose:
// extern "C" int clock_gettime(clockid_t unused, struct timespec *tp);

extern bool tick1000ms;
extern bool tick3000ms;
extern bool state500ms;

extern bool timeSetFlag;

extern uint32_t utcTime;
extern uint32_t localTime;
extern uint32_t lastSync; ///< Stored time of last successful sync
extern uint32_t _firstSync; ///< Stored time of first successful sync after boot
extern uint32_t _lastBoot;

typedef struct
{
  // float timezone = 7.0;
  bool dst = false;
  // bool enablentp = true;
  // char ntpserver_0[48] = "0.id.pool.ntp.org";
  // char ntpserver_1[48] = "0.asia.pool.ntp.org";
  // char ntpserver_2[48] = "192.168.10.1";
  // bool enablertc = true;
  uint32_t syncinterval = 600;
} strConfigTime;
extern strConfigTime configTime;

typedef enum timeSource
{
  TIMESOURCE_NOT_AVAILABLE,
  TIMESOURCE_NTP,
  TIMESOURCE_RTC
} strTimeSource;
extern strTimeSource timeSource;

float TimezoneFloat();
int32_t TimezoneMinutes();
int32_t TimezoneSeconds();

char *getDateStr(time_t rawtime);
char *getTimeStr(time_t rawtime);
char *getDateTimeStr(uint32_t moment);
// char *GetRtcDateTimeStr(const RtcDateTime &dt);
char *getLastBootStr();
char *getUptimeStr();
char *getLastSyncStr();
char *getNextSyncStr();

/* date strings */ 
// #define dt_MAX_STRING_LEN 9 // length of longest date string (excluding terminating null)
char* monthStr(uint8_t month);
char* dayStr(uint8_t day);
char* monthShortStr(uint8_t month);
char* dayShortStr(uint8_t day);

time_t tmConvert_t(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss);
// unsigned long tmConvert_t(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss);

void TimeSetup();
void TimeLoop();

#endif