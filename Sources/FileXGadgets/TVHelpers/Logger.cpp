#include "stdafx.h"
#include "Logger.h"
#include <time.h>

IMPLEMENT_RUNTIME_GADGET_EX(Logger, CRenderGadget, LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);

bool Logger::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  pk.WriteInt("LogFormat", m_iLogFormat);
  pk.WriteInt("TimeFormat", m_iTimeFormat);
  text = pk;
  return true;
}

bool Logger::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXPropertyKit pk(text);
  FXString fileName;
  pk.GetInt("LogFormat", m_iLogFormat);
  pk.GetInt("TimeFormat",m_iTimeFormat);
  return true;
}

bool Logger::ScanSettings(FXString& text)
{
  text = _T("template("
    "ComboBox(LogFormat(DataOnly(0),IdAndData(1),IdTimeAndData(2),TimeAndData(3))),"
    "ComboBox(TimeFormat(HrAndLess(0),Full(1),Diff(2))");
  return true;
}

Logger::Logger(): 
m_Terminal(NULL),
  m_rcb(NULL),
  m_wParam(NULL),
  m_iLogFormat(1),
  m_iTimeFormat(1),
  m_dLastLogTime( GetHRTickCount())
{
  m_pInput = new CInputConnector(transparent);
  Resume();
}

void Logger::Render(const CDataFrame* pDataFrame)
{
  if (!Tvdb400_IsEOS(pDataFrame)) //Do net render EOS
  {
    FXString outp; 
    FXString Time ;
    FXString tmpS;
    
    if ( m_iLogFormat >= 2 )
    {
      TCHAR buff[100] ;
      switch ( m_iTimeFormat )
      {
      case 0: 
      case 1:
        {
          time_t CurrTime = time( NULL ) ;
          tm * AsStruct = localtime( &CurrTime ) ;
          strftime( buff , sizeof(buff) , 
            (m_iTimeFormat == 0) ? "%X" : "%x %X", AsStruct ) ;
          Time = buff ;
        }
        break ;
      case 2:
        {
          double dCurrTime_ms = GetHRTickCount() ;
          double dDiff = dCurrTime_ms - m_dLastLogTime ;
          m_dLastLogTime = dCurrTime_ms ;
          Time.Format( "%8.3f" , dDiff ) ;
        }
        break ;
      }
    }
    if (Tvdb400_TypesCompatible(pDataFrame->GetDataType(), text))
    {
      CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(text);
      if (Iterator)
      {
        CTextFrame* TextFrame = (CTextFrame*)Iterator->Next(DEFAULT_LABEL);
        while (TextFrame)
        {
          FormString( _T("text") , TextFrame , tmpS , Time ,  TextFrame->GetString() ) ;
          outp += tmpS;
          TextFrame = (CTextFrame*)Iterator->Next(DEFAULT_LABEL);
        }
        delete Iterator;
      }
      else
      {
        const CTextFrame* TextFrame = pDataFrame->GetTextFrame(DEFAULT_LABEL);
        if (TextFrame)
        {
          FormString( _T("text") , TextFrame , tmpS , Time ,  TextFrame->GetString() ) ;
          outp += tmpS;
          outp += _T("\r\n") ;
        }
      }
    }
    else if (Tvdb400_TypesCompatible(pDataFrame->GetDataType(), quantity))
    {
      CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(quantity);
      if (Iterator)
      {
        const CQuantityFrame* QuantityFrame = (CQuantityFrame*)Iterator->Next(DEFAULT_LABEL);

        FXString values;
        while (QuantityFrame)
        {
          FormString( _T("quantity") , QuantityFrame , tmpS , Time , QuantityFrame->ToString() ) ;
          outp += tmpS;
          QuantityFrame = (CQuantityFrame*)Iterator->Next(DEFAULT_LABEL);
        }
        delete Iterator;
      }
      else
      {
        const CQuantityFrame* QuantityFrame = pDataFrame->GetQuantityFrame(DEFAULT_LABEL);
        if (QuantityFrame)
        {
          FormString( _T("quantity") , QuantityFrame , tmpS , Time , QuantityFrame->ToString() ) ;
          outp += tmpS;
        }
      }
    }
    else if (Tvdb400_TypesCompatible(pDataFrame->GetDataType(), logical))
    {
      const CBooleanFrame* BooleanFrame = pDataFrame->GetBooleanFrame(DEFAULT_LABEL);
      FormString( _T("boolean") , BooleanFrame , tmpS , Time , (BooleanFrame->operator bool() ? "true" : "false")) ;
      outp += tmpS;
    }
    else if (Tvdb400_TypesCompatible(pDataFrame->GetDataType(),rectangle))
    {
      const CRectFrame* RectFrame = pDataFrame->GetRectFrame(DEFAULT_LABEL);
      if (RectFrame)
      {
        CRect rc=(LPRECT)RectFrame;
        FXString AsString ;
        AsString.Format( _T("(%d,%d,%d,%d)") ,rc.left,rc.top,rc.right,rc.bottom ) ;
        FormString( _T("rectangle") , RectFrame , tmpS , Time , (LPCTSTR)AsString ) ;
        outp += tmpS;
      }
    }
    else
    {
      FormString( _T("other") , pDataFrame , tmpS , Time , Tvdb400_TypeToStr(pDataFrame->GetDataType()) ) ;
      outp += tmpS;
    }
    FXAutolock al(m_TermLock);
    if ((!m_Terminal) || (!m_Terminal->IsValid()))
      return;
    m_Terminal->AppendText( (LPCTSTR) outp);
  }
}

void Logger::Create()
{
  CRenderGadget::Create();
}

void Logger::ShutDown()
{
  CRenderGadget::ShutDown();
  delete m_pInput; m_pInput=NULL;
  delete m_Terminal; m_Terminal=NULL;
}

void Logger::Attach(CWnd* pWnd)
{
  Detach();
  m_Terminal = new CTermWindow();
  m_Terminal->Create(pWnd); 
  CRenderGadget::Create();
}

void Logger::Detach()
{
  VERIFY(m_TermLock.LockAndProcMsgs()); 
  if (m_Terminal)
  {
    if (m_Terminal->IsValid())
      m_Terminal->DestroyWindow();
    delete m_Terminal;
    m_Terminal=NULL;
  }
  m_TermLock.Unlock();
}
