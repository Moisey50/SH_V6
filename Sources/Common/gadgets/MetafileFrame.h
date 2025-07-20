// MetafileFrame.h: interface for the CMetafileFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_METAFILEFRAME_H__405A6A0B_0170_4F07_8370_31CC223B9038__INCLUDED_)
#define AFX_METAFILEFRAME_H__405A6A0B_0170_4F07_8370_31CC223B9038__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>

class FX_EXT_GADGET CMFHelper
{
protected:
  LPVOID          m_WMFData;
  UINT            m_WMFSize;
  HDC             m_MFHDC;
public:
  CMFHelper();
  CMFHelper( CMFHelper* wmHelper );
  ~CMFHelper();
  void Copy( const CMFHelper& mfSrc );
  CMFHelper& operator =( const CMFHelper& mfSrc )
  {
    Copy( mfSrc );
    return *this;
  }
  //  Get access to draw functions
  HDC     StartDraw( LPRECT rc );
  void    EndDraw();
  //  Get metafile - don't forget delete with DeleteMetaFile after using!
  HENHMETAFILE GetMF() { return SetEnhMetaFileBits( m_WMFSize , ( LPBYTE ) m_WMFData ); }
  void    GetMFData( LPVOID* Data , UINT* size ) const { *Data = m_WMFData; *size = m_WMFSize; }
  void    SetMFData( LPVOID Data , UINT size ) { ASSERT( m_WMFData == NULL );  m_WMFData = Data;     m_WMFSize = size; }
};

class FX_EXT_GADGET CMetafileFrame : public CDataFrame , public CMFHelper
{
protected:
  CMetafileFrame();
  CMetafileFrame( const CMetafileFrame* WMFFrame );
  ~CMetafileFrame();
public:
  CDataFrame* Copy() const { return new CMetafileFrame( this ); }
  CMetafileFrame* GetMetafileFrame( LPCTSTR label = DEFAULT_LABEL );
  const CMetafileFrame* GetMetafileFrame( LPCTSTR label = DEFAULT_LABEL ) const;
  static CMetafileFrame* Create();
  BOOL IsNullFrame() const;
  BOOL Serialize( LPBYTE* ppData , UINT* cbData ) const { ASSERT( FALSE ); return FALSE; }
  BOOL Restore( LPBYTE lpData , UINT cbData ) { ASSERT( FALSE ); return FALSE; }
};

class CMFHelpers : public FXArray<CMFHelper , CMFHelper&>
{
public:
  ~CMFHelpers() { RemoveAll(); }
};

#endif // !defined(AFX_METAFILEFRAME_H__405A6A0B_0170_4F07_8370_31CC223B9038__INCLUDED_)
