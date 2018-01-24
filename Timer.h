#ifndef ___Included_Timer

#define ___Included_Timer

#ifdef WIN32
  #include <time.h>
  #include <windows.h>
#else
  #include <sys/time.h>
  #include <unistd.h>
#endif

/** class Timer.
  
    Provides a timer for measuring time delays, without the hassle of
    doing the donkey work.
*/
class Timer
  {
    protected:
      /// start of timing interval.
#ifdef WIN32
      unsigned __int64    lasttime;
#else
      struct timeval      lasttime;
#endif

    public:
      static const char * classIdentifier;

      Timer ();
      ~Timer ();
 
      /// mark the start of a timing interval.
      void mark ();
      /// return the interval length.
      double interval ();
  };

#endif

