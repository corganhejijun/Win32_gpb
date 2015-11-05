// Copyright (C) 2002 David R. Martin <dmartin@eecs.berkeley.edu>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA, or see http://www.gnu.org/copyleft/gpl.html.

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "timer.hh"
#include "types.hh"

namespace Util {
#ifdef LINUX_COMPILE
#else
    __int64 Timer::FileTimeToInt64(const FILETIME& time)
    {
        ULARGE_INTEGER tt;
        tt.LowPart = time.dwLowDateTime;
        tt.HighPart = time.dwHighDateTime;
        return(tt.QuadPart);
    }
#endif

    int Timer::gettimeofday(struct timeval *tv, struct timezone *tz/*Should always be NULL*/)
    {
        __int64 DELTA_EPOCH_IN_MICROSECS = 11644473600000000;
        FILETIME ft;
        __int64 tmpres = 0;
        TIME_ZONE_INFORMATION tz_winapi;
        int rez = 0;

        ZeroMemory(&ft, sizeof(ft));
        ZeroMemory(&tz_winapi, sizeof(tz_winapi));

        GetSystemTimeAsFileTime(&ft);

        tmpres = ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;

        /*converting file time to unix epoch*/
        tmpres /= 10;  /*convert into microseconds*/
        tmpres -= DELTA_EPOCH_IN_MICROSECS;
        tv->tv_sec = (__int32)(tmpres*0.000001);
        tv->tv_usec = (tmpres % 1000000);


        //_tzset(),don't work properly, so we use GetTimeZoneInformation
        rez = GetTimeZoneInformation(&tz_winapi);
        //tz->tz_dsttime = (rez == 2) ? true : false;
        //tz->tz_minuteswest = tz_winapi.Bias + ((rez == 2) ? tz_winapi.DaylightBias : 0);

        return 0;
    }


  void
  Timer::_compute ()
  {
      // Compute elapsed time.
      long sec = _elapsed_stop.tv_sec - _elapsed_start.tv_sec;
      long usec = _elapsed_stop.tv_usec - _elapsed_start.tv_usec;
      if (usec < 0) {
        sec -= 1;
        usec += 1000000;
      }
      _elapsed += (double) sec + usec / 1e6;

      // Computer CPU user and system times.  
#ifdef LINUX_COMPILE
      _user += (double) (_cpu_stop.tms_utime - _cpu_start.tms_utime) / sysconf(_SC_CLK_TCK);
      _system += (double) (_cpu_stop.tms_stime - _cpu_start.tms_stime) / sysconf(_SC_CLK_TCK);
#else
      LARGE_INTEGER nFreq;
      QueryPerformanceFrequency(&nFreq);
      _user += (double)(_cpu_stop.tms_utime - _cpu_start.tms_utime) / nFreq.QuadPart;
      _system += (double) (_cpu_stop.tms_stime - _cpu_start.tms_stime) / nFreq.QuadPart;
#endif
  }

  // Convert time in seconds into a nice human-friendly format: h:mm:ss.ss
  // Return a pointer to a static buffer.
  const char* 
  Timer::formatTime (double sec, int precision)
  {
      static char buf[128];

      // Limit range of precision for safety and sanity.
      precision = (precision < 0) ? 0 : precision;
      precision = (precision > 9) ? 9 : precision;
      uint64 base = 1;
      for (int digit = 0; digit < precision; digit++) { base *= 10;}

      bool neg = (sec < 0);
      uint64 ticks = (uint64) rint (fabs (sec) * base);
      uint64 rsec = ticks / base;		// Rounded seconds.
      uint64 frac = ticks % base;

      uint64 h = rsec / 3600;
      uint64 m = (rsec / 60) % 60;
      uint64 s = rsec % 60;

      sprintf_s (buf, "%s%llu:%02llu:%02llu", 
         neg ? "-" : "", h, m, s);

      if (precision > 0) {
        static char fmt[10];
        sprintf_s (fmt, ".%%0%dlld", precision);
        int buflen = strlen(buf);
        sprintf_s ((buf + buflen), 128 - buflen, "%s%llu", fmt, frac);
      }

      return buf;
  }

} // namespace Util

