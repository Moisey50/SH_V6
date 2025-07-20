// Knob.cpp: implementation of the Knob class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CtrlGadgets.h"
#include "Knob.h"
#include <gadgets\TextFrame.h>
#include <gadgets\QuantityFrame.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_RUNTIME_GADGET_EX( Knob , CCtrlGadget , LINEAGE_CTRL , TVDB400_PLUGIN_NAME );

void __stdcall KnobCallback( UINT nID , int nCode , void* cbParam , void* pParam )
{
  ( ( Knob* ) cbParam )->OnCommand( nID , nCode );
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Knob::Knob()
{
  SetMonitor( SET_INPLACERENDERERMONITOR );
  m_ButtonName = "Press me!";
  m_pOutput = new COutputConnector( text * quantity * logical );
  m_Type = KOT_Text ;
  SetTicksIdle( 100 );
}

void Knob::ShutDown()
{
  Detach();
  CCtrlGadget::ShutDown();
  delete m_pOutput;
}

void Knob::Attach( CWnd* pWnd )
{
  Detach();

  CCtrlGadget::Create();
  CCtrlGadget::Attach( pWnd );


  CRect rc;
  pWnd->GetClientRect( rc );
  m_Proxy.Create( pWnd );
  m_Button.Create( m_ButtonName , WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON , rc , &m_Proxy , 13 );
  m_Proxy.Init( &m_Button , KnobCallback , this );
  m_Button.EnableWindow( FALSE );
  if ( ( m_pStatus ) && ( m_pStatus->GetStatus() == CExecutionStatus::RUN ) )
    OnStart();
}

void Knob::Detach()
{
  if ( m_Button.GetSafeHwnd() )
    m_Button.DestroyWindow();
  if ( m_Proxy.GetSafeHwnd() )
    m_Proxy.DestroyWindow();
}

void Knob::OnCommand( UINT nID , int nCode )
{
  if ( ( nID == 13 ) && ( nCode == BN_CLICKED ) 
    && ( m_pStatus ) && ( m_pStatus->GetStatus() == CExecutionStatus::RUN ) )
  {
    CDataFrame * pOutFrame = NULL ;
    switch ( m_Type )
    {
      case KOT_Text:
      {
        CTextFrame* pDataFrame = CTextFrame::Create();
        if ( m_Value.IsEmpty() )
          pDataFrame->GetString().Format( "%s(BN_CLICKED)" , m_ButtonName );
        else
        {
          pDataFrame->GetString().Format( "%s" , m_Value );
        }
        
        pDataFrame->ChangeId( 0 );
        pDataFrame->SetTime( GetGraphTime() * 1.e-3 );
        pOutFrame = pDataFrame ;
      }
      break ;
      case KOT_Int:
      {
        if ( !m_Value.IsEmpty() && 
          ( _istdigit( m_Value[ 0 ] ) || ( m_Value[ 0 ] == _T( '+' ) ) 
          || ( m_Value[ 0 ] == _T( '-' ) )  ) )
        {
          int iVal = atoi( m_Value ) ;
          CQuantityFrame * pQF = CQuantityFrame::Create( iVal ) ;
          pOutFrame = pQF ;
        }
      }
      break ;
      case KOT_Double:
      {
        if ( !m_Value.IsEmpty() &&
          ( _istdigit( m_Value[ 0 ] ) || ( m_Value[ 0 ] == _T( '+' ) )
          || ( m_Value[ 0 ] == _T( '-' ) ) || ( m_Value[ 0 ] == _T( '.' ) ) ) )
        {
          double dVal = atof( m_Value ) ;
          CQuantityFrame * pQF = CQuantityFrame::Create( dVal ) ;
          pOutFrame = pQF ;
        }
      }
      break ;
      case KOT_Bool:
      {
        if ( !m_Value.IsEmpty() )
        {
          CBooleanFrame * pBF = CBooleanFrame::Create( m_bVal == TRUE ) ;
          pOutFrame = pBF ;
        }
      }
      break ;
    }
    if ( pOutFrame )
    {
      pOutFrame->ChangeId( 0 );
      pOutFrame->SetTime( GetGraphTime() * 1.e-3 );
      if ( ( !m_pOutput ) || ( !m_pOutput->Put( pOutFrame ) ) )
        pOutFrame->Release();
    }
    
  }
}

bool Knob::ScanSettings( FXString& text )
{
  text = _T("template("
    "EditBox(ButtonName)"
    ",EditBox(Value)"
    ",ComboBox(Mode(Text(0),Int(1),Double(2),Boolean(3)))"
    ")") ;
  return true;
}


bool Knob::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  CCtrlGadget::ScanProperties( text , Invalidate );
  FXPropertyKit pk( text );
  pk.GetString( "ButtonName" , m_ButtonName );
  if ( pk.GetString( "Value" , m_Value ) )
  {
    m_Value = m_Value.Trim( _T( "\b\t" ) );
    FXSIZE iPos = -1 ;
    while ( ( iPos = m_Value.Find( _T( '\\' ) , ++iPos ) ) >= 0 )
    {
      if ( iPos < m_Value.GetLength() - 2 )
      {
        switch ( m_Value[ iPos + 1 ] )
        {
          case _T( 'r' ):
          case _T( 'R' ):
            m_Value.Insert( iPos , _T( '\r' ) ) ;
            m_Value.Delete( iPos + 1 , 2 ) ;
            break ;
          case _T( 'n' ):
          case _T( 'N' ):
            m_Value.Insert( iPos , _T( '\n' ) ) ;
            m_Value.Delete( iPos + 1 , 2 ) ;
            break ;
        }
      }
    };
  }
  pk.GetInt( "Mode" , (int&)m_Type ) ;
  if ( m_Type == KOT_Bool )
  {
    FXString Upper = m_Value.MakeUpper() ;
    if ( Upper == _T( "TRUE" ) )
      m_bVal = TRUE ;
    else if ( Upper == _T( "FALSE" ) )
      m_bVal = FALSE ;
    else if ( _istdigit( Upper[ 0 ] )
      || Upper[ 0 ] == _T( '+' ) || Upper[ 0 ] == _T( '-' ) )
    {
      m_bVal = atoi( Upper ) != 0 ;
    }
  }

  
  if ( m_Button.GetSafeHwnd() )
    m_Button.SetWindowText( m_ButtonName );
  return true;
}

bool Knob::PrintProperties( FXString& text )
{
  FXPropertyKit pk;
  CCtrlGadget::PrintProperties( text );
  pk.WriteString( "ButtonName" , m_ButtonName );
  pk.WriteString( "Value" , m_Value );
  pk.WriteInt( "Mode" , (int)m_Type ) ;
  text += pk;
  return true;
}

void Knob::OnStart()
{
  if ( m_Button.GetSafeHwnd() )
    m_Button.EnableWindow( TRUE );
  CCtrlGadget::OnStart();
}

void Knob::OnStop()
{
  if ( m_Button.GetSafeHwnd() )
    m_Button.EnableWindow( FALSE );
  CCtrlGadget::OnStop();
}
