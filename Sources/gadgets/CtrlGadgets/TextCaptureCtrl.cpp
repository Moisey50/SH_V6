// TextCaptureCtrl.cpp: implementation of the TextCaptureCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ctrlgadgets.h"
#include "TextCaptureCtrl.h"
#include <gadgets\TextFrame.h>
#include <gadgets\QuantityFrame.h>
#include <helpers\FXParser2.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_RUNTIME_GADGET_EX(TextCaptureCtrl, CCtrlGadget, LINEAGE_TEXT, TVDB400_PLUGIN_NAME);

void __stdcall TextCaptureCtrlCallback(UINT nID, int nCode, void* cbParam, void* pParam)
{
  ((TextCaptureCtrl*)cbParam)->OnCommand(nID, nCode);
}

void __stdcall TextCaptureOnInput(CDataFrame* lpData, void* lpParam, CConnector* lpInput)
{
  ((TextCaptureCtrl*)lpParam)->OnInput(lpData, lpInput);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TextCaptureCtrl::TextCaptureCtrl():
  m_FramesCounter(0),
  m_SendQuantity(FALSE),
  m_NoID(FALSE),
  m_Manual(FALSE),
  m_FrameRate(1),
  m_dFramePeriod_ms(1000.),
  m_bSendOnStart(0),
  m_StringSeparate(0),
  m_iStringCount(0),
  m_Loop(0),
  m_bChanged(false)
{
  SetMonitor(SET_INPLACERENDERERMONITOR);
  m_pOutput   = new COutputConnector(text*quantity);
  m_pInput    = new CInputConnector(nulltype,TextCaptureOnInput,this);
}

void TextCaptureCtrl::ShutDown()
{
  CCtrlGadget::ShutDown();
  delete m_pInput ;
  m_pInput = NULL ;
  delete m_pOutput; 
  m_pOutput=NULL;

}

void    TextCaptureCtrl::Attach(CWnd* pWnd)
{
  m_CtrlDlg.Create(IDD_TEXTCAPTURE_DLG,NULL);
  m_CtrlDlg.SetParent(pWnd);
  m_CtrlDlg.Init(TextCaptureCtrlCallback,this);
  m_CtrlDlg.GetDlgItem(IDC_EDIT_CTRL)->SetWindowText(m_Message);
  m_CtrlDlg.GetDlgItem(IDC_SEND)->SetWindowText((m_Manual)?"Send":"Apply");
}

void    TextCaptureCtrl::Detach()
{
  if (m_CtrlDlg.m_hWnd)
    m_CtrlDlg.DestroyWindow();
}

CDataFrame* TextCaptureCtrl::GetNextFrame(double* StartTime)
{
  CDataFrame* retV;
  FXAutolock al( m_TextProcessLock ) ;
  if ( m_Manual 
    || (m_Strings.GetCount() && !m_Loop && (m_iStringCount >= m_Strings.GetCount())) 
    || m_Message.IsEmpty() ) 
    return NULL;
  if ( m_StringSeparate && m_iStringCount >= m_Strings.GetCount()  && m_Loop )
    m_iStringCount = 0 ;
  FXString SendString = ( m_StringSeparate ) ? m_Strings[ m_iStringCount++] : m_Message ;
  if (m_SendQuantity)
  {
    int a=atoi(SendString);
    CQuantityFrame* pDataFrame = CQuantityFrame::Create(a);

    pDataFrame->ChangeId((m_NoID)?0:m_FramesCounter);
    pDataFrame->SetTime( GetGraphTime() * 1.e-3 );
    retV=pDataFrame;
  }
  else
  {
    FXString Decoded ;
    ConvViewStringToBin( SendString , Decoded ) ;
    CTextFrame* pDataFrame = CTextFrame::Create(Decoded);

    pDataFrame->ChangeId((m_NoID)?0:m_FramesCounter);
    pDataFrame->SetTime( GetGraphTime() * 1.e-3 );
    retV=pDataFrame;    
  }
  m_FramesCounter++;
  return retV;
}

void TextCaptureCtrl::OnCommand(UINT nID, int nCode)
{
  FXAutolock al( m_TextProcessLock ) ;
  if ((nID==IDC_SEND) && (nCode==BN_CLICKED) && (m_pStatus->GetStatus()==CExecutionStatus::RUN))
  {
    if (m_Manual)
    {
      CString Msg ;
      m_CtrlDlg.GetDlgItem(IDC_EDIT_CTRL)->GetWindowText( Msg );
      m_Message = (LPCTSTR)Msg ;
      if ( !m_Message.IsEmpty() )
      {
        if ( m_bChanged )
          CheckAndFormStrings() ;

        if ( m_StringSeparate && m_iStringCount >= m_Strings.GetCount()  && m_Loop )
          m_iStringCount = 0 ;
        if ( !m_StringSeparate || (m_iStringCount < m_Strings.GetCount()) )
        {
          FXString SendString = ( m_StringSeparate ) ? m_Strings[ m_iStringCount++] : m_Message ;
          if (m_SendQuantity)
          {
            int a=atoi(SendString);
            CQuantityFrame* pDataFrame = CQuantityFrame::Create(a);

            pDataFrame->ChangeId((m_NoID)?0:m_FramesCounter);
            pDataFrame->SetTime( GetGraphTime() * 1.e-3 );
            if ((!m_pOutput) || (!m_pOutput->Put(pDataFrame)))
              pDataFrame->Release();
          }
          else
          {
            FXString Decoded ;
            ConvViewStringToBin( SendString , Decoded ) ;
            CTextFrame* pDataFrame = CTextFrame::Create( Decoded );

            pDataFrame->ChangeId((m_NoID)?0:m_FramesCounter);
            pDataFrame->SetTime( GetGraphTime() * 1.e-3 );
            if ((!m_pOutput) || (!m_pOutput->Put(pDataFrame)))
              pDataFrame->Release();
          }
          m_FramesCounter++;
        }
      }
    }
    else // Apply cmd
    {
      CString Msg ;
      m_CtrlDlg.GetDlgItem(IDC_EDIT_CTRL)->GetWindowText( Msg );
      m_Message = (LPCTSTR)Msg ;
      CheckAndFormStrings() ;
    }
    m_bChanged = false ;
  }
  else if ((nID==IDC_EDIT_CTRL) && (nCode==EN_CHANGE))
  {
    if (m_Manual)
    {
      CString Msg ;
      m_CtrlDlg.GetDlgItem(IDC_EDIT_CTRL)->GetWindowText( Msg );
      m_Message = (LPCTSTR)Msg ;
    }
    m_bChanged = true ;
  }
  else if ( (nID==IDC_FROM_FILE) && (nCode==EN_CHANGE))
  {
    CString Msg ;
    m_CtrlDlg.GetDlgItem(IDC_EDIT_CTRL)->GetWindowText( Msg );
    m_Message = (LPCTSTR)Msg ;
    CheckAndFormStrings() ;
    m_bChanged = true ;
  }

}

void    TextCaptureCtrl::OnStart() 
{ 
  CCaptureGadget::OnStart() ; 
  if ( !m_CtrlDlg.m_FileName.IsEmpty() )
  {
    m_CtrlDlg.ReadFile( m_CtrlDlg.m_FileName) ;
  };

  if ( m_Manual && m_bSendOnStart )
  {
    ///OnInput( NULL , NULL ) ;
    ::PostMessage( m_CtrlDlg.m_hWnd , UM_SEND_DATA , 0 , 0 ) ;
  }
}


bool TextCaptureCtrl::ScanSettings(FXString& text)
{
  text.Format(
    "template(ComboBox(Quantity(False(%d),True(%d)))"
    ",ComboBox(NoSync(False(%d),True(%d)))"
    ",ComboBox(Manual(False(%d),True(%d)))"
    ",Spin(FrameRate,1,25)"
    ",EditBox(FramePeriod_ms)"
    ",ComboBox(SendOnStart(No(0),Yes(1)))"
    ",ComboBox(StringSeparate(No(0),Yes(1)))"
    ",ComboBox(Loop(No(0),Yes(1)))"
    ",EditBox(FileName)"
    ")",FALSE,TRUE,FALSE,TRUE,FALSE,TRUE);
  return true;
}

bool TextCaptureCtrl::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  CCtrlGadget::ScanProperties(text,Invalidate);
  FXPropertyKit pk(text);
  pk.GetString("Text",m_Message);
  pk.GetInt("Quantity",m_SendQuantity);    
  pk.GetInt("NoSync",m_NoID);   
  pk.GetInt("Manual",m_Manual);
  if ( pk.GetInt( "FrameRate" , m_FrameRate ) )
    m_dFramePeriod_ms = 1000. / m_FrameRate ;
  pk.GetDouble( "FramePeriod_ms" , m_dFramePeriod_ms );
  pk.GetInt("SendOnStart" , m_bSendOnStart ) ;
  pk.GetInt("StringSeparate" , m_StringSeparate ) ;
  pk.GetInt("Loop" , m_Loop ) ;
  if ( pk.GetString( "FileName" , m_CtrlDlg.m_FileName ) 
    && !m_CtrlDlg.m_FileName.IsEmpty() && m_CtrlDlg.m_hWnd )
  {
    m_CtrlDlg.ReadFile( m_CtrlDlg.m_FileName) ;
  };
  if ( m_dFramePeriod_ms >= 5.0 )
    SetTicksIdle( ROUND(m_dFramePeriod_ms) ) ;
  else
    SetTicksIdle(1000/m_FrameRate);
  if (IsCtrlReady())
  {
    m_CtrlDlg.GetDlgItem(IDC_SEND)->SetWindowText((m_Manual)?"Send":"Apply");
  }
  CheckAndFormStrings() ;
  return true;
}

bool TextCaptureCtrl::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  CCtrlGadget::PrintProperties(text);

  CString tmpS;
  pk.WriteString("Text",m_Message);
  pk.WriteInt("Quantity",m_SendQuantity);
  pk.WriteInt("NoSync",m_NoID);   
  pk.WriteInt("Manual",m_Manual);   
  pk.WriteInt( "FrameRate" , m_FrameRate );
  pk.WriteDouble( "FramePeriod_ms" , m_dFramePeriod_ms , "%.2f" );
  pk.WriteInt("SendOnStart" , m_bSendOnStart ) ;
  pk.WriteInt("StringSeparate" , m_StringSeparate ) ;
  pk.WriteInt("Loop" , m_Loop ) ;
  pk.WriteString( "FileName" , m_CtrlDlg.m_FileName ) ;
  text+=pk;
  return true;
}


int TextCaptureCtrl::CheckAndFormStrings(void)
{
  if ( m_StringSeparate )
  {
    m_Strings.RemoveAll() ;
    FXSIZE iPos = 0 ;
    FXString Token = m_Message.Tokenize( _T("\r\n") , iPos ) ;
    while ( iPos >= 0 )
    {
      if ( !Token.IsEmpty() )
        m_Strings.Add( Token ) ;
      Token = m_Message.Tokenize( _T("\r\n") , iPos ) ; 
    }
    m_iStringCount = 0 ;
  }
  m_bChanged = false ;

  return (int)m_Strings.GetCount() ;
}

void TextCaptureCtrl::OnInput(CDataFrame* lpData, CConnector* lpInput)
{
  if ( !lpData && lpInput )
    return ;
  if ( lpData && Tvdb400_IsEOS(lpData))
  {
    if (((!m_pOutput) || (!m_pOutput->Put(lpData))))
      lpData->RELEASE(lpData);
    return ;
  }
  FXString Label = (lpData)? lpData->GetLabel() : "" ;
  if ( !Label.IsEmpty() )
  {
    Label = Label.MakeLower() ;
    if ( Label == "reset" )
      m_iStringCount = 0 ;
  }
  CDataFrame* retV;
  FXAutolock al( m_TextProcessLock ) ;
  if ( (m_Strings.GetCount() && !m_Loop && (m_iStringCount >= m_Strings.GetCount())) 
     || m_Message.IsEmpty() ) 
  {
  }
  else
  {
    if ( m_StringSeparate && m_iStringCount >= m_Strings.GetCount()  && m_Loop )
      m_iStringCount = 0 ;
    FXString SendString = ( m_StringSeparate ) ? m_Strings[ m_iStringCount++] : m_Message ;
    int iId = lpData->GetId() ;
    if (m_SendQuantity)
    {
      int a=atoi(SendString);
      CQuantityFrame* pDataFrame = CQuantityFrame::Create(a);

      pDataFrame->ChangeId((m_NoID)?0: iId);
      pDataFrame->SetTime( GetGraphTime() * 1.e-3 );
      retV=pDataFrame;
    }
    else
    {
      CTextFrame* pDataFrame = CTextFrame::Create(SendString);

      pDataFrame->ChangeId((m_NoID)?0: iId);
      pDataFrame->SetTime( GetGraphTime() * 1.e-3 );
      retV=pDataFrame;    
    }
    if ( retV )
    {
      retV->ChangeId((m_NoID)?0: iId);
      retV->SetTime( GetGraphTime() * 1.e-3 );
      if ((!m_pOutput) || (!m_pOutput->Put(retV)))
        retV->Release();
    }
  }
  if ( lpData )
    lpData->Release() ;
}
