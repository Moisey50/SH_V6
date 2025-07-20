#include <stdafx.h>

#include <time.h>
#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "get_time.h"

static double dPerfFreq;	// processor performance frequency
static double dStartTime;	// reference tick (first tick after start)
static bool bInitialized = false ;
double get_k_frequency(  )
{
  return dPerfFreq;
}

double
get_current_time()
{
  LARGE_INTEGER Ticks ;
  if ( ! bInitialized )   // init is necessary
  {
    bInitialized = 1;
    LARGE_INTEGER Freq ;
    if ( QueryPerformanceFrequency( &Freq ) )
    {
      dPerfFreq = (double)Freq.QuadPart/1000. ; // for msec calculation
      if ( QueryPerformanceCounter( &Ticks ) )
      {
        dStartTime = (double)Ticks.QuadPart/dPerfFreq ;
        return 0. ; // first call
      }
    }
    dPerfFreq = 0. ;
    dStartTime = (double) GetTickCount() ;
    return 0. ; // failure in OS timer system, we will use GetTickCount
  }
  if ( dPerfFreq > 0 )
  {
    if ( QueryPerformanceCounter( &Ticks ) )
      return ((double)Ticks.QuadPart/dPerfFreq) - dStartTime ;
  }
  return (double)GetTickCount() - dStartTime ;
}
