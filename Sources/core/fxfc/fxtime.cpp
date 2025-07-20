#include "stdafx.h"
#include <fxfc\fxfc.h>
#include <time.h>

const int FXmaxTimeBufferSize = 128;

FXTime::FXTime() throw() :
	m_time(0)
{
}

FXTime::FXTime( __time64_t time )  throw():
	m_time( time )
{
}

static int days_in_month[] = { 0 , 31 , 28 , 31 , 30 , 31 , 30 , 31 , 31 , 30 , 31 , 30 , 31 };

int is_leap( int nYear )
{
  return ( ( nYear % 4 == 0 && nYear % 100 != 0 ) || nYear % 400 == 0 );
}


bool IsCorrectMonthAndDay( int nYear , int nMonth , int nDay )
{
  if ( nDay < 1 )
    return false ;
  if ( nMonth == 2 )
  {
    if ( is_leap( nYear ) )
      return ( nDay <= 29 ) ;
    else
      return ( nDay <= 28 ) ;
  }
  else if ( ( 0 < nMonth ) && ( nMonth <= 12 ) )
    return ( nDay <= days_in_month[ nMonth ] ) ;

  return false ;
}

void next_day( tm& Ttm )
{
  Ttm.tm_mday += 1;
  if ( Ttm.tm_mday > days_in_month[ Ttm.tm_mon ] )
  {
    Ttm.tm_mday = 1;
    Ttm.tm_mon += 1;
    if ( Ttm.tm_mon > 12 )
    {
      Ttm.tm_mon = 1;
      Ttm.tm_year += 1;
      if ( is_leap( Ttm.tm_year ) )
      {
        days_in_month[ 2 ] = 29;
      }
      else
      {
        days_in_month[ 2 ] = 28;
      }
    }
  }
}

FXTime::FXTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,
               int nDST): m_time(-1L)
{
	bool goodtime = (( nYear >= 1900 ) && IsCorrectMonthAndDay( nYear , nMonth , nDay ) &&
	                 ( nHour >= 0 && nHour <= 23 )&&
	                 ( nMin >= 0 && nMin <= 59 ) &&
                     ( nSec >= 0 && nSec <= 59 ));
  if (goodtime)
  {
      struct ::tm atm;

	  atm.tm_sec = nSec;
	  atm.tm_min = nMin;
	  atm.tm_hour = nHour;
	  atm.tm_mday = nDay;
	  atm.tm_mon = nMonth - 1;        // tm_mon is 0 based
	  atm.tm_year = nYear - 1900;     // tm_year is 1900 based
	  atm.tm_isdst = nDST;

	  m_time = _mktime64(&atm);
  }
	if ( m_time == -1 )
	{
		throw (E_INVALIDARG);
	}
}

FXTime& FXTime::operator=( __time64_t time ) throw()
{
	m_time = time;
	return( *this );
}

bool FXTime::operator==( FXTime time ) const throw()
{
	return( m_time == time.m_time );
}

bool FXTime::operator==( __time64_t time ) const throw()
{
	return( m_time == m_time );
}

FXTime& FXTime::operator += ( tm& Add )
{
  struct tm Ttm ;
  gmtime_s( &Ttm , &m_time  ) ;

  Ttm.tm_sec += Add.tm_sec ;
  Ttm.tm_min += Ttm.tm_sec / 60 ;
  Ttm.tm_sec %= 60 ;
  if ( Ttm.tm_sec < 0 )
    Ttm.tm_sec = 60 - Ttm.tm_sec ;

  Ttm.tm_min += Add.tm_min ;
  Ttm.tm_hour += Ttm.tm_min / 60 ;
  Ttm.tm_min %= 60 ;
  if ( Ttm.tm_min < 0 )
    Ttm.tm_min = 60 - Ttm.tm_min ;

  Ttm.tm_hour += Add.tm_hour ;
  Ttm.tm_mday += Ttm.tm_hour / 24 ;
  Ttm.tm_hour %= 24 ;
  if ( Ttm.tm_hour < 0 )
    Ttm.tm_hour = 24 - Ttm.tm_hour ;

  Ttm.tm_mday += Add.tm_mday ;
  if ( Ttm.tm_mday > days_in_month[ Ttm.tm_mon ] )
  {
  }
  else if ( Ttm.tm_mday < 0 )
  {
  }
  else
  {

  }
  Ttm.tm_mon += Add.tm_mon ;
  Ttm.tm_year += Add.tm_year ;

  m_time = _mktime64( &Ttm );

  return *this ;
}

FXTime FXTime::GetCurrentTime( )
{
    __time64_t time;
    _time64(&time); 
    FXTime retV(time);
    return retV;
}

FXString FXTime::Format(LPCTSTR pszFormat) const
{
	if(pszFormat == NULL)
		return pszFormat;

	TCHAR szBuffer[FXmaxTimeBufferSize];

	struct tm ptmTemp;
	errno_t err = _localtime64_s(&ptmTemp, &m_time);
	if (err != 0 || !_tcsftime(szBuffer, FXmaxTimeBufferSize, pszFormat, &ptmTemp))
	{
		szBuffer[0] = '\0';
	}

	return szBuffer;
}