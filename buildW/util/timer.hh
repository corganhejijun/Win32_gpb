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

#ifndef TIMER_HH
#define TIMER_HH

#ifdef LINUX_COMPILE
#include <sys/times.h>
#include <sys/time.h>
#else
#include <Winsock2.h>
#include <Winbase.h>
#endif
#include <time.h>
#include "unistd.h"
#include <assert.h>

namespace Util {

  class Timer
  {
    public:

      inline Timer ();
      inline ~Timer ();
      
      inline void start ();
      inline void stop ();
      inline void reset ();

      // All times are in seconds.
      inline double cpu ();	
      inline double user ();
      inline double system ();
      inline double elapsed ();

      // Convert time in seconds into a nice human-friendly format: h:mm:ss.ss
      // Precision is the number of digits after the decimal.
      // Return a pointer to a static buffer.
      static const char* formatTime (double sec, int precision = 2);
      static int gettimeofday(struct timeval *tv, struct timezone *tz);
      
    private:

      void _compute ();
    
      enum State { stopped, running };

      State _state;
    
      struct timeval _elapsed_start;
      struct timeval _elapsed_stop;
      double _elapsed;

#ifdef LINUX_COMPILE
#else
      struct tms {
          clock_t tms_utime; /* user time */
          clock_t tms_stime; /* system time */
          clock_t tms_cutime; /* user time of children */
          clock_t tms_cstime; /* system time of children */
      };
#endif
      struct tms _cpu_start;
      struct tms _cpu_stop;
      double _user;
      double _system;
#ifdef LINUX_COMPILE
#else
      __int64 FileTimeToInt64(const FILETIME& time);
#endif
  };

  Timer::Timer ()
  {
      reset ();
  }

  Timer::~Timer ()
  {
  }

  void
  Timer::reset ()
  {
      _state = stopped;
      _elapsed = _user = _system = 0;
  }

  void
  Timer::start ()
  {
      assert (_state == stopped);
      _state = running;
      gettimeofday (&_elapsed_start, NULL);
#ifdef LINUX_COMPILE
      times (&_cpu_start);
#else
      DWORD pid = GetCurrentProcessId();   // 得到当前进程id
      HANDLE hd = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);  // 通过id得到进程的句柄
      FILETIME create;
      FILETIME exit;
      FILETIME ker;  // 内核占用时间
      FILETIME user; // 用户占用时间
      FILETIME now;
      GetProcessTimes(hd, &create, &exit, &ker, &user);
      _cpu_start.tms_stime = FileTimeToInt64(ker);
#endif
  }

  void
  Timer::stop ()
  {
      assert (_state == running);
      gettimeofday (&_elapsed_stop, NULL);
#ifdef LINUX_COMPILE
      times (&_cpu_stop);
#else
#endif
      _compute ();
     _state = stopped;
  }

  double
  Timer::cpu ()
  {
      assert (_state == stopped);
      return _user + _system;
  }

  double
  Timer::user ()
  {
      assert (_state == stopped);
      return _user;
  }

  double
  Timer::system ()
  {
      assert (_state == stopped);
      return _system;
  }

  double
  Timer::elapsed ()
  {
      assert (_state == stopped);
      return _elapsed;
  }

} // namespace Util

#endif // __Timer_hh__



