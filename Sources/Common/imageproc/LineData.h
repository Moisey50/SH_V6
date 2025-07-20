#ifndef __LINE_DATA_H_
#define __LINE_DATA_H_

#include <classes\DRect.h>

#ifndef EMBEDDED_OBJ_NAME_LEN
#define EMBEDDED_OBJ_NAME_LEN  30
#define MAX_OBJ_NAME_LEN       (EMBEDDED_OBJ_NAME_LEN - 1)
#endif

class CLineResult
{
public:
  int     m_iIndex ;
  char    m_szName[ EMBEDDED_OBJ_NAME_LEN ] ;
  CDRect  m_DRect ;
  CDPoint m_Center ;
  double  m_dExtremalAmpl  ;
  double  m_dAverCent5x5 ;
  double  m_dAngle ;
  int     m_ImageMaxBrightness ;
  int     m_ImageMinBrightness ;


  CLineResult() { memset( this , 0 , sizeof(*this) ) ; } ;
  CLineResult( CLineResult& Orig ) 
  {
    memcpy( this , &Orig , sizeof( *this ) ) ;
  }
  ~CLineResult() {} ;
  CLineResult& operator =(const CLineResult& Orig)
  {
    if ( this != &Orig )
    {
      memcpy( this , &Orig , sizeof( *this ) ) ;
    }
    return *this ;
  }

  FXString GetCaption() { return FXString(
    _T( "  #  ,   Xc   ,  Yc   ,Iprof_max,Ic_aver , Xleft  , Xright , Ytop  , Ybottom,  Imin ,Imax \n" ) ); } ;

  void ToString( FXString& Result , LPCTSTR pName = NULL )
  {
    Result.Format(
      "%5d %9.2f %7.2f %9.2f %8.2f %8.2f %7.2f %8.2f %8.2f %5d %5d Name=%s; \n" ,
      m_iIndex ,
      m_Center.x , m_Center.y , m_dExtremalAmpl , m_dAverCent5x5 ,
      m_DRect.left , m_DRect.right , m_DRect.top , m_DRect.bottom ,
      m_ImageMinBrightness , m_ImageMaxBrightness ,
      pName ? pName : "\0"
    ) ;
  }
  void ToStringCSV( FXString& Result , LPCTSTR pName = NULL )
  {
    Result.Format(
      "%5d,%8.2f,%8.2f,%6.0f,%6.0f,%7.2f,%7.2f,%7.2f,%7.2f,%5d,%5d,Name=%s; \n" ,
      m_iIndex ,
      m_Center.x , m_Center.y , m_dExtremalAmpl , m_dAverCent5x5 ,
      m_DRect.left , m_DRect.right , m_DRect.top , m_DRect.bottom ,
      m_ImageMinBrightness , m_ImageMaxBrightness ,
      pName ? pName : "\0"
    ) ;
  }
  bool FromString( FXString& Src , FXString * pName = NULL )
  {
    if (11 == sscanf( ( LPCTSTR ) Src , _T( "%d %lf %lf %lf %lf %lf %lf %lf %lf %d %d" ) ,
      &m_iIndex ,
      &m_Center.x , &m_Center.y , &m_dExtremalAmpl , &m_dAverCent5x5 ,
      &m_DRect.left , &m_DRect.right , &m_DRect.top , &m_DRect.bottom ,
      &m_ImageMinBrightness , &m_ImageMaxBrightness ))
    {
      if (pName)
      {
        int iNamePos = ( int ) Src.Find( "Name=" ) ;
        if (iNamePos > 0)
        {
          pName->Empty() ;
          iNamePos += 5 ;
          LPCTSTR pChar = ( ( LPCTSTR ) ( *pName ) ) + iNamePos ;
          while (*pChar  &&  *pChar != _T( ';' ))
            *pName += *pChar ;
        }
      }
      return true ;
    }
    else
      return false ;
  }
  bool FromStringCSV( FXString& Src , FXString * pName = NULL )
  {
    if (11 == sscanf( ( LPCTSTR ) Src , _T( "%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%d,%d" ) ,
      &m_iIndex ,
      &m_Center.x , &m_Center.y , &m_dExtremalAmpl , &m_dAverCent5x5 ,
      &m_DRect.left , &m_DRect.right , &m_DRect.top , &m_DRect.bottom ,
      &m_ImageMinBrightness , &m_ImageMaxBrightness ))
    {
      if (pName)
      {
        int iNamePos = ( int ) Src.Find( "Name=" ) ;
        if (iNamePos > 0)
        {
          pName->Empty() ;
          iNamePos += 5 ;
          LPCTSTR pChar = ( ( LPCTSTR ) ( *pName ) ) + iNamePos ;
          while (*pChar  &&  *pChar != _T( ';' ))
            *pName += *pChar ;
        }
      }
      return true ;
    }
    else
      return false ;
  }
  BOOL Serialize( LPBYTE* ppData , UINT* pDataLen ) const
  {
    if ( !ppData )
      return FALSE ;
    UINT cb = sizeof( *this ) ;
    UINT uiAllocLen = cb + sizeof( UINT ); // block len will be written before real data

    UINT uiOldDataLen = *pDataLen ;
    *pDataLen += uiAllocLen ;
    *ppData = ( LPBYTE )realloc( *ppData , *pDataLen );
    LPBYTE ptr = *ppData + uiOldDataLen ;
    *( ( UINT* )ptr ) = uiAllocLen ;
    ptr += sizeof( uiAllocLen ) ;
    memcpy( ptr , this , cb );
    return TRUE;
  }
  BOOL Restore( LPBYTE pData , UINT uiDataLen )
  {
    UINT uiLen = sizeof(*this) + sizeof(UINT) ;
    if ( !pData || uiDataLen < uiLen )
    {
      ASSERT( 0 ) ; // Not enough data for necessary block len
      return FALSE ;
    }
    UINT uiBlockLen = *( ( UINT* )pData ) ;
    UINT uiLineDataLen = uiBlockLen - sizeof( uiBlockLen ) ;
    if ( uiDataLen != sizeof(this) )
    {
      ASSERT( 0 ) ; // incompatible classes
      return FALSE ;
    }
    
    pData += sizeof( uiBlockLen ) ;
 
    memcpy( ( LPVOID )this , pData , uiLen );
    return TRUE;
  }
};

typedef FXArray <CLineResult> CLineResults ;

#endif  //__LINE_DATA_H_

