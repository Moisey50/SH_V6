// CommandGadget.cpp: implementation of the Command class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CMDGadgets.h"
#include "CommandGadget.h"
#include "ExecAndRedirect.h"
#include "Math\Intf_sup.h"

IMPLEMENT_RUNTIME_GADGET_EX( Command , CFilterGadget , LINEAGE_TEXT , TVDB400_PLUGIN_NAME );

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Command::Command() :
  m_Prefix( DEFAULT_PREFIX ) ,
  m_Commandline( DEFAULT_COMMAND_LINE ) ,
  m_Busy( false )
{
  m_pInput = new CInputConnector( transparent );
  m_pOutput = new COutputConnector( text );
  Resume();
}

void Command::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
}

CDataFrame* Command::DoProcessing( const CDataFrame* pDataFrame )
{
  CTextFrame* pResultFrame = NULL;
  if ( pDataFrame )
  {
    FXString Output;
    DWORD   ExitCode;
    FXString message;
    BOOL res;
    FXString CurrentCommand;

    pResultFrame = CTextFrame::Create();

    const CTextFrame * pText = pDataFrame->GetTextFrame() ;

    if ( m_Prefix.CompareNoCase( _T( "System" ) ) == 0 )
    {
      HWND hWnd = NULL ;
      if ( !m_WindowName.IsEmpty() )
      {
        hWnd = GetWindowHandle( (LPCSTR) m_WindowName ) ;
        if ( !hWnd )
        {
          int iRes = _tsystem( (LPCTSTR) m_Commandline ) ;
          if ( iRes == -1 )
          {
            pResultFrame->GetString().Format( "Error %d on 'system' execution" , errno ) ;
          }
        }
      }
      if ( !m_WindowName.IsEmpty() )
      {
        if ( !hWnd )
        {
          Sleep( m_iDelay ) ;
          HWND hWnd = GetWindowHandle( (LPCSTR) m_WindowName ) ;
        }
        if ( hWnd )
        {
          if ( !m_SubWindowName.IsEmpty() )
          {
            hWnd = GetWindowHandle( (LPCSTR) m_SubWindowName , hWnd ) ;
          }
          TCHAR Buf[ 1000 ] ;
          GetWindowText( hWnd , Buf , 999 ) ;
          pResultFrame->GetString().Format( "hWnd=0x%X (Window text is '%s')" ,
            hWnd , Buf ) ;
          pResultFrame->SetLabel( "hWnd" ) ;
        }
        else
        {
          pResultFrame->GetString().Format( "Can't find window %s %s %s" , 
            (LPCTSTR)m_WindowName ,
            m_SubWindowName.IsEmpty() ? "" : "subwindow" ,
            m_SubWindowName.IsEmpty() ? "" : (LPCTSTR)m_SubWindowName ) ;
        }
      }
    }
    else
    {
      if ( pText )
      {
        const CTextFrame* TextFrame = pDataFrame->GetTextFrame( DEFAULT_LABEL );
        CurrentCommand = m_Prefix + " " + m_Commandline + " " + TextFrame->GetString();
      }
      else
      {
        CurrentCommand = m_Prefix + " " + m_Commandline;
      }
      res = ExecAndRedirect( CurrentCommand , Output , &ExitCode );
      if ( !res )
      {
        message.Format( "Can't execute command: '%s, exit code %d\r\n' " , CurrentCommand , ExitCode );
        pResultFrame->SetString( message );
      }
      else
      {
        Output.TrimRight( "\r\n" );
        pResultFrame->SetString( Output );
      }
    }
    pResultFrame->CopyAttributes( pDataFrame );
    return pResultFrame;
  }
  else
    return CTextFrame::Create();
}

bool Command::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pk( text );
  if ( pk.GetString( "Prefix" , m_Prefix ) )
    m_Prefix = m_Prefix.Trim( " \t" ) ;
  pk.GetString( "Command" , m_Commandline );
  if ( pk.GetString( "WindowName" , m_WindowName ) )
    m_WindowName = m_WindowName.Trim( " \t" ).MakeUpper() ;
  if ( pk.GetString( "SubWindowName" , m_SubWindowName ) )
    m_SubWindowName = m_SubWindowName.Trim( " \t" ).MakeUpper() ;
  pk.GetInt( "Delay" , m_iDelay ) ;
  return true;
}

bool Command::PrintProperties( FXString& text )
{
  FXPropertyKit pk;
  pk.WriteString( "Prefix" , m_Prefix );
  pk.WriteString( "Command" , m_Commandline );
  pk.WriteString( "WindowName" , m_WindowName ) ;
  pk.WriteString( "SubWindowName" , m_WindowName ) ;
  pk.WriteInt( "Delay" , m_iDelay ) ;
  text = pk;
  return true;
}

bool Command::ScanSettings( FXString& text )
{
  text.Format( "template("
    "EditBox(Prefix)"
    ",EditBox(Command)"
    ",EditBox(WindowName)"
    ",Spin(Delay,1,100000)"
    ",EditBox(SubWindowName)"
  ")" );
  return true;
}

#define THIS_MODULENAME "PostMsg"

IMPLEMENT_RUNTIME_GADGET_EX( PostMsg , CFilterGadget , _T("Helpers") , TVDB400_PLUGIN_NAME );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PostMsg::PostMsg() :
  m_hWnd( NULL ) 
  , m_dMinimalInterval(0.)
  , m_dLastPostTime(0.)
  , m_LogMessages( FALSE )
{
  m_pInput = new CInputConnector( transparent );
  m_pOutput = new COutputConnector( text );
  Resume();
}

void PostMsg::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
}

CDataFrame* PostMsg::DoProcessing( const CDataFrame* pDataFrame )
{
  CTextFrame* pResultFrame = NULL;
  double dNow = GetHRTickCount() ;
  const CTextFrame * pText = pDataFrame->GetTextFrame() ;
  if ( pDataFrame 
    && dNow > m_dLastPostTime + m_dMinimalInterval )
  {
    if ( pText )
    {
      LPCTSTR pLabel = pText->GetLabel() ;
      if ( pLabel && *pLabel )
      {
        if ( _tcscmp( _T( "hWnd" ) , pLabel ) == 0 )
        {
          FXString Content = pText->GetString() ;
          FXSIZE iPos = Content.Find( _T( "hWnd=" ) ) ;
          if ( iPos >= 0 )
          {
            FXSIZE iNewHandle = 0 ;
            if ( ConvToBinary( (LPCTSTR) Content + iPos + 5 , iNewHandle ) )
            {
              m_hWnd = (HWND) (size_t) iNewHandle ;
              SENDINFO_1( "New hWnd=0x%X" , m_hWnd ) ;
            }
          }
          return NULL ;
        }
      }
      else if ( pText->GetString().Find( _T("PostControl:") ) == 0 ) 
      {
        if ( pText->GetString().Find( "Enable" ) > 0 )
        {
          m_bPostEnabled = true ;
        }
        else if ( pText->GetString().Find( "Disable" ) > 0 )
        {
          m_bPostEnabled = false ;
        }
        return NULL ;
      }
      if ( !m_hWnd || !m_bPostEnabled )
        return NULL ;

      int iX = 0 , iY = 0 ;
      bool bOK = false ;
      double dX , dY ;
      FXPropertyKit pk( pText->GetString() ) ;
      if ( pk.GetDouble( "x" , dX ) && pk.GetDouble( "y" , dY ) )
      {
        iX = ROUND( dX ) ;
        iY = ROUND( dY ) ;
        bOK = true ;
      }
      else if ( pk.GetInt( "x" , iX ) && pk.GetInt( "y" , iY ) )
        bOK = true ;

      if ( bOK )
      {
        LPARAM LPar = (LPARAM) ((iX & 0x0000ffff) | ((iY << 16) & 0xffff0000)) ;
        PostMessage( m_hWnd , WM_LBUTTONDOWN , MK_LBUTTON , LPar ) ;
        PostMessage( m_hWnd , WM_LBUTTONUP , 0 , LPar ) ;
        m_dLastPostTime = dNow ;
        if ( m_LogMessages )
        {
          SEND_GADGET_INFO( "WM_LBOTTONDOWN Pt(%d,%d) LPar=0x%X then WM_LBUTTONUP" , iX , iY , LPar ) ;
        }
      }
    }
    else
      SEND_GADGET_INFO( "Message thrown (not text)" ) ;
  }
  else if ( m_LogMessages )
  {
    if ( pText )
      SEND_GADGET_INFO( "Message %s thrown (Tmin=%d)" , (LPCTSTR)pText->GetString() , (int)m_dMinimalInterval ) ;
    else
      SEND_GADGET_INFO( "Message thrown (Tmin=%d)" ) ;
  }
  return NULL ;
}

bool PostMsg::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pk( text );
  pk.GetDouble( "MinInterval_ms" , m_dMinimalInterval ) ;
  pk.GetInt( "Log" , m_LogMessages ) ;
  return true;
}

bool PostMsg::PrintProperties( FXString& text )
{
  FXPropertyKit pk;
  pk.WriteDouble( "MinInterval_ms" , m_dMinimalInterval );
  pk.WriteInt( "Log" , m_LogMessages ) ;
  text = pk;
  return true;
}

bool PostMsg::ScanSettings( FXString& text )
{
  text.Format( "template("
    "EditBox(MinInterval_ms)"
    ",Spin(Log,0,1)"
    ")" );
  return true;
}

LPCTSTR PostMsg::GetGadgetInfo()
{
  GetGadgetName( m_GadgetInfo ) ;
  return (LPCTSTR) m_GadgetInfo ;
}

