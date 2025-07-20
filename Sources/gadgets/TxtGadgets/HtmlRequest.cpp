#include "stdafx.h"
#include "HtmlRequest.h"
#include "TxtGadgets.h"
#include <gadgets\textframe.h>

IMPLEMENT_RUNTIME_GADGET_EX(HtmlRequest, CCaptureGadget, LINEAGE_TEXT, TVDB400_PLUGIN_NAME);

HtmlRequest::HtmlRequest():
m_pInput(NULL),
m_bCmdPin(FALSE),
m_InetConnect(NULL),
m_bRun(FALSE),
m_FrameRate(1.0),
m_frameID(0)
{
	m_pOutput = new COutputConnector(text);
    SetTicksIdle((DWORD)(1000 / m_FrameRate+0.5));
}

void HtmlRequest::ShutDown()
{
	OnStop();
	m_InetSession.Close();
	if (m_pInput)
		delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
	CCaptureGadget::ShutDown();
}

void HtmlRequest::OnStart()
{
    m_frameID=0;
	m_bRun = TRUE;
	if (m_pInput)
		return;
	try
	{
		if (!m_InetConnect)
		{
			m_InetConnect = m_InetSession.GetHttpConnection(m_URL);
		}
	}
	catch (CInternetException* pEx)
	{
		TCHAR pszError[1024];
		pEx->GetErrorMessage(pszError, 1024);
		_tprintf_s("%1023s", pszError);
		FXString strMsg = pszError;
		CTextFrame* msg = CTextFrame::Create(strMsg);
		if (!m_pOutput->Put(msg))
			msg->Release(msg);
		pEx->Delete();
	}
}

void HtmlRequest::OnStop()
{
	m_bRun = FALSE;
	SendData(NULL, true);
	if (m_InetConnect)
		delete m_InetConnect;
	m_InetConnect = NULL;
}

void HtmlRequest::OnInput(CDataFrame* pFrame)
{
	if (!Tvdb400_IsEOS(pFrame))
	{
		CTextFrame* Params = pFrame->GetTextFrame();
		if (Params)
		{
			FXString URL = Params->GetString(), request;
			if (URL.Find("http:") == 0)
				URL = URL.Mid(7);
			FXSIZE pos = URL.Find('/');
			if (pos > 0)
			{
				request = URL.Mid(pos);
				URL = URL.Left(pos);
			}
			if (!URL.IsEmpty() && URL != m_URL)
			{
				OnStop();
				m_URL = URL;
			}
			m_Request = request;
		}
		SendData(pFrame);
	}
	pFrame->Release(pFrame);
}

int HtmlRequest::DoJob()
{
	ASSERT(m_pStatus);
	switch (m_pStatus->GetStatus())
	{
	case CExecutionStatus::STOP:
		if (m_bRun)
			OnStop();
		return WR_CONTINUE;
	case CExecutionStatus::PAUSE:
        {
            HANDLE pEvents[] = { m_evExit, m_pStatus->GetStartHandle(),m_pStatus->GetStpFwdHandle()};
            DWORD cEvents = sizeof(pEvents) / sizeof(HANDLE);
            DWORD retVal=::WaitForMultipleObjects(cEvents, pEvents, FALSE, INFINITE);
		    if ((retVal==2) && (!m_pInput))
			SendData();
		return WR_CONTINUE;
        }
	case CExecutionStatus::RUN:
		if (!m_bRun)
			OnStart();
		if (!m_pInput)
		  SendData();		
		return WR_CONTINUE;
	case CExecutionStatus::EXIT:
		return WR_EXIT;
	default:
		ASSERT(FALSE);
		return WR_CONTINUE;
	}
}

void HtmlRequest::SendData(CDataFrame* pFrame, bool bEOS)
{
	CTextFrame* msg = NULL;
	if (bEOS)
	{
		msg = CTextFrame::Create();
		Tvdb400_SetEOS(msg);
	}
	else
	{
		try
		{
			if (!m_InetConnect)
				m_InetConnect = m_InetSession.GetHttpConnection(m_URL);
			CHttpFile* File = m_InetConnect->OpenRequest(CHttpConnection::HTTP_VERB_GET, m_Request);
			File->SendRequest();
			FXString response;
			CString line;
			while (File->ReadString(line))
				response += (LPCTSTR)line;
			delete File;
			FXString str = response;
			msg = CTextFrame::Create(str);
		}
		catch (CInternetException* pEx)
		{
			TCHAR pszError[1024];
			pEx->GetErrorMessage(pszError, 1024);
			_tprintf_s("%1023s", pszError);
			FXString strMsg = pszError;
			msg = CTextFrame::Create(strMsg);
			pEx->Delete();
		}
	}
	if (msg)
    {
        if (pFrame)
            msg->CopyAttributes(pFrame);
        else
        {
            msg->ChangeId(m_frameID);
            msg->SetTime(GetGraphTime() * 1.e-3 );
            msg->SetLabel(m_URL);
            m_frameID++;
        }
        if ((!m_pOutput) || (!m_pOutput->Put(msg)))
		msg->Release(msg);
    }
}

bool HtmlRequest::ScanSettings(FXString& text)
{
    if (m_bCmdPin)
	text.Format("template(EditBox(URL),EditBox(request),ComboBox(CmdPin(False(%d),True(%d))))", FALSE, TRUE);
    else
        text.Format("template(EditBox(URL),EditBox(request),ComboBox(CmdPin(False(%d),True(%d))),EditBox(FrameRate))", FALSE, TRUE);
	return true;
}

bool HtmlRequest::ScanProperties(LPCTSTR text, bool& Invalidate) 
{
	FXPropertyKit pk(text);
	pk.GetString("URL", m_URL);
	pk.GetString("request", m_Request);
	BOOL bCmdPin = m_bCmdPin;
	pk.GetInt("CmdPin", bCmdPin);
	if (bCmdPin != m_bCmdPin)
	{
		m_bCmdPin = bCmdPin;
		if (m_bCmdPin)
		{
			ASSERT(!m_pInput);
			m_pInput = new CInputConnector(transparent, fnInput, this);
		}
		else
		{
			ASSERT(m_pInput);
			delete m_pInput;
			m_pInput = NULL;
		}
        Invalidate=true;
        Status().WriteBool(STATUS_REDRAW, true);
	}
    if (!m_bCmdPin)
    {
        FXString tmpS; 
        double tmpD=m_FrameRate;
        if (pk.GetString("FrameRate",tmpS))
            tmpD=atof(tmpS);
        if (tmpD<0.1) tmpD=0.1;
        if (tmpD>10)  tmpD=10;
        if (tmpD!=m_FrameRate)
        {
            m_FrameRate=tmpD;
            SetTicksIdle((DWORD)(1000 / m_FrameRate+0.5));
        }
	}
	return true;
}

bool HtmlRequest::PrintProperties(FXString& text)
{
	FXPropertyKit pk(text);
	pk.WriteString("URL", m_URL);
	pk.WriteString("request", m_Request);
	pk.WriteInt("CmdPin", m_bCmdPin);
    FXString tmpS; tmpS.Format("%.1f",m_FrameRate);
    pk.WriteString("FrameRate", tmpS);
	text = pk;
	return true;
}
