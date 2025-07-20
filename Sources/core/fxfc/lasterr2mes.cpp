#include "stdafx.h"
#include <fxfc\fxfc.h>


FXString FxLastErr2Mes( LPCTSTR Prefix , LPCTSTR Suffix )
{
  LPVOID lpMsgBuf;
  FXString mes;

  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS ,
    NULL ,
    GetLastError() ,
    MAKELANGID( LANG_NEUTRAL , SUBLANG_DEFAULT ) , // Default language
    ( LPTSTR ) &lpMsgBuf ,
    0 ,
    NULL
  );
  mes.Format( _T( "%s%s (%d) %s" ) , Prefix , ( CHAR* ) lpMsgBuf , GetLastError() , Suffix );
  LocalFree( lpMsgBuf );
  return mes;
}

FXString FxLastErr2Mes( DWORD LastErr , LPCTSTR Prefix , LPCTSTR Suffix )
{
  LPVOID lpMsgBuf;
  FXString mes;

  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS ,
    NULL ,
    LastErr ,
    MAKELANGID( LANG_NEUTRAL , SUBLANG_DEFAULT ) , // Default language
    ( LPTSTR ) &lpMsgBuf ,
    0 ,
    NULL
  );
  ( ( CHAR* ) lpMsgBuf )[ _tcsclen( ( CHAR* ) lpMsgBuf ) - 1 ] = 0;
  mes.Format( _T( "%s%s%s" ) , Prefix , ( CHAR* ) lpMsgBuf , Suffix );
  LocalFree( lpMsgBuf );
  return mes;
}

FXString ProcessHRESULT( HRESULT hr , LPCTSTR Prefix , LPCTSTR Suffix )
{
  FXString Result ;
  DWORD ErrCode = Win32FromHResult( hr ) ;
  if ( ErrCode )
  {
    if ( ErrCode == ERROR_CAN_NOT_COMPLETE )
      Result = "Can't complete error decoding" ;
    else
      Result = FxLastErr2Mes( ErrCode , Prefix , Suffix ) ;
  }
  return Result ;
}

#include <VFW.H>
/*#define AVIERR_INTERNAL         MAKE_AVIERR(104)
#define AVIERR_BADFLAGS         MAKE_AVIERR(105)
#define AVIERR_BADPARAM         MAKE_AVIERR(106)
#define AVIERR_BADSIZE          MAKE_AVIERR(107)
#define AVIERR_BADHANDLE        MAKE_AVIERR(108)
#define AVIERR_FILEREAD         MAKE_AVIERR(109)
#define AVIERR_FILEWRITE        MAKE_AVIERR(110)
#define AVIERR_FILEOPEN         MAKE_AVIERR(111)
#define AVIERR_COMPRESSOR       MAKE_AVIERR(112)
#define AVIERR_NOCOMPRESSOR     MAKE_AVIERR(113)
#define AVIERR_READONLY		    MAKE_AVIERR(114)
#define AVIERR_NODATA		    MAKE_AVIERR(115)
#define AVIERR_BUFFERTOOSMALL	MAKE_AVIERR(116)
#define AVIERR_CANTCOMPRESS	    MAKE_AVIERR(117)
#define AVIERR_USERABORT        MAKE_AVIERR(198)
#define AVIERR_ERROR            MAKE_AVIERR(199) */
/// MY_ONEERRORS
#define AVIERR_DONTOPEN         MAKE_AVIERR(300)

FXString FxAviFileError2Mes( UINT Error )
{
  FXString res;
  switch ( Error )
  {
    case AVIERR_OK:             res = _T( "No error" ); break;
    case AVIERR_UNSUPPORTED:    res = _T( "AVIERR_UNSUPPORTED" ); break;
    case AVIERR_BADFORMAT:      res = _T( "AVIERR_BADFORMAT" ); break;
    case AVIERR_MEMORY:         res = _T( "AVIERR_MEMORY" ); break;
    case AVIERR_INTERNAL:       res = _T( "AVIERR_INTERNAL" ); break;
    case AVIERR_BADFLAGS:       res = _T( "AVIERR_BADFLAGS" ); break;
    case AVIERR_BADPARAM:       res = _T( "AVIERR_BADPARAM" ); break;
    case AVIERR_BADSIZE:        res = _T( "AVIERR_BADSIZE" ); break;
    case AVIERR_BADHANDLE:      res = _T( "AVIERR_BADHANDLE" ); break;
    case AVIERR_FILEREAD:       res = _T( "AVIERR_FILEREAD" ); break;
    case AVIERR_FILEWRITE:      res = _T( "AVIERR_FILEWRITE" ); break;
    case AVIERR_FILEOPEN:       res = _T( "AVIERR_FILEOPEN" ); break;
    case AVIERR_COMPRESSOR:     res = _T( "AVIERR_COMPRESSOR" ); break;
    case AVIERR_NOCOMPRESSOR:   res = _T( "AVIERR_NOCOMPRESSOR" ); break;
    case AVIERR_READONLY:       res = _T( "AVIERR_READONLY" ); break;
    case AVIERR_NODATA:         res = _T( "AVIERR_NODATA" ); break;
    case AVIERR_BUFFERTOOSMALL: res = _T( "AVIERR_BUFFERTOOSMALL" ); break;
    case AVIERR_CANTCOMPRESS:   res = _T( "AVIERR_CANTCOMPRESS" ); break;
    case AVIERR_USERABORT:      res = _T( "AVIERR_USERABORT" ); break;
    case AVIERR_ERROR:          res = _T( "AVIERR_ERROR" ); break;
    case AVIERR_DONTOPEN:       res = _T( "Avi file does not open" ); break;
    case REGDB_E_CLASSNOTREG: res = _T( "REGDB_E_CLASSNOTREG" ); break;
    default: res.Format( _T( "Unknown AVIFILE error: 0x%x" ) , Error );
  }
  return res;
}