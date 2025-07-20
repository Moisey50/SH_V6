// SlideView.h : Implementation of the SlideView class


#include "StdAfx.h"
#include "SlideView.h"
#include <math\Intf_sup.h>
#include <helpers\FramesHelper.h>

IMPLEMENT_RUNTIME_GADGET_EX( SlideView , CRenderGadget , "Video.renderers" , TVDB400_PLUGIN_NAME );

void SlideView_DispFunc( CDataFrame* pDataFrame , void *pParam )
{
  ( ( SlideView* )pParam )->DispFunc( pDataFrame );
}

//////////////////////////////////////////////////////////////////////

SlideView::SlideView( void ) :
m_wndOutput( NULL ) ,
m_Rescale( true ) ,
m_Scale( 1 ) ,
m_Monochrome( false ) ,
m_TargetWidth( 200 ) ,
m_FramesLen( 16 ) ,
m_FramesInRow( 4 ) ,
m_ShowMetafiles( false ) ,
m_Shift( false ) ,
m_iIntegrationRadius(-1),
m_bShiftWasPressed( false ),
m_bCtrlWasPressed(false) ,
m_iColorCnt(0) ,
m_FigureLabel( _T("NameNotSet") )
{
  m_pInput = new CInputConnector( transparent );
  m_pOutput = new COutputConnector( vframe );
  m_pSpectrumOutput = new COutputConnector( figure );
  m_pSpectrumAsTextOutput = new COutputConnector( text ) ;
  m_Colors.Add( "color=0x0000ff;" ) ;
  m_Colors.Add( "color=0x00ff00;" ) ; 
  m_Colors.Add( "color=0xff0000;" ) ;
  m_Colors.Add( "color=0x00ffff;" ) ;
  m_Colors.Add( "color=0xff00ff;" ) ;
  m_Colors.Add( "color=0xffff00;" ) ;
  m_Colors.Add( "color=0xffffff;" ) ;
  m_Colors.Add( "color=0x0040c0;" ) ;
  m_Colors.Add( "color=0x00c040;" ) ;
  m_Colors.Add( "color=0xc04000;" ) ;
  m_Colors.Add( "color=0x40c000;" ) ;
  m_Colors.Add( "color=0xc00040;" ) ;
  m_Colors.Add( "color=0x4000c0;" ) ;
  m_Colors.Add( "color=0x404080;" ) ;
  m_Colors.Add( "color=0x408040;" ) ;
  m_Colors.Add( "color=0x804040;" ) ;
  Resume();
}

void SlideView::ShutDown()
{
  Detach();
  CRenderGadget::ShutDown();
  delete m_pInput; m_pInput = NULL;
  delete m_pOutput; m_pOutput = NULL;
  delete m_pSpectrumOutput; m_pSpectrumOutput = NULL;
  delete m_pSpectrumAsTextOutput ; m_pSpectrumAsTextOutput = NULL ;
  if ( m_wndOutput )
    m_wndOutput->DestroyWindow();
  delete m_wndOutput; m_wndOutput = NULL;
}

void SlideView::Attach( CWnd* pWnd )
{
  Detach();
  m_wndOutput = new CSlideViewRender( m_Monitor );
  m_wndOutput->Create( pWnd );
  m_wndOutput->SetDispFunc( SlideView_DispFunc , this );
  ApplySettings();
}

void SlideView::Detach()
{
  m_Lock.LockAndProcMsgs();
  if ( ::IsWindow( m_wndOutput->GetSafeHwnd() ) )
  {
    m_wndOutput->SetDispFunc( NULL , NULL );
    m_wndOutput->DestroyWindow();
  }
  if ( m_wndOutput ) 
  {
    delete m_wndOutput;
    m_wndOutput = NULL;
  }
  m_Lock.Unlock();
}

#define KB_CTRL    0x01
#define KB_SHIFT   0x02
#define KB_LCNTRL  0x04
#define KB_RCNTRL  0x08
#define KB_LSHIFT  0x10
#define KB_RSHIFT  0x20

void SlideView::Render( const CDataFrame* pDataFrame )
{
  FXAutolock al( m_Lock );
  if ( ( m_wndOutput ) && ( ::IsWindow( m_wndOutput->GetSafeHwnd() ) ) )
  {
    int iOriginalFrameCnt = m_wndOutput->GetFrameCnt() ;
    const CTextFrame * pTextFrame = pDataFrame->GetTextFrame() ;
    if ( pTextFrame )
    {
      LPCTSTR pLabel = ( LPCTSTR )pTextFrame->GetLabel() ;
      if ( strcmp( pLabel , "Caption" ) == 0 ) 
      {
        m_LastCaption = pTextFrame->GetString() ;
      }
      else if ( m_wndOutput->GetFrameCnt() 
        && strcmp( pLabel , "GetSpectrum" ) == 0 )
      {
  //       bool bKbShiftPressed = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0 ;
        FXPropertyKit pc = pTextFrame->GetString() ;
        int iX , iY ;
        if ( pc.GetInt( "x" , iX ) && pc.GetInt( "y" , iY ) )
        {  // OK, coordinates and status available
          int KBKeys = 0 ;
          pc.GetInt( "Keys" , KBKeys ) ;
        
          CFigureFrame * pGraphData = NULL ;
          const CVideoFrame * pVF = m_wndOutput->GetFrame( 0 ) ;
          if ( pVF )
          {
            DWORD dwHeight = GetHeight( pVF ) ;
            DWORD dwWidth = GetWidth( pVF ) ;
            DWORD dwInRow = m_iIntegrationRadius * 2 + 1 ;
            DWORD dwStep = (dwInRow < 2) ? dwWidth : dwWidth - dwInRow ;
            BOUND( iX , m_iIntegrationRadius , dwWidth - m_iIntegrationRadius - 1 ) ;
            BOUND( iY , m_iIntegrationRadius , dwHeight - m_iIntegrationRadius - 1 ) ;
            DWORD dwShift = dwWidth * (iY - m_iIntegrationRadius) 
              + iX - m_iIntegrationRadius ;
            pGraphData = CFigureFrame::Create() ;
            if ( pGraphData )
            {
              for ( int i = 0 ; i < iOriginalFrameCnt ; i++ )
              {
                const CVideoFrame * pForSpectrum = m_wndOutput->GetFrame( i ) ;
                if ( pForSpectrum )
                {
                  LPBYTE pImage = GetData( pForSpectrum ) ;
                  if ( pImage )
                  {
                    double dValue = GetAverageValue( pImage , dwShift ,
                      dwInRow , dwStep , is16bit( pForSpectrum ) ) ;
                    pGraphData->Add( CDPoint( i , dValue ) ) ;
                  }
                }
              }
              if ( ( KBKeys & KB_SHIFT ) && !m_bShiftWasPressed )
              {
                m_bShiftWasPressed = true ;
                FXString Label ;
                Label.Format( "Replace:%dVals:Pt_X%d_Y%d" , 
                  m_wndOutput->GetFrameCnt() , iX , iY ) ;
                pGraphData->SetLabel( Label ) ;
                *( pGraphData->Attributes() ) = m_Colors[ ++m_iColorCnt ] ;
                if ( m_iColorCnt >= m_Colors.GetCount() )
                  m_iColorCnt = 0 ;
              }
              else
              {
                m_bShiftWasPressed = (( KBKeys & KB_SHIFT ) != 0) ;
                pGraphData->SetLabel( "Replace:Unknown" ) ;
                *( pGraphData->Attributes() ) = _T( "color=0x008000;" ) ;
              }
              if ( (KBKeys & KB_CTRL) && m_bCtrlWasPressed )
              {
                m_bCtrlWasPressed = true ;
                if ( m_pSpectrumAsTextOutput->IsConnected() )
                {
                  FXString Label ;
                  Label.Format( "%dVals:Pt_X%d_Y%d\n" ,
                    m_wndOutput->GetFrameCnt() , iX , iY ) ;
                  CTextFrame * pSpectrumAsText = CTextFrame::Create( Label ) ;
                  FXString Addition ;
                  bool bIsCaptions = ( m_Captions.GetCount() >= pGraphData->GetCount() ) ;
                  for ( int i = 0 ; i < pGraphData->GetCount() ; i++ )
                  {
                    Addition.Format( "%s , %g\n" , bIsCaptions? m_Captions[ i ] : "0" ,
                      pGraphData->GetAt( i ).y ) ;
                    pSpectrumAsText->GetString() += Addition ;
                  }
                  PutFrame( m_pSpectrumAsTextOutput , pSpectrumAsText ) ;
                }
              }
              else 
                m_bCtrlWasPressed = (( KBKeys & KB_CTRL ) != 0) ;

              PutFrame( m_pSpectrumOutput , pGraphData ) ;
            }
          }
        }
      }
    }
    m_wndOutput->Render( pDataFrame );
    int iNewLen = m_wndOutput->GetFrameCnt() ;
    if ( iNewLen != iOriginalFrameCnt )
    {
      if ( iNewLen > iOriginalFrameCnt && !m_LastCaption.IsEmpty() )
      {
        m_Captions.Add( m_LastCaption ) ;
      }
      else
      {
        m_Captions.RemoveAll() ;
        m_LastCaption.Empty() ;
      }
    }
    m_wndOutput->Invalidate( FALSE );
  }
}

void SlideView::DispFunc( CDataFrame* pDataFrame )
{
  if ( pDataFrame )
  {
    pDataFrame->AddRef();
    if ( !m_pOutput->Put( pDataFrame ) )
      pDataFrame->Release();
  }
}

void SlideView::ApplySettings()
{
  if ( m_wndOutput )
  {
    m_wndOutput->ApplySettings( m_Shift , m_Rescale ,
      ( ( m_Scale == 0 ) ? -1 : m_Scale ) , m_Monochrome ,
      m_TargetWidth , m_FramesLen , m_FramesInRow , 
      m_ShowMetafiles , m_iIntegrationRadius );
  }
}

//////////////////////////////////////////////////////////////////////

bool SlideView::PrintProperties( FXString& text )
{
  CRenderGadget::PrintProperties( text );
  FXPropertyKit pc;
  pc.WriteBool( "ShiftMode" , m_Shift );
  pc.WriteBool( "Rescale" , m_Rescale );
  pc.WriteBool( "Monochrome" , m_Monochrome );
  pc.WriteInt( "Scale" , m_Scale );
  pc.WriteInt( "TargetWidth" , m_TargetWidth );
  pc.WriteInt( "FramesLen" , m_FramesLen );
  pc.WriteInt( "FramesInRow" , m_FramesInRow );
  pc.WriteBool( "ShowMetafiles" , m_ShowMetafiles );
  pc.WriteInt( "IntegrRadius" , m_iIntegrationRadius ) ;
  pc.WriteString( "FigureLabel" , m_FigureLabel ) ;
  text += pc;
  return true;
}

bool SlideView::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  CString tmpS;
  CRenderGadget::ScanProperties( text , Invalidate );
  FXPropertyKit pc( text );
  pc.GetBool( "ShiftMode" , m_Shift );
  pc.GetInt( "Scale" , m_Scale );
  pc.GetBool( "Monochrome" , m_Monochrome );
  pc.GetInt( "TargetWidth" , m_TargetWidth );
  pc.GetInt( "FramesLen" , m_FramesLen );
  pc.GetInt( "FramesInRow" , m_FramesInRow );
  bool oRescale = m_Rescale; pc.GetBool( "Rescale" , m_Rescale );
  if ( oRescale != m_Rescale )
  {
    Invalidate = true;
    m_TargetWidth = 100;
  }
  pc.GetBool( "ShowMetafiles" , m_ShowMetafiles );
  pc.GetInt( "IntegrRadius" , m_iIntegrationRadius ) ;
  pc.GetString( "FigureLabel" , m_FigureLabel ) ;
  ApplySettings();
  return true;
}

bool SlideView::ScanSettings( FXString& text )
{
  text = "template("
    "ComboBox(Scale(Fit_window(0),x1(1),x2(2),x4(4),x8(8),x16(16))),"
    "ComboBox(Monochrome(False(false),True(true))),"
    "ComboBox(ShiftMode(False(false),True(true))),"
    "ComboBox(Rescale(False(false),True(true))),";
  if ( m_Rescale )
    text += "ComboBox(TargetWidth(100(100),200(200),300(300))),";
  text += "Spin(FramesLen,1,640),"
          "Spin(FramesInRow , 1 , 32),"
          "ComboBox(ShowMetafiles(False(false),True(true))),"
          "Spin(IntegrRadius,-1,100),"
          "EditBox(FigureLabel)"
          ")";
  return true;
}

double SlideView::GetAverageValue( LPBYTE pIm , DWORD dwShift ,
  DWORD dwZone , DWORD dwStep , bool b16 )
{
  double dResult = 0. ;
  if ( !b16 )
  {
    LPBYTE pImage = pIm + dwShift ;
    if ( dwZone < 2 )
      return ( double )( *pImage ) ;
    for ( DWORD dwYCnt = 0 ; dwYCnt < dwZone ; dwYCnt++ )
    {
      for ( DWORD dwXCnt = 0 ; dwXCnt < dwZone ; dwXCnt++ )
        dResult += *( pImage++ ) ;
      pImage += dwStep ;
    }
  }
  else
  {
    LPWORD pImage = ((LPWORD)pIm) + dwShift ;
    if ( dwZone < 2 )
      return ( double )( *pImage ) ;
    for ( DWORD dwYCnt = 0 ; dwYCnt < dwZone ; dwYCnt++ )
    {
      for ( DWORD dwXCnt = 0 ; dwXCnt < dwZone ; dwXCnt++ )
        dResult += *( pImage++ ) ;
      pImage += dwStep ;
    }
  }
  return dResult / ( dwZone * dwZone ) ;
}

