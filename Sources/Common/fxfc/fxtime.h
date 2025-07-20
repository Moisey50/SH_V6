#ifndef FXTIME_INCLUDE
#define FXTIME_INCLUDE
// fxtime.h : header file
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class FXFC_EXPORT FXTime
{
private:
  __time64_t m_time;
public:
  FXTime() throw();
  FXTime( __time64_t time ) throw();
	FXTime( int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, int nDST = -1 );
///
  FXTime& operator=( __time64_t time ) throw();

  bool operator==( FXTime time ) const throw();
  bool operator==( __time64_t time ) const throw();

  FXTime& operator+=( tm& Add ) ;

  FXString Format(LPCTSTR pszFormat) const;
  static FXTime GetCurrentTime( );
};

#endif // #ifndef FXTIME_INCLUDE
