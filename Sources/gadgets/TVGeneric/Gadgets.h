// Gadgets.h: interface for the Gadgets class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GADGETS_H__628AE475_517F_48CD_B85D_20F15A54E84E__INCLUDED_)
#define AFX_GADGETS_H__628AE475_517F_48CD_B85D_20F15A54E84E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _TRACE_DATAFRAMERELEASE
#define GADEGTNAME m_Name
#else
#define GADEGTNAME
#endif
#include <gadgets\textframe.h>
#include <gadgets\videoframe.h>

class Delay : public CFilterGadget
{
  DWORD delay;
public:
  virtual void ShutDown();
  bool PrintProperties( FXString& text );
  bool ScanProperties( LPCTSTR text , bool& Invalidate );
  bool ScanSettings( FXString& text );
private:
  Delay();
  virtual CDataFrame* DoProcessing( const CDataFrame* pDataFrame )
  {
    Sleep( delay );
    ((CDataFrame*) pDataFrame)->AddRef();
    return (CDataFrame*) pDataFrame;
  };
  DECLARE_RUNTIME_GADGET( Delay );
};

__forceinline Delay::Delay() :
  delay( 1 )
{
  m_pInput = new CInputConnector( transparent );
  m_pOutput = new COutputConnector( transparent );
  Resume();
}

__forceinline void Delay::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
}

__forceinline bool Delay::PrintProperties( FXString& text )
{
  FXPropertyKit pk;
  pk.WriteInt( "Delay" , delay );
  text = pk;
  return true;
}

__forceinline bool Delay::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pk( text );
  pk.GetInt( "Delay" , (int&) delay );
  return true;
}

__forceinline bool Delay::ScanSettings( FXString& text )
{
  text = "template(Spin(Delay,1,10000))";
  return true;
}


class ChangeID : public CFilterGadget
{
  int m_Item;
public:
  virtual void ShutDown();
  bool PrintProperties( FXString& text );
  bool ScanProperties( LPCTSTR text , bool& Invalidate );
  bool ScanSettings( FXString& text );
private:
  ChangeID();
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );
  DECLARE_RUNTIME_GADGET( ChangeID );
};

__forceinline ChangeID::ChangeID() :
  m_Item( 1 )
{
  m_OutputMode = modeReplace;
  m_pInput = new CInputConnector( transparent );
  m_pOutput = new COutputConnector( transparent );
  m_Item = AfxGetApp()->GetProfileInt( "root" , "shift" , 1 );
  Resume();
}

__forceinline void ChangeID::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
  AfxGetApp()->WriteProfileInt( "root" , "shift" , m_Item );
}

__forceinline CDataFrame* ChangeID::DoProcessing( const CDataFrame* pDataFrame )
{
  ASSERT( pDataFrame != NULL );
  int id = pDataFrame->GetId();
  CDataFrame* retV = pDataFrame->Copy();
  retV->ChangeId( (m_Item != 0) ? id + m_Item : 0 );
  return retV;
}

__forceinline bool ChangeID::PrintProperties( FXString& text )
{
  FXPropertyKit pk;
  pk.WriteInt( "Add_to_ID" , m_Item );
  text = pk;
  return true;
}

__forceinline bool ChangeID::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pk( text );
  pk.GetInt( "Add_to_ID" , m_Item );
  return true;
}

__forceinline bool ChangeID::ScanSettings( FXString& text )
{
  text = "template(Spin(Add_to_ID,-99,99))";
  return true;
}

class RenderID : public CRenderGadget
{
  CTextView*  m_wndOutput;
  DWORD       m_FramesCnt;
  FXLockObject m_Lock;
public:
  virtual void ShutDown();
  virtual void Attach( CWnd* pWnd );
  virtual void Detach();
  CWnd* GetRenderWnd() { return m_wndOutput; }
  virtual void GetDefaultWndSize( RECT& rc ) { rc.left = rc.top = 0; rc.right = 2 * DEFAULT_GADGET_WIDTH; rc.bottom = DEFAULT_GADGET_HEIGHT; }
private:
  RenderID();
  virtual void Render( const CDataFrame* pDataFrame );

  DECLARE_RUNTIME_GADGET( RenderID );
};

__forceinline RenderID::RenderID() :
  m_wndOutput( NULL ) ,
  m_FramesCnt( 0 )
{
  m_pInput = new CInputConnector( nulltype );
  Resume();
}

__forceinline void RenderID::ShutDown()
{
  Detach();
  CRenderGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
}

__forceinline void RenderID::Attach( CWnd* pWnd )
{
  Detach();
  m_wndOutput = new CTextView;
  m_wndOutput->Create( pWnd );
  m_wndOutput->SetText( "" );
}

__forceinline void RenderID::Detach()
{
  VERIFY( m_Lock.LockAndProcMsgs() );
  if ( m_wndOutput )
  {
    if ( m_wndOutput->IsValid() )
      m_wndOutput->DestroyWindow();
    delete m_wndOutput; m_wndOutput = NULL;
  }
  m_Lock.Unlock();
}

__forceinline void RenderID::Render( const CDataFrame* pDataFrame )
{
  FXAutolock lock( m_Lock );
  if ( (m_wndOutput == NULL) || (!m_wndOutput->IsValid()) ) return;
  CString text;
  double frTime = pDataFrame->GetTime();
  if ( Tvdb400_IsEOS( pDataFrame ) )
  {
    m_FramesCnt = 0;
    if ( frTime != -1 )
      text.Format( "ID: EOS\nTime:%.0f\nType:%s" , frTime , Tvdb400_TypeToStr( pDataFrame->GetDataType() ) );
    else
      text.Format( "ID: EOS\nTime: Undefined\nType:%s" , Tvdb400_TypeToStr( pDataFrame->GetDataType() ) );
  }
  else
  {
    if ( frTime != -1 )
    {
      CString time;
      unsigned sec = (unsigned) (frTime / 1000000);
      int hours = sec / 3600;
      int mins = (sec % 3600) / 60;
      sec %= 60;
      int msec = ((unsigned) (frTime / 1000)) % 1000;
      time.Format( "%02d:%02d:%02d.%03d" , hours , mins , sec , msec );
      //text.Format("ID:%d\nTime:%.0f\nType:%s", pDataFrame->GetId(),frTime,Tvdb400_TypeToStr(pDataFrame->GetDataType()));
      text.Format( "ID:%d\nTime:%s\nType:%s\nFrame # %d" , pDataFrame->GetId() , time , Tvdb400_TypeToStr( pDataFrame->GetDataType() ) , m_FramesCnt );
    }
    else
      text.Format( "ID:%d\nTime: Undefined\nType:%s\nFrame # %d" , pDataFrame->GetId() , Tvdb400_TypeToStr( pDataFrame->GetDataType() ) , m_FramesCnt );
    m_FramesCnt++;
  }
  m_wndOutput->SetText( text );
}

class RenderFrameRate : public CRenderGadget ,
  public FXTimer
{
  DWORD        m_FrCnt;
  double       m_dLastFrameTime ;
  double       m_dLastInterval ;
  double       m_dAverageInterval ;
  double       m_dMinInterval ;
  double       m_dMaxInterval ;
  double       m_dCoeff ;
  int          m_iAverageMax ;
  DWORD        m_LstTick;
  CTextView   *m_wndOutput;
  FXLockObject m_Lock;
  datatype     m_LastType ;
  DWORD        m_dwLastID ;
  CSize        m_LastVFrameSize ;
  UINT         m_LastVFrameFormat ;
  char         m_LastLabel[ 300 ] ;
public:
  virtual void ShutDown();
  virtual void    Create();
  virtual void    Attach( CWnd* pWnd );
  virtual void    Detach();
  CWnd*   GetRenderWnd() { return m_wndOutput; }
  virtual void    GetDefaultWndSize( RECT& rc ) { rc.left = rc.top = 0; 
  rc.right = 2 * DEFAULT_GADGET_WIDTH ; rc.bottom = DEFAULT_GADGET_HEIGHT ; }
  virtual void    OnAlarm( DWORD TimeID );
private:
  RenderFrameRate();
  virtual void Render( const CDataFrame* pDataFrame );

  DECLARE_RUNTIME_GADGET( RenderFrameRate );
};

__forceinline RenderFrameRate::RenderFrameRate() :
  m_FrCnt( 0 ) ,
  m_LstTick( -1 ) ,
  FXTimer( _T( "RenderFrameRate" ) ) ,
  m_wndOutput( NULL )
{
  memset( m_LastLabel , 0 , sizeof( m_LastLabel ) ) ;
  m_pInput = new CInputConnector( nulltype );
  Resume();
}

__forceinline void RenderFrameRate::ShutDown()
{
  AlarmOff();
  CRenderGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  Detach();
}

__forceinline void RenderFrameRate::Render( const CDataFrame* pDataFrame )
{
  if ( (m_wndOutput == NULL) || (!m_wndOutput->IsValid()) )
    return;
  else
  {
    m_FrCnt++;
    double dTime = GetHRTickCount() ;
    if ( m_dLastFrameTime != 0. )
      m_dLastInterval = dTime - m_dLastFrameTime ;
    m_dLastFrameTime = dTime ;
    m_dAverageInterval *= m_dCoeff ;
    m_dAverageInterval += (1. - m_dCoeff) * m_dLastInterval ;
    if ( m_dMinInterval == 0. || (m_dMinInterval > m_dLastInterval) )
      m_dMinInterval = m_dLastInterval ;
    if ( m_dMaxInterval == 0. || (m_dMaxInterval < m_dLastInterval) )
      m_dMaxInterval = m_dLastInterval ;

//     if ( m_FrCnt % m_iAverageMax == 0 )
//       m_dMaxInterval = m_dMinInterval = 0. ;
    m_LastType = pDataFrame->GetDataType() ;
    m_dwLastID = pDataFrame->GetId() ;
    strcpy_s( m_LastLabel , sizeof( m_LastLabel ) - 2 , pDataFrame->GetLabel() ) ;
    if ( m_LastType == vframe )
    {
      const CVideoFrame * pv = pDataFrame->GetVideoFrame() ;
      m_LastVFrameSize.cx = GetWidth( (pTVFrame)pv ) ;
      m_LastVFrameSize.cy = GetHeight( (pTVFrame)pv ); 
      m_LastVFrameFormat = pv->lpBMIH->biCompression ;
    }
  }
}

__forceinline void RenderFrameRate::OnAlarm( DWORD TimeID )
{
  FXAutolock al( m_Lock );
  if ( m_LstTick == -1 )
  {
    m_LstTick = GetTickCount();
    m_FrCnt = 0;
  }
  else
  {
    DWORD tick = GetTickCount();
    if ( m_wndOutput->GetSafeHwnd() )
    {
      TCHAR buf[300];


      //       double frRate=0.0;
      //       frRate=((double)m_FrCnt)*1000.0/(tick-m_LstTick);
      //       text.Format("%.2f", frRate);
      if ( m_LastType != vframe )
      {
        sprintf_s( buf , "FPS=%g\nTavg=%gms\n[%g,%g]\nId=%u Lab=%s\n%s" ,
          m_dAverageInterval != 0. ? 1000. / m_dAverageInterval : 0. ,
          m_dAverageInterval ,
          m_dMinInterval , m_dMaxInterval , m_dwLastID , m_LastLabel ,
          ( LPCTSTR )Tvdb400_TypeToStr( m_LastType ) ) ;
      }
      else
      {
        sprintf_s( buf ,"FPS=%g\nTavg=%gms\n[%g,%g]\nID=%u %.11s\n%s  %dx%d" ,
          m_dAverageInterval != 0. ? 1000. / m_dAverageInterval : 0. ,
          m_dAverageInterval ,
          m_dMinInterval , m_dMaxInterval , m_dwLastID , m_LastLabel ,
          GetVideoFormatName( m_LastVFrameFormat ) ,
          m_LastVFrameSize.cx , m_LastVFrameSize.cy ) ;
      }
      m_wndOutput->SetText( buf );
    }
    m_LstTick = tick; m_FrCnt = 0;
    m_dMaxInterval = m_dMinInterval = 0. ;
  }
}

__forceinline void RenderFrameRate::Create()
{
  m_dLastFrameTime = m_dLastInterval = m_dAverageInterval = m_dMinInterval
    = m_dMaxInterval = 0. ;
  m_iAverageMax = 20 ;
  m_dCoeff = (double) (m_iAverageMax - 1) / (double) m_iAverageMax ;
  SetAlarmFreq( 1.0 );
  AlarmOn();
}

__forceinline void RenderFrameRate::Attach( CWnd* pWnd )
{
  Detach();
  m_wndOutput = new CTextView();
  m_wndOutput->Create( pWnd );
  m_wndOutput->SetText( "" );
}

__forceinline void RenderFrameRate::Detach()
{
  VERIFY( m_Lock.LockAndProcMsgs() );
  if ( m_wndOutput )
  {
    if ( m_wndOutput->IsValid() )
      m_wndOutput->DestroyWindow();
    delete m_wndOutput; m_wndOutput = NULL;
  }
  m_Lock.Unlock();
}

class RenderContainer : public CRenderGadget
{
  CContainerView* m_wndOutput;
  FXLockObject    m_Lock;
public:
  virtual void ShutDown();
  virtual void	Attach( CWnd* pWnd );
  virtual void	Detach();
  CWnd*	GetRenderWnd() { return m_wndOutput; }
  virtual void	GetDefaultWndSize( RECT& rc ) { rc.left = rc.top = 0; rc.right = 4 * DEFAULT_GADGET_WIDTH; rc.bottom = DEFAULT_GADGET_HEIGHT; }
private:
  RenderContainer();
  virtual void Render( const CDataFrame* pDataFrame );
  DECLARE_RUNTIME_GADGET( RenderContainer );
};

__forceinline RenderContainer::RenderContainer() : m_wndOutput( NULL )
{
  m_pInput = new CInputConnector( nulltype );
  Resume();
}

__forceinline void RenderContainer::ShutDown()
{
  Detach();
  CRenderGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
}

__forceinline void RenderContainer::Attach( CWnd* pWnd )
{
  Detach();
  m_wndOutput = new CContainerView();
  m_wndOutput->Create( pWnd );
}

__forceinline void RenderContainer::Detach()
{
  VERIFY( m_Lock.LockAndProcMsgs() );
  if ( m_wndOutput )
  {
    if ( ::IsWindow( m_wndOutput->GetSafeHwnd() ) )
      m_wndOutput->DestroyWindow();
    delete m_wndOutput; m_wndOutput = NULL;
  }
  m_Lock.Unlock();
}

__forceinline void RenderContainer::Render( const CDataFrame* pDataFrame )
{
  FXAutolock lock( m_Lock );
  if ( m_wndOutput )
  {
    m_wndOutput->SetRedraw( FALSE );
    m_wndOutput->Render( pDataFrame );
    m_wndOutput->SetRedraw( TRUE );
    m_wndOutput->Invalidate( FALSE );
  }
}

class SerializeTest : public CFilterGadget
{
public:
  virtual void ShutDown()
  {
    CFilterGadget::ShutDown();
    delete m_pInput;
    m_pInput = NULL;
    delete m_pOutput;
    m_pOutput = NULL;
  }
  bool PrintProperties( FXString& text ) { return true; }
  bool ScanProperties( LPCTSTR text , bool& Invalidate ) { return true; }
  bool ScanSettings( FXString& text );
private:
  SerializeTest()
  {
    m_pInput = new CInputConnector( transparent );
    m_pOutput = new COutputConnector( transparent );
    Resume();
  }
  virtual CDataFrame* DoProcessing( const CDataFrame* pDataFrame )
  {
    LPBYTE lpData;
    FXSIZE cbData;
    CDataFrame* pResultsFrame = NULL;
    if ( Tvdb400_Serialize( pDataFrame , &lpData , &cbData ) )
    {
      pResultsFrame = Tvdb400_Restore( lpData , cbData );
      free( lpData );
    }
    return pResultsFrame;
  }
  DECLARE_RUNTIME_GADGET( SerializeTest );
};

#endif // !defined(AFX_GADGETS_H__628AE475_517F_48CD_B85D_20F15A54E84E__INCLUDED_)

class Meas_dT : public CFilterGadget
{
public:
  virtual void ShutDown()
  {
    CFilterGadget::ShutDown();
    delete m_pInput;
    m_pInput = NULL;
    delete m_pOutput;
    m_pOutput = NULL;
  }
  //   bool PrintProperties( FXString& text ) { return true; }
  //   bool ScanProperties( LPCTSTR text , bool& Invalidate ) { return true; }
  //   bool ScanSettings( FXString& text ) { return true; };
private:
  double m_dPrevTime ;
  Meas_dT()
  {
    m_pInput = new CInputConnector( transparent );
    m_pOutput = new COutputConnector( transparent );
    m_dPrevTime = GetHRTickCount() ;
    Resume();
  }
  virtual CDataFrame* DoProcessing( const CDataFrame* pDataFrame )
  {
    double dNow = GetHRTickCount() ;
    double dDelta = dNow - m_dPrevTime ;
    m_dPrevTime = dNow ;
    CTextFrame* pResultsFrame = CTextFrame::Create() ;
    FXString& ResultAsText = pResultsFrame->GetString() ;
    ResultAsText.Format( "%.3f" , dDelta ) ;
    pResultsFrame->CopyAttributes( pDataFrame ) ;
    return pResultsFrame;
  }
  DECLARE_RUNTIME_GADGET( Meas_dT );
};
