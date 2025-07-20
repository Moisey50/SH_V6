// "get_time.cpp"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include "stdafx.h"
//#include <windows.h>

#include <time.h>
#include <stdio.h>
#include "get_time.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#ifdef __cplusplus 
extern  "C" {
#endif

  __int64 get_int_time()
  {
    __int64 iCount ;
    if ( !QueryPerformanceCounter( (LARGE_INTEGER*)&iCount ) )
      return 0 ;
    else
      return iCount ;
  }

  int             initialized = 0;
  int             accurate = 1 ;
  double          k_frequency;  // ticks per second
  __int64         start;
  __int64         start_measure;

  void start_time_measurement(  )
  {
    start_measure = get_int_time();
  }

  __int64 end_time_measurement(  )
  {
    return ( get_int_time(  ) - start_measure );
  }

  double get_interval(  )
  {
    return ( k_frequency * ( get_int_time(  ) - start_measure ) );
  }
  double get_k_frequency(  )
  {
    return k_frequency;
  }

  double get_cur_time()
  {
    if ( ! initialized )   // inti is necessary
    {
      initialized = 1;

      accurate = 1;

      __int64 ifrequ ;
      if ( !QueryPerformanceFrequency( (LARGE_INTEGER*) &ifrequ )
        || (ifrequ == 0) ) 
        ASSERT( 0 ) ;
      else
        k_frequency = 1000. / (double)(ifrequ);
      start = get_int_time();
    }
    if ( accurate )
    {
      return ( k_frequency * (double)( get_int_time() - start ) );
    }
    else
      return (double)clock();
  }

#ifdef __cplusplus 
}
#endif
