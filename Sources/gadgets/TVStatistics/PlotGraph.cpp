#include "StdAfx.h"
#include "PlotGraph.h"
#include <gadgets\TextFrame.h>
#include <helpers\propertykitEx.h>

#ifdef THIS_MODULENAME
#undef THIS_MODULENAME 
#endif
#define THIS_MODULENAME _T("PlotGraph")


IMPLEMENT_RUNTIME_GADGET_EX(PlotGraph, CRenderGadget, "Data.renderers", TVDB400_PLUGIN_NAME);

static const UINT VM_TVDB400_SET_GADGET_PROPERTIES = ::RegisterWindowMessage( _T( "Tvdb400_SetGadgetProperties" ) );


PlotGraph::PlotGraph():
  m_wndOutput(NULL),
  m_bFit(TRUE),
  m_iNSamples(320) ,
  m_bViewNames( TRUE ),
  m_bViewRanges( TRUE ),
  m_pOwnWnd( NULL ) ,
  m_hExternalWnd( NULL ) ,
  m_pAttachedWnd( NULL ) ,
  m_iNAttachments( 0 ) ,
  m_dSamplePeriod_ms( 0. )
{
  m_pInput = new CInputConnector(transparent);
  m_pOutput = new COutputConnector(text);
  m_MinMaxes.m_dXMin = 0. ;
  m_MinMaxes.m_dXMax = 640. ;
  m_MinMaxes.m_dYMin = 0. ;
  m_MinMaxes.m_dYMax = 480. ;
  memset( &m_Labels.m_dXMin , 0 , sizeof( m_Labels.m_dXMin ) * 4 ) ;
  Resume();
}

void PlotGraph::ShutDown()
{
  CRenderGadget::ShutDown();
  delete m_pInput; m_pInput=NULL;
  delete m_pOutput;m_pOutput=NULL;
  if (m_wndOutput)
    m_wndOutput->DestroyWindow();
  delete m_wndOutput; m_wndOutput=NULL;
}

void PlotGraph::Attach(CWnd* pWnd)
{
  if ( !pWnd )
   Detach();
  HWND hOwnWnd = (m_pOwnWnd) ?
    (::IsWindow( m_pOwnWnd->GetSafeHwnd() ) ? m_pOwnWnd->GetSafeHwnd() : (HWND) 0)
    : (HWND) (-1) ;
  if ( !pWnd || !::IsWindow( pWnd->GetSafeHwnd() ) )
  {
    SEND_GADGET_ERR( "No Window for Attachment. hOwn=0x%08X " , hOwnWnd ) ;
    return ;
  }
  BOOL bLocked = m_Lock.LockAndProcMsgs( INFINITE , "FRender::Attach" ) ;
  VERIFY( bLocked );
  if ( m_pAttachedWnd && (m_pAttachedWnd != m_pOwnWnd) )
  {
    HWND hOldWnd = m_pAttachedWnd->Detach() ;
    m_pAttachedWnd->DestroyWindow() ;
    m_pAttachedWnd = NULL ;
  }
  HWND hWnd = pWnd->GetSafeHwnd() ;
  if ( !m_wndOutput )
  {
    m_wndOutput = new CPlotGraphView( m_Monitor );
    m_wndOutput->Create( pWnd );
    GetGadgetName( m_GadgetInfo );
    m_wndOutput->SetAutoFit( m_bFit );
    m_wndOutput->SetNSamples( m_iNSamples ) ;
    m_wndOutput->GetMinMaxes() = m_MinMaxes ;
    m_wndOutput->SetViewNet( m_iViewNet ) ;
    m_wndOutput->SetViewNames( m_bViewNames );
    m_MinMaxes = m_wndOutput->GetMinMaxes();
    m_wndOutput->SetLabels( m_Labels );
  }
  else
  {
    SetParent( m_wndOutput->m_hWnd , hWnd );
    CRect cr;
    pWnd->GetClientRect( &cr );
    DWORD dwSize = (cr.Height() << 16) | (cr.Width() & 0xffff);
    PostMessage( m_wndOutput->m_hWnd , WM_SIZE , 0 , dwSize );
  }
  m_iNAttachments++;
  if ( m_pOwnWnd == NULL )
    m_pOwnWnd = pWnd ;
  if ( hWnd == hOwnWnd )
    m_hExternalWnd = NULL ;
  m_pAttachedWnd = pWnd ;

  m_Lock.Unlock() ;

  TRACE( "\nAttached to hWnd 0x%08X, hOutWnd=0X%08X" ,
    hWnd , m_wndOutput->m_hWnd ) ;
  SEND_GADGET_INFO( "Attached to hWnd 0x%08X, hOutWnd=0X%08X", 
    hWnd, m_wndOutput->m_hWnd) ;
}

void PlotGraph::Detach()
{
  BOOL bLocked = m_Lock.LockAndProcMsgs(INFINITE , "PlotGraph::Detach");
  VERIFY(bLocked);
  HWND hWndBefore = NULL;
  bool bExiting = false;
  if (m_wndOutput)
  {
    hWndBefore = m_wndOutput->m_hWnd;
    if (::IsWindow(m_wndOutput->GetSafeHwnd()))
    {
      //m_wndOutput->SetCallback(NULL , NULL);
      m_wndOutput->DestroyWindow();
      bExiting = true;
    }
    delete m_wndOutput;
    m_wndOutput = NULL;
  }
  HWND hExtWnd = m_hExternalWnd;
  if (m_hExternalWnd)
  {
    m_pAttachedWnd->Detach();
    delete m_pAttachedWnd;
    m_pAttachedWnd = NULL;
    m_hExternalWnd = NULL;
  }
  TRACE("Detach %s window 0x%08X. Ext Wnd was 0x%08X" ,
    bExiting ? "real" : "Unused" , hWndBefore , hExtWnd);

  m_Lock.Unlock();
}

void PlotGraph::Render(const CDataFrame* pDataFrame)
{
  if ( !pDataFrame->IsContainer() )
  {
    const CTextFrame * pCommand = pDataFrame->GetTextFrame();
    if ( pCommand )
    {
      if ( _tcscmp( pDataFrame->GetLabel() , _T( "SetWndHandle" ) ) == 0 )
      {
        CWnd * pMainWnd = AfxGetApp()->GetMainWnd() ;
        FXString GadgetName ;
        if ( pMainWnd && GetGadgetName( GadgetName ) )
        {
          FXSIZE iPos = GadgetName.ReverseFind( _T( '.' ) ) ;
          if ( iPos >= 0 )
            GadgetName = GadgetName.Mid( iPos + 1 ) ;
          FXString NewHandleAsString = pCommand->GetString() ;
          FXString NotificationToMainWnd ;
          NotificationToMainWnd.Format( "Gadget=%s;Properties=(hTargetWindow=%s;);" ,
            (LPCTSTR) GadgetName , (LPCTSTR) NewHandleAsString ) ;
          int iAllocLen = (int)NotificationToMainWnd.GetLength() + 5 ;
          TCHAR * pMsg = new TCHAR[ iAllocLen ] ;
          _tcscpy_s( pMsg , iAllocLen , (LPCTSTR) NotificationToMainWnd ) ;
          pMainWnd->PostMessage( VM_TVDB400_SET_GADGET_PROPERTIES , 0 , (LPARAM) pMsg ) ;
        }
        return ;
      }
      else if ( _tcsicmp( pDataFrame->GetLabel() , _T( "Clear" ) ) == 0 )
      {
        if ( m_wndOutput )
          m_wndOutput->RemoveAllGraphs() ;
      }
    }
  }
  if (::IsWindow(m_wndOutput->GetSafeHwnd()))
  {
    CDPoint dpnt;

    m_wndOutput->Render(pDataFrame);
    if ( m_pOutput )
    {
      if ( m_wndOutput->GetMouseXY(dpnt) )
      {
        FXString mes; mes.Format("x=%f; y=%f;",dpnt.x,dpnt.y);
        CTextFrame* tFrame=CTextFrame::Create(mes);
        tFrame->CopyAttributes(pDataFrame);
        if ((m_pOutput) && (!m_pOutput->Put(tFrame)))
          tFrame->RELEASE(tFrame);
      }
    }
  }
}

bool PlotGraph::PrintProperties(FXString& text)
{
  CRenderGadget::PrintProperties(text);
  FXPropertyKit pk;
  FXString min, max /*, Kx*/;
  if ( m_wndOutput )
    m_wndOutput->WriteMinMaxes( pk ) ;
  else
  {
    WriteArray( pk , "RangeX" , _T('f') , 2 , &m_MinMaxes.m_dXMin ) ;
    WriteArray( pk , "RangeY" , _T('f') , 2 , &m_MinMaxes.m_dXMin ) ;
  }
  WriteArray( pk , "Labels" , _T( 'f' ) , 4 , &m_Labels.m_dXMin ) ;
  pk.WriteInt("Fit", m_bFit);
  pk.WriteInt( _T("DataLength") , 
    m_wndOutput ? m_iNSamples = m_wndOutput->GetNSamples() : m_iNSamples ) ;
  pk.WriteInt( _T("ViewNames") , m_wndOutput ? 
    (m_bViewNames = (m_wndOutput->IsViewNames() != FALSE) ) : m_bViewNames ) ;
  pk.WriteInt( _T("ViewNet") , 
    m_wndOutput ? m_wndOutput->GetViewNet() : 0 ) ;
  pk.WriteInt( _T("ViewRanges") , m_wndOutput ? 
    (m_bViewRanges = (m_wndOutput->GetViewRanges() != FALSE ) ) : m_bViewRanges ) ;
  pk.WriteInt( _T("Clear") , 0 ) ;
  pk.WriteDouble( _T( "SamplePeriod_ms" ) , m_dSamplePeriod_ms ) ;
  FXString TargetAsString ;
  TargetAsString.Format( "0x%0X" , m_hExternalWnd ) ;
  if ( m_wndOutput && m_wndOutput->m_hWnd && !m_hExternalWnd )
  {
    CWnd * pWnd = m_wndOutput->GetParent();
    if ( pWnd )
    {
      HANDLE hWnd = pWnd->m_hWnd;
      FXString AddInfo;
      AddInfo.Format( " - 0x%0X" , hWnd );
      AddInfo += (m_hExternalWnd == NULL) ? "  Own" : "  Ext" ;
      TargetAsString += AddInfo;
    }
  }
  //pk.WriteString( _T( "hTargetWindow" ) , TargetAsString ) ;
  text += pk;
  return true;
}

bool PlotGraph::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  CRenderGadget::ScanProperties(text, Invalidate);
  FXPropertyKit pk(text);
  FXString tmpS;
  if ( pk.GetString( _T( "hTargetWindow" ) , tmpS ) )
  {
    // // if handle exists, it should be the only property in input string
    FXSIZE iMaxPropLenAsText = ((FXSIZE) strlen( _T( "hTargetWindow" ) ) + tmpS.GetLength() + 5) ;
    if ( pk.GetLength() < iMaxPropLenAsText ) // if handle exists, check for absence of another items
    {
      HWND hTarget = (HWND) ConvToBinary( tmpS );
      if ( hTarget != m_hExternalWnd )
      {
        Detach();

        if ( hTarget == NULL )
        {
          if ( m_pOwnWnd && m_pOwnWnd->GetSafeHwnd() )
            Attach( m_pOwnWnd );
        }
        else
        {
          CWnd * pAttachedWnd = new CWnd;
          double dNow = GetHRTickCount();
          if ( pAttachedWnd )
          {
            if ( pAttachedWnd->FromHandlePermanent( hTarget ) != NULL )
              SENDERR( "Window h=0x%x is already attached, ATTACHMENT ABORTED" , hTarget );
            else
            {
              if ( pAttachedWnd->Attach( hTarget ) )
              {
                Attach( pAttachedWnd );
                m_hExternalWnd = hTarget;
              }
              else
                SENDERR( "Can't attach to external window h=0x%x, ATTACHMENT FAILED" , hTarget );
            }
          }
        }
      }
    }
  }
  if ( !pk.GetBool( _T("Fit") , m_bFit ) )
  {
    BOOL bTmp ;
    if ( pk.GetInt( _T("Fit") , bTmp ) )
      m_bFit = ( bTmp != FALSE ) ;
  }; 
  pk.GetInt( _T("DataLength") , m_iNSamples ) ;
    BOOL bTmp ;
    if ( pk.GetInt( _T("ViewNames") , bTmp ) )
      m_bViewNames = ( bTmp != FALSE ) ;
  if ( pk.GetInt( _T("ViewNet") , m_iViewNet ) )
  {
    if ( m_wndOutput )
      m_wndOutput->SetViewNet( m_iViewNet ) ;
  }; 
  int iTmp ;
  if ( pk.GetInt( _T("ViewRanges") , iTmp ) )
  {
    if ( m_wndOutput )
      m_wndOutput->SetViewRanges( iTmp ) ;
  };
  int bClear ;
  if ( pk.GetInt( _T("Clear") , bClear ) && bClear )
  {
    if ( m_wndOutput )
    {
      m_wndOutput->RemoveAllGraphs() ;
      Invalidate = true ;
    }
  }
  double dLabelMinMaxes[ 4 ] ;
  int iNNumbers = GetArray( pk , "Labels" , _T( 'f' ) , 
    ARRSZ( dLabelMinMaxes ) , &dLabelMinMaxes ) ;
  if ( iNNumbers >= 2 )
  {
    m_Labels.m_dXMin = dLabelMinMaxes[ 0 ] ;
    m_Labels.m_dXMax = dLabelMinMaxes[ 1 ] ;
    if ( iNNumbers == 4 )
    {
      m_Labels.m_dYMin = dLabelMinMaxes[ 2 ] ;
      m_Labels.m_dYMax = dLabelMinMaxes[ 3 ] ;
    }
  }

  pk.GetDouble( _T( "SamplePeriod_ms" ) , m_dSamplePeriod_ms ) ;
  if ( m_dSamplePeriod_ms != 0. )
  {
    m_Labels.m_dXMin = 0. ;
    m_Labels.m_dXMax = m_dSamplePeriod_ms * m_iNSamples / 1000.;
  }
  CXYMinMaxes MinMaxes = m_MinMaxes ;
  if ( GetArray( pk , "RangeX" , _T( 'f' ) , 2 , &MinMaxes.m_dXMin ) == 2 )
  {
    m_MinMaxes.m_dXMin = MinMaxes.m_dXMin ;
    m_MinMaxes.m_dXMax = MinMaxes.m_dXMax ;
  }
  if ( GetArray( pk , "RangeY" , _T( 'f' ) , 2 , &MinMaxes.m_dYMin ) == 2 )
  {
    m_MinMaxes.m_dYMin = MinMaxes.m_dYMin ;
    m_MinMaxes.m_dYMax = MinMaxes.m_dYMax ;
  }
  if (m_wndOutput)
  {
    m_wndOutput->SetAutoFit( m_bFit );
    m_wndOutput->SetNSamples( m_iNSamples ) ;
    m_wndOutput->SetViewNames( m_bViewNames ) ;
    m_wndOutput->SetMinMaxes( pk ) ;  // set min maxes inside output window
    m_MinMaxes = m_wndOutput->GetMinMaxes() ;
//     m_wndOutput->RemoveAllGraphs() ;
    m_wndOutput->SetLabels( m_Labels ) ;
    m_wndOutput->SetSamplePeriod( m_dSamplePeriod_ms ) ;
  }
  else
  {
  }
  return true;
}

bool PlotGraph::ScanSettings(FXString& text)
{
  text.Format( _T(
    "template(EditBox(RangeX)"
    ",EditBox(RangeY),"
    "Spin(ViewNet,0,10),"
    "ComboBox(Fit(true(1),false(0))),"
    "ComboBox(ViewNames(true(1),false(0))),"
    "ComboBox(ViewRanges(true(1),false(0))),"
    //     "EditBox(Kx),Spin(DataLength,20,500))"), TRUE, FALSE);
    "Spin(DataLength,20,2000),"
    "ComboBox(Clear(No(0),Yes(1))),"
    "EditBox(Labels)," // minX, maxX, minY , maxY, number of markers is ViewNet
    "EditBox(SamplePeriod_ms)"
    ",EditBox(hTargetWindow)"
    ")"), TRUE, FALSE);
  return true;
}

bool PlotGraph::ReceiveEOS(const CDataFrame* pDataFrame)
{
  return true;
}
