#pragma once
#include "cslideview.h"
#include <gadgets\gadbase.h>
#include <gadgets\videoframe.h>

typedef void DispFunc( CDataFrame* pDataFrame , void *pParam );

class CFrameBuffer : public FXArray<CDataFrame* , CDataFrame*>
{
private:
  FXLockObject m_Lock;
public:
  void RemoveAt( INT nIndex , INT nCount = 1 , bool doLock = true )
  {
    if ( doLock )
      m_Lock.Lock();
    if ( ( nIndex + nCount ) > GetSize() ) return;
    for ( int i = nIndex; i < nIndex + nCount; i++ )
    {
      if ( GetAt( i ) )
        GetAt( i )->Release();
    }
    FXArray::RemoveAt( nIndex , nCount );
    if ( doLock )
      m_Lock.Unlock();
  }
  void RemoveAll()
  {
    FXAutolock al( m_Lock );
    while ( GetSize() )
      RemoveAt( 0 , 1 , false );
  }
  void SetSize( INT nNewSize , INT nGrowBy = -1 )
  {
    FXAutolock al( m_Lock );
    FXArray::SetSize( nNewSize , nGrowBy );
  }
  void SetAt( INT nIndex , CDataFrame* newElement )
  {
    FXAutolock al( m_Lock );
    if ( GetAt( nIndex ) )
      GetAt( nIndex )->Release();
    FXArray::SetAt( nIndex , newElement );
  }
  void InsertAt( INT nIndex , CDataFrame* newElement )
  {
    FXAutolock al( m_Lock );
    FXArray::InsertAt( nIndex , newElement );
  }
  void Lock()
  {
    m_Lock.Lock();
  }
  void Unlock()
  {
    m_Lock.Unlock();
  }
};

class CSlideViewRender :
  public CSlideView
{
protected:
  bool            m_ShowMetafiles ;
  CFrameBuffer    m_FramesBuffer ;
  DispFunc*       m_pDispEventFunc ;
  void*           m_pDispEventParam ;
  int             m_iIntegrationRadius ;
public:
  CSlideViewRender( LPCTSTR name = "" );
  ~CSlideViewRender( void );
  void    ApplySettings( bool Shift , bool Rescale ,
    double Scale , bool Monochrome , int TargetWidth ,
    int FramesLen , int FramesInRow , bool   ShowMetafiles ,
    int iIntegrationRadius );
  void    Render( const CDataFrame* pDataFrame );
  void    OnItemSelected();
  void    SetDispFunc( DispFunc* df , void *pParam )
  {
    m_pDispEventFunc = df; m_pDispEventParam = pParam;
  }
  DWORD GetFrameCnt() { return (DWORD)m_FramesBuffer.GetCount() ;  }
  double GetAverageValue( int iBufIndex , int iX , int iY ) ;
  const CVideoFrame * GetFrame( int iBufIndex )
  {
    if ( m_iIntegrationRadius >= 0
      && ( 0 <= iBufIndex ) && ( iBufIndex < m_FramesBuffer.GetCount() ) )
    {
      CDataFrame * pData = m_FramesBuffer[ iBufIndex ] ;
      if ( pData )
      {
        const CVideoFrame * pVF = pData->GetVideoFrame() ;
        if ( pVF && pVF->lpBMIH )
          return pVF ;
      }
    }
    return NULL ;
  }
protected:
  CVideoFrame* MergeLayers( const CDataFrame* pDataFrame );
};
