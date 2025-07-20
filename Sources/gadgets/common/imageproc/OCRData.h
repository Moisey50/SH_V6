#ifndef __OCR_DATA_H_
#define __OCR_DATA_H_

#include <classes\DRect.h>

#ifndef EMBEDDED_OBJ_NAME_LEN
#define EMBEDDED_OBJ_NAME_LEN  30
#define MAX_OBJ_NAME_LEN       (EMBEDDED_OBJ_NAME_LEN - 1)
#endif

class COCRResult
{
public:
  VOBJ_TYPE m_Type ; // == OCR (for compatibility)
  char    m_szName[ EMBEDDED_OBJ_NAME_LEN ] ;
  FXString m_OCRText ;

  COCRResult() 
  { 
    m_Type = OCR ;
    memset( m_szName , 0 , sizeof( m_szName ) ) ; 
  } ;
  COCRResult( COCRResult& Orig )
  {
    m_Type = OCR ;
    strcpy_s( m_szName , Orig.m_szName ) ;
    m_OCRText = Orig.m_OCRText ;
  }
  ~COCRResult() {} ;
  COCRResult& operator =( const COCRResult& Orig )
  {
    if ( this != &Orig )
    {
      m_Type = Orig.m_Type ;
      strcpy_s( m_szName , Orig.m_szName ) ;
      m_OCRText = Orig.m_OCRText ;
    }
    return *this ;
  }

  void ToString( FXString& Result )
  {
    Result.Format("%s (Name=%s); \n" , (LPCTSTR) m_OCRText , m_szName ) ;
  }
  bool FromString( FXString& Src )
  {
    TCHAR Buf[ 2000 ] ;
    memset( m_szName , 0 , sizeof( m_szName ) ) ;
    if ( sscanf( ( LPCTSTR )Src , _T( "%s" ) , Buf ) )
    {
      int iNamePos = (int) Src.Find( "Name=" ) ;
      if ( iNamePos > 0 )
      {
        iNamePos += 5 ;
        LPCTSTR pChar = (LPCTSTR)Src + iNamePos ;
        LPTSTR pName = m_szName ;
        while ( *pChar  &&  *pChar != _T( ';' )  )
        {
          *( pName++ ) = *pChar ;
          if ( ( pName - m_szName ) >= MAX_OBJ_NAME_LEN )
            break ;
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
    UINT uiOCRTextLen = (int) m_OCRText.GetLength() ;
    UINT cb = sizeof(VOBJ_TYPE) + sizeof(m_szName) + uiOCRTextLen + 1 ;
    UINT uiAllocLen = cb + sizeof( UINT ); // block len will be written before real data

    UINT uiOldDataLen = *pDataLen ;
    *pDataLen += uiAllocLen ;
    *ppData = ( LPBYTE )realloc( *ppData , *pDataLen );
    LPBYTE ptr = *ppData + uiOldDataLen ;
    *( ( UINT* )ptr ) = uiAllocLen ;
    ptr += sizeof( uiAllocLen ) ;
    *( ( VOBJ_TYPE* )ptr ) = m_Type ;
    ptr += sizeof( VOBJ_TYPE ) ;
    memcpy( ptr , m_szName , sizeof( m_szName ) );
    ptr += sizeof( m_szName ) ;
    memcpy( ptr , ( LPCTSTR )m_OCRText , uiOCRTextLen ) ;
    ptr += uiOCRTextLen ;
    *ptr = '\0' ;
    return TRUE;
  }
  BOOL Restore( LPBYTE pData , UINT uiDataLen )
  {
    UINT uiLen = sizeof(*this) + sizeof(UINT) ;
    if ( !pData )
    {
      ASSERT( 0 ) ; // Not enough data for necessary block len
      return FALSE ;
    }
    UINT uiBlockLen = *( ( UINT* )pData ) ;
    pData += sizeof( uiBlockLen ) ;
    UINT uiTypeAndNameLen = sizeof( VOBJ_TYPE ) + sizeof( m_szName ) ;
    memcpy( &m_Type , pData , uiTypeAndNameLen ) ;
    pData += uiTypeAndNameLen ;
    m_OCRText = pData ;
    return TRUE;
  }
};

#endif  //__OCR_DATA_H_

