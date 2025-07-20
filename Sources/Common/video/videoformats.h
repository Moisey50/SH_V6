#ifndef VIDEOFORMATS_INC
#define VIDEOFORMATS_INC

#define BI_YUV411   0x31313459
#define BI_YUV422   0x32323459
#define BI_Y411     0x31313459
#define BI_YUV9     0x39555659
#define BI_YUV12    0x32315659
#define BI_I420     0x30323449
#define BI_UYVY			0x59565955
#define BI_YUY2			0x32595559
#define BI_Y8   	  0x20203859
#define BI_Y800     0x30303859
#define BI_Y16      0x20363159
#define BI_Y12      0x20323159
#define BI_YP12     0x32315059
#define BI_MJPG 	  0x47504a4d
#define BI_H264     0x34363248
#define BI_RGB48    0x38344752
#define BI_NV12     0x3231564E
#define I50_FORMAT  0x30355649
#define I32_FORMAT  0x32335649
//#define I420_FORMAT 0x30323449
#define NV12_FORMAT 0x3231564E

static int FormatsForEnums[] =
{
  0
  , BI_Y8
  , BI_Y800
  , BI_RGB
  , BI_YUV411
  , BI_YUV422
  , BI_Y411
  , BI_YUV9
  , BI_YUV12
  , BI_I420
  , BI_UYVY
  , BI_YUY2
  , BI_Y16
  , BI_Y12
  , BI_YP12
  , BI_MJPG
  , BI_H264
  , BI_RGB48
  , BI_NV12
  , I50_FORMAT
  , I32_FORMAT
  , NV12_FORMAT
} ;

static const UINT NFormats = ( sizeof( FormatsForEnums ) / sizeof( FormatsForEnums[ 0 ] ) ) ;

inline const TCHAR * GetVideoFormatName( DWORD dwFormat )
{
  switch ( dwFormat )
  {
    case BI_YUV411: return _T( "YUV411" ) ;
    case BI_YUV422: return _T( "YUV422" ) ;
    case BI_YUV9: return _T( "YUV9" ) ;
    case BI_YUV12: return _T( "YUV12" ) ;
    case BI_UYVY: return _T( "UYUV" ) ;
    case BI_YUY2: return _T( "YUY2" ) ;
    case BI_Y8: return _T( "Y8" ) ;
    case BI_Y800: return _T( "Y800" ) ;
    case BI_Y16: return _T( "Y16" ) ;
    case BI_Y12: return _T( "Y12" ) ;
    case BI_YP12: return _T( "YP12" ) ;
    case BI_MJPG: return _T( "MJPG" ) ;
    case BI_H264: return _T( "H264" ) ;
    case BI_RGB48: return _T( "RGB48" ) ;
    case BI_RGB: return _T( "RGB" ) ;
    case I50_FORMAT: return _T( "I50" ) ;
    case I32_FORMAT: return _T( "I32" ) ;
//    case I420_FORMAT: return _T( "I420" ) ;
    case NV12_FORMAT: return _T( "NV12" ) ;
  }
  return _T( "Unknown Format" ) ;
}

inline int GetVideoFormatEnum( DWORD dwFormat )
{
  switch ( dwFormat )
  {
    case BI_Y8: return 1 ;
    case BI_Y800: return 2 ;
    case BI_RGB: return 3 ;
    case BI_YUV411: return 4 ;
    case BI_YUV422: return 5 ;
    case BI_YUV9: return 6 ;
    case BI_YUV12: return 7 ;
    case BI_UYVY: return 8 ;
    case BI_YUY2: return 9 ;
    case BI_Y16: return 10 ;
    case BI_Y12: return 11 ;
    case BI_YP12: return 12 ;
    case BI_MJPG: return 13 ;
    case BI_H264: return 14 ;
    case BI_RGB48: return 15 ;
    case I50_FORMAT: return 16 ;
    case I32_FORMAT: return 17 ;
//    case I420_FORMAT: return _T( "I420" ) ;
    case NV12_FORMAT: return 18 ;
  }
  return 0 ;
}

inline int GetVideoFormatForEnum( DWORD Enumerator )
{
  if ( (0 < Enumerator) 
    && (Enumerator <= NFormats) )
  {
    return FormatsForEnums[ Enumerator ] ;
  }
  return 0 ;
}

#endif
