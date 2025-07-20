// TextFrame.h: interface for the CTextFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEXTFRAME_H__869B63DD_84C4_4DE2_9EA1_1A4BA1E42040__INCLUDED_)
#define AFX_TEXTFRAME_H__869B63DD_84C4_4DE2_9EA1_1A4BA1E42040__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>

class FX_EXT_GADGET CTextFrame : public CDataFrame
{
  FXString m_TextData;
protected:
  CTextFrame( LPCTSTR string )
  {
    m_DataType = text;
    if ( !string )
      m_TextData.Empty();
    else
    {
      m_TextData = string;
    }
  }
  CTextFrame( const CTextFrame* TextFrame )
  {
    m_DataType = text;
    m_TextData = TextFrame->m_TextData;
    CopyAttributes( TextFrame );
  }
  virtual ~CTextFrame() {};
public:
  const FXString& GetString() const { return m_TextData; }
  FXString& GetString() { return m_TextData; }
  void     SetString( LPCTSTR string ) { m_TextData = string; }
  CTextFrame* GetTextFrame( LPCTSTR label = DEFAULT_LABEL )
  {
    if ( !label || m_Label == label )
      return this;
    return NULL;
  }
  const CTextFrame* GetTextFrame( LPCTSTR label = DEFAULT_LABEL ) const
  {
    if ( !label || m_Label == label )
      return this;
    return NULL;
  }
  virtual  CDataFrame* Copy() const { return new CTextFrame( this ); }
  static CTextFrame* Create( LPCTSTR string = NULL ) { return new CTextFrame( string ); }
  BOOL IsNullFrame() const { return (m_TextData.GetLength() == 0); }

  virtual BOOL Serialize( LPBYTE* ppData , FXSIZE* cbData ) const
  {
    ASSERT( ppData );
    FXSIZE cb;
    if ( !CDataFrame::Serialize( ppData , &cb ) )
      return FALSE;
    *cbData = cb + m_TextData.GetLength() + 1;
    *ppData = (LPBYTE) realloc( *ppData , *cbData );
    memcpy( *ppData + cb , (LPCTSTR) m_TextData , m_TextData.GetLength() + 1 );
    return TRUE;
  }

  virtual BOOL Serialize( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const ;

  virtual FXSIZE GetSerializeLength( FXSIZE& uiLabelLen ,
    FXSIZE * pAttribLen = NULL ) const ;

  virtual BOOL Restore( LPBYTE lpData , FXSIZE cbData )
  {
    FXSIZE cb;
    if ( !CDataFrame::Serialize( NULL , &cb ) || !CDataFrame::Restore( lpData , cbData ) )
      return FALSE;
    LPBYTE ptr = lpData + cb;
    m_TextData = (LPCTSTR) ptr;
    return TRUE;
  }
  virtual void ToLogString( FXString& Output )
  {
    CDataFrame::ToLogString( Output );
    Output += m_TextData.GetLength() < 30 ? m_TextData : m_TextData.Left( 30 ) ;
  }
};

#ifndef SHBASE_CLI
using namespace std;

#include <complex>
typedef complex<double> cmplx ;

FX_EXT_GADGET CTextFrame * CreateTextFrameEx( cmplx& Pt , LPCTSTR Text ,
  DWORD Color = 0xc0ffc0 , int iSize = 16 ) ;

FX_EXT_GADGET CTextFrame * CreateTextFrameEx( cmplx& Pt ,
  DWORD Color , int iSize , LPCTSTR pContentFormat , ... ) ;

FX_EXT_GADGET CTextFrame * CreateTextFrameEx( LPCTSTR pLabel , LPCTSTR pContentFormat , ... ) ;


#endif // SHBASE_CLI

#endif // !defined(AFX_TEXTFRAME_H__869B63DD_84C4_4DE2_9EA1_1A4BA1E42040__INCLUDED_)
