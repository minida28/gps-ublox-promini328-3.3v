#include <Arduino.h>
#include "sholat.h"
#include "sholathelper.h"
#include "timehelper.h"
// #include "locationhelper.h"
// #include "progmemmatrix.h"

#define PRINTPORT Serial
#define DEBUGPORT Serial

#define RELEASE

#ifndef RELEASE
#define PRINT(fmt, ...)                     \
  {                                         \
    static const char pfmt[] PROGMEM = fmt; \
    printf_P(pfmt, ##__VA_ARGS__);          \
  }

#define DEBUGLOG(fmt, ...)                    \
  {                                           \
    static const char pfmt[] PROGMEM_T = fmt; \
    DEBUGPORT.printf_P(pfmt, ##__VA_ARGS__);  \
  }
#else
#define DEBUGLOG(...)
#define PRINT(...)
#endif

PrayerTimes sholat;

uint8_t HOUR;
uint8_t MINUTE;
uint8_t SECOND;
uint8_t ceilHOUR;
uint8_t ceilMINUTE;
uint8_t CURRENTTIMEID, NEXTTIMEID;

char bufHOUR[3];
char bufMINUTE[3];
char bufSECOND[3];

time_t currentSholatTime = 0;
time_t nextSholatTime = 0;

uint32_t timestampSholatTimesYesterday[TimesCount];
uint32_t timestampSholatTimesToday[TimesCount];
uint32_t timestampSholatTimesTomorrow[TimesCount];
uint32_t timestampPreviousSholatTime;
uint32_t timestampCurrentSholatTime;
uint32_t timestampNextSholatTime;

char sholatTimeYesterdayArray[TimesCount][6];
char sholatTimeArray[TimesCount][6];
char sholatTimeTomorrowArray[TimesCount][6];

// char bufCommonSholat[30];

char *sholatNameStr(uint8_t id)
{
  static char buf[30];
  // time_t t_utc = time(nullptr);
  struct tm *tm_local = localtime(&utcTime);

  if (tm_local->tm_wday == 5 && id == Dhuhr)
  {
    char JUMUAH[] = "JUMAT";
    strcpy(buf, JUMUAH);
  }
  else
  {
    const char *ptr = (const char *)pgm_read_word(&(sholatName_P[id]));
    strcpy_P(buf, ptr);
    // uint8_t len = strlen_P(sholatName_P[id]);
    // buf[len+1] = '\0';
  }

  return buf;
}

void process_sholat()
{
  DEBUGLOG("\n%s\r\n", __PRETTY_FUNCTION__);

  // char buf[64];

  sholat.set_calc_method(_sholatConfig.calcMethod);
  sholat.set_asr_method(_sholatConfig.asrJuristic);
  sholat.set_high_lats_adjust_method(_sholatConfig.highLatsAdjustMethod);

  if (_sholatConfig.calcMethod == Custom)
  {
    sholat.set_fajr_angle(_sholatConfig.fajrAngle);
    //sholat.set_maghrib_angle(_sholatConfig.maghribAngle);
    sholat.set_maghrib_minutes(_sholatConfig.maghribAngle);
    sholat.set_isha_angle(_sholatConfig.ishaAngle);
  }

  // apply offset to timeOffsets array
  double timeOffsets[TimesCount] = {
      //_sholatConfig.offsetImsak,
      _sholatConfig.offsetFajr,
      _sholatConfig.offsetSunrise,
      _sholatConfig.offsetDhuhr,
      _sholatConfig.offsetAsr,
      _sholatConfig.offsetSunset,
      _sholatConfig.offsetMaghrib,
      _sholatConfig.offsetIsha};

  // Tuning SHOLAT TIME
  sholat.tune(timeOffsets);

  // timezone in seconds

  // location settings;
  double lat = configLocation.latitude;
  double lon = configLocation.longitude;
  float tZ = configLocation.timezone;

  // time_t now;
  struct tm *tm;
  int year;
  int month;
  int day;

  // time(&now);

  //CALCULATE YESTERDAY'S SHOLAT TIMES
  tm = localtime(&localTime);

  tm->tm_mday--; // alter tm struct to yesterday

  time_t t_yesterday = mktime(tm);
  tm = localtime(&t_yesterday);

  year = tm->tm_year + 1900;
  month = tm->tm_mon + 1;
  day = tm->tm_mday;

  sholat.get_prayer_times(year, month, day, lat, lon, tZ, sholat.timesYesterday);

  // print sholat times for yesterday
  // PRINT("\r\nYESTERDAY's Schedule - %s", asctime(tm));
  // snprintf_P(buf, sizeof(buf), PSTR("\r\nYESTERDAY's Schedule - %s\r\n"), asctime(tm));
  // Serial.println(asctime(tm));
  for (unsigned int i = 0; i < sizeof(sholat.timesYesterday) / sizeof(double); i++)
  {
    // Convert sholat time from float to hour and minutes
    // and store to an array (to retrieve if needed)
    const char *temp = sholat.float_time_to_time24(sholat.timesYesterday[i]);
    strlcpy(sholatTimeYesterdayArray[i], temp, sizeof(sholatTimeYesterdayArray[i]));

    // Calculate timestamp of of sholat time

    uint8_t hr, mnt;
    sholat.get_float_time_parts(sholat.timesYesterday[i], hr, mnt);

    // modify time struct
    tm->tm_hour = hr;
    tm->tm_min = mnt;
    tm->tm_sec = 0;

    //store to timestamp array
    timestampSholatTimesYesterday[i] = mktime(tm) - TimezoneSeconds();

    /*
    //Print all results
    // char tmpFloat[10];
    // PRINT("%d\t%-8s  %8.5f  %s  %lu\r\n",
    snprintf_P(buf, sizeof(buf), PSTR("%d\t%s  %lu\r\n"),
               i,
               //TimeName[i],
               //  sholatNameStr(i),
               // dtostrf(sholat.timesYesterday[i], 8, 5, tmpFloat),
               // sholat.timesYesterday[i],
               sholatTimeYesterdayArray[i],
               timestampSholatTimesYesterday[i]);
    Serial.print(buf);
    //Note:
    //snprintf specifier %f for float or double is not available in embedded device avr-gcc.
    //Therefore, float or double must be converted to string first. For this case I've used dtosrf to achive that.
    */
  }
  Serial.println();

  // CALCULATE TODAY'S SHOLAT TIMES
  tm = localtime(&localTime);

  year = tm->tm_year + 1900;
  month = tm->tm_mon + 1;
  day = tm->tm_mday;

  sholat.get_prayer_times(year, month, day, lat, lon, tZ, sholat.times);

  // print sholat times
  // PRINT("\r\nTODAY's Schedule - %s", asctime(tm));
  // snprintf_P(buf, sizeof(buf), PSTR("\r\nTODAY's Schedule - %s\r\n"), asctime(tm));
  // Serial.print(buf);
  for (unsigned int i = 0; i < sizeof(sholat.times) / sizeof(double); i++)
  {
    //Convert sholat time from float to hour and minutes
    //and store to an array (to retrieve if needed)
    const char *temp = sholat.float_time_to_time24(sholat.times[i]);
    strlcpy(sholatTimeArray[i], temp, sizeof(sholatTimeArray[i]));

    //Calculate timestamp of of sholat time
    uint8_t hr, mnt;
    sholat.get_float_time_parts(sholat.times[i], hr, mnt);

    // modify time struct
    tm->tm_hour = hr;
    tm->tm_min = mnt;
    tm->tm_sec = 0;

    //store to timestamp array
    timestampSholatTimesToday[i] = mktime(tm) - TimezoneSeconds();

    /*
    //Print all results
    //char tmpFloat[10];
    // PRINT("%d\t%-8s  %8.5f  %s  %lu\r\n",
    snprintf_P(buf, sizeof(buf), PSTR("%d\t%s  %lu\r\n"),
               i,
               //  sholatNameStr(i),
               //dtostrf(sholat.times[i], 8, 5, tmpFloat),
               // sholat.times[i],
               sholatTimeArray[i],
               timestampSholatTimesToday[i]);
    Serial.print(buf);
    //Note:
    //snprintf specifier %f for float or double is not available in embedded device avr-gcc.
    //Therefore, float or double must be converted to string first. For this case I've used dtosrf to achive that.
    */
  }
  Serial.println();

  // CALCULATE TOMORROW'S SHOLAT TIMES
  tm = localtime(&localTime);

  tm->tm_mday++; // alter tm struct to tomorrow
  time_t t_tomorrow = mktime(tm);
  tm = localtime(&t_tomorrow);

  year = tm->tm_year + 1900;
  month = tm->tm_mon + 1;
  day = tm->tm_mday;

  sholat.get_prayer_times(year, month, day, lat, lon, tZ, sholat.timesTomorrow);

  // print sholat times for Tomorrow
  // PRINT("\r\nTOMORROW's Schedule - %s", asctime(tm));
  // snprintf_P(buf, sizeof(buf), PSTR("\r\nTOMORROW's Schedule - %s\r\n"), asctime(tm));
  // Serial.print(buf);
  for (unsigned int i = 0; i < sizeof(sholat.timesTomorrow) / sizeof(double); i++)
  {
    //Convert sholat time from float to hour and minutes
    //and store to an array (to retrieve if needed)
    const char *temp = sholat.float_time_to_time24(sholat.times[i]);
    strlcpy(sholatTimeTomorrowArray[i], temp, sizeof(sholatTimeTomorrowArray[i]));

    //Calculate timestamp of of sholat time
    uint8_t hr, mnt;
    sholat.get_float_time_parts(sholat.timesTomorrow[i], hr, mnt);

    // modify time struct
    tm->tm_hour = hr;
    tm->tm_min = mnt;
    tm->tm_sec = 0;

    //store to timestamp array
    timestampSholatTimesTomorrow[i] = mktime(tm) - TimezoneSeconds();

    /*
    //Print all results
    //char tmpFloat[10];
    // PRINT("%d\t%-8s  %8.5f  %s  %lu\r\n",
    snprintf_P(buf, sizeof(buf), PSTR("%d\t%s  %lu\r\n"),
               i,
               // sholatNameStr(i),
               // dtostrf(sholat.timesTomorrow[i], 8, 5, tmpFloat),
               // sholat.timesTomorrow[i],
               sholatTimeTomorrowArray[i],
               timestampSholatTimesTomorrow[i]);
    Serial.print(buf);
    //Note:
    //snprintf specifier %f for float or double is not available in embedded device avr-gcc.
    //Therefore, float or double must be converted to string first. For this case I've used dtosrf to achive that.
    */
  }
  Serial.println();

  //config_save_sholat("Bekasi", latitude, longitude, timezoneSholat, Egypt, Shafii, AngleBased, 20, 1, 18);
}

void process_sholat_2nd_stage()
{
  // DEBUGLOG("%s\r\n", __PRETTY_FUNCTION__);

  time_t s_tm = 0;

  // time_t t_utc;

  // time(&now);

  //int hrNextTime, mntNextTime;

  //for (unsigned int i = 0; i < sizeof(sholat.times) / sizeof(double); i++) {
  for (int i = 0; i < TimesCount; i++)
  {
    if (i != Sunset)
    {

      //First we decide, what the ID for current and next sholat time are.
      int tempCurrentID, tempPreviousID, tempNextID;

      tempCurrentID = i;
      tempPreviousID = i - 1;
      tempNextID = i + 1;

      //check NextID
      if (tempNextID == Sunset)
      {
        tempNextID = Maghrib;
      }
      if (tempCurrentID == Isha)
      {
        tempNextID = Fajr;
      }

      //check PreviousID
      if (tempPreviousID == Sunset)
      {
        tempPreviousID = Asr;
      }
      if (tempCurrentID == Fajr)
      {
        tempPreviousID = Isha;
      }

      //then
      time_t timestamp_current_today;
      time_t timestamp_next_today;
      time_t timestamp_next_tomorrow;

      timestamp_current_today = timestampSholatTimesToday[tempCurrentID];

      timestamp_next_today = timestampSholatTimesToday[tempNextID];

      timestamp_next_tomorrow = timestampSholatTimesTomorrow[tempNextID];

      DEBUGLOG("timestamp_current_today=%lu, timestamp_next_today=%lu, timestamp_next_tomorrow=%lu\r\n", timestamp_current_today, timestamp_next_today, timestamp_next_tomorrow);

      if (timestamp_current_today < timestamp_next_today)
      {
        if (utcTime <= timestamp_current_today && utcTime < timestamp_next_today)
        {
          CURRENTTIMEID = tempPreviousID;
          NEXTTIMEID = tempCurrentID;
          s_tm = timestamp_current_today;

          break;
        }
        else if (utcTime > timestamp_current_today && utcTime <= timestamp_next_today)
        {
          CURRENTTIMEID = tempCurrentID;
          NEXTTIMEID = tempNextID;
          s_tm = timestamp_next_today;

          break;
        }
      }
      else if (timestamp_current_today > timestamp_next_today)
      {
        if (utcTime >= timestamp_current_today && utcTime < timestamp_next_tomorrow)
        {
          CURRENTTIMEID = tempCurrentID;
          NEXTTIMEID = tempNextID;
          s_tm = timestamp_next_tomorrow;

          break;
        }
      }
    }
  } //end of for loop

  DEBUGLOG("CURRENTTIMEID=%d, NEXTTIMEID=%d\r\n", CURRENTTIMEID, NEXTTIMEID);

  time_t timestamp_current_yesterday;
  time_t timestamp_current_today;
  time_t timestamp_next_today;
  time_t timestamp_next_tomorrow;

  timestamp_current_yesterday = timestampSholatTimesYesterday[CURRENTTIMEID];
  timestamp_current_today = timestampSholatTimesToday[CURRENTTIMEID];
  timestamp_next_today = timestampSholatTimesToday[NEXTTIMEID];
  timestamp_next_tomorrow = timestampSholatTimesTomorrow[NEXTTIMEID];

  if (NEXTTIMEID > CURRENTTIMEID)
  {
    currentSholatTime = timestamp_current_today;
    nextSholatTime = timestamp_next_today;
    //PRINT("%s %lu %lu\n", "Case 2a", currentSholatTime, nextSholatTime);
    DEBUGLOG("NEXTTIMEID > CURRENTTIMEID, currentSholatTime=%lu, nextSholatTime=%lu\r\n", currentSholatTime, nextSholatTime);
  }
  else if (NEXTTIMEID < CURRENTTIMEID)
  {
    // time_t t_utc = time(nullptr);
    struct tm *tm_utc = gmtime(&utcTime);

    if (tm_utc->tm_hour >= 12) // is PM ?
    {
      currentSholatTime = timestamp_current_today;
      nextSholatTime = timestamp_next_tomorrow;
      DEBUGLOG("NEXTTIMEID < CURRENTTIMEID, currentSholatTime=%lu, nextSholatTime=%lu Hour: %d, is PM\r\n", currentSholatTime, nextSholatTime, tm_utc->tm_hour);
    }
    if (tm_utc->tm_hour < 12) // is AM ?
    {
      currentSholatTime = timestamp_current_yesterday;
      nextSholatTime = timestamp_next_today;
      //PRINT("%s %lu %lu\n", "Case 2c", currentSholatTime, nextSholatTime);
      DEBUGLOG("NEXTTIMEID < CURRENTTIMEID, currentSholatTime=%lu, nextSholatTime=%lu Hour: %d, is PM\r\n", currentSholatTime, nextSholatTime, tm_utc->tm_hour);
    }
  }

  time_t timeDiff = s_tm - utcTime;
  DEBUGLOG("s_tm: %lu, now: %lu, timeDiff: %lu\r\n", s_tm, now, timeDiff);

  // uint16_t days;
  // uint8_t hr;
  // uint8_t mnt;
  // uint8_t sec;

  // // METHOD 2
  // // days = elapsedDays(timeDiff);
  // HOUR = numberOfHours(timeDiff);
  // MINUTE = numberOfMinutes(timeDiff);
  // SECOND = numberOfSeconds(timeDiff);

  // METHOD 3 -> https://stackoverflow.com/questions/2419562/convert-seconds-to-days-minutes-and-seconds/17590511#17590511
  tm *diff = gmtime(&timeDiff); // convert to broken down time
  // DAYS = diff->tm_yday;
  HOUR = diff->tm_hour;
  MINUTE = diff->tm_min;
  SECOND = diff->tm_sec;

  // METHOD 4 -> https://stackoverflow.com/questions/2419562/convert-seconds-to-days-minutes-and-seconds/2419597#2419597
  // HOUR = floor(timeDiff / 3600.0);
  // MINUTE = floor(fmod(timeDiff, 3600.0) / 60.0);
  // SECOND = fmod(timeDiff, 60.0);

  dtostrf(SECOND, 0, 0, bufSECOND);
  dtostrf(MINUTE, 0, 0, bufMINUTE);
  dtostrf(HOUR, 0, 0, bufHOUR);

  ceilMINUTE = MINUTE;
  ceilHOUR = HOUR;

  if (SECOND > 0)
  {
    ceilMINUTE++;
    if (ceilMINUTE == 60)
    {
      ceilHOUR++;
      ceilMINUTE = 0;
    }
  }

  if (SECOND == 0)
  {
    if (ceilHOUR != 0 || ceilMINUTE != 0)
    {
      if (ceilHOUR != 0)
      {
        PRINT("%d jam ", ceilHOUR);
      }
      if (ceilMINUTE != 0)
      {
        PRINT("%d menit ", ceilMINUTE);
      }
      PRINT("menuju %s\r\n", sholatNameStr(NEXTTIMEID));
    }
    else if (HOUR == 0 && MINUTE == 0)
    {
      PRINT("Waktu %s telah masuk!\r\n", sholatNameStr(NEXTTIMEID));
      PRINT("Dirikanlah sholat tepat waktu.\r\n");
    }
  }
}

void ProcessSholatEverySecond()
{
  // time_t now = time(nullptr);
  if (utcTime >= nextSholatTime)
  {
    process_sholat();
  }

  process_sholat_2nd_stage();

  static int NEXTTIMEID_old = 100;

  if (NEXTTIMEID != NEXTTIMEID_old)
  {
    NEXTTIMEID_old = NEXTTIMEID;
  }
}

// float TimezoneFloat()
// {
//   time_t rawtime;
//   struct tm *timeinfo;
//   char buffer[6];

//   time(&rawtime);
//   timeinfo = localtime(&rawtime);

//   strftime(buffer, 6, "%z", timeinfo);

//   char bufTzHour[4];
//   strncpy(bufTzHour, buffer, 3);
//   int8_t hour = atoi(bufTzHour);

//   char bufTzMin[4];
//   bufTzMin[0] = buffer[0]; // sign
//   bufTzMin[1] = buffer[3];
//   bufTzMin[2] = buffer[4];
//   float min = atoi(bufTzMin) / 60.0;

//   float TZ_FLOAT = hour + min;
//   return TZ_FLOAT;
// }

// int32_t TimezoneMinutes()
// {
//   return TimezoneFloat() * 60;
// }

// int32_t TimezoneSeconds()
// {
//   return TimezoneMinutes() * 60;
// }