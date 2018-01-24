#include <iostream>
#include "Timer.h"

using namespace std;

const char * Timer::classIdentifier = "$Id: Timer.cc 746 2011-04-08 04:15:44Z shaun $";

/// flag to allowing debugging statements within the file
//#define DEBUGTIMER

Timer::Timer ()

{
#ifdef DEBUGTIMER
  cout << "Creating Timer\n";
#endif

  mark ();
}

Timer::~Timer ()

{
#ifdef DEBUGTIMER
  cout << "Destroying Timer\n";
#endif

#ifdef DEBUGTIMER
  cout << "Destroyed Timer\n";
#endif
}

void Timer::mark ()

{
#ifdef WIN32
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  lasttime = ft.dwHighDateTime;
  lasttime <<= 32;
  lasttime |= ft.dwLowDateTime;
  lasttime /= 10;  /*convert into microseconds*/
#else
  gettimeofday (&lasttime, NULL);
#endif
}

double Timer::interval ()

{
#ifdef WIN32
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  unsigned __int64 currtime = 0;
  currtime |= ft.dwHighDateTime;
  currtime <<= 32;
  currtime |= ft.dwLowDateTime;
  currtime /= 10;  /*convert into microseconds*/
  return ((double) (currtime - lasttime)) / 1000000.0;
#else
  struct timeval currtime;
  gettimeofday (&currtime, NULL);

  double ct = currtime.tv_sec * 1000000.0 + currtime.tv_usec;
  double lt = lasttime.tv_sec * 1000000.0 + lasttime.tv_usec;
  return (ct - lt) / 1000000.0;
#endif
}

