// UserExampleGadget.h.h : Implementation of the UserExampleGadget class


#include "StdAfx.h"
#include "SyncRecieverGadget.h"
#include <string>
#include <exception>
#pragma comment(lib, "Winmm.lib")
USER_FILTER_RUNTIME_GADGET(SyncReceiverGadget, "SyncReceiver");	//	Mandatory


SyncReceiverGadget::SyncReceiverGadget()
{
	m_ListeningNow = false;
	m_threadHandle = NULL;
	init();
}

void CALLBACK TimerFunction(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	SyncReceiverGadget* params = (SyncReceiverGadget* )dwUser;
	params->StopListener();
}
CDataFrame* SyncReceiverGadget::DoProcessing(const CDataFrame* pDataFrame)
{
	const CTextFrame * ParamText = pDataFrame->GetTextFrame(DEFAULT_LABEL);
	FXString str = ParamText->GetString();
	if (m_ListeningNow)
	{
		m_lock.Lock();
		AnalyzeData(str);
		m_ListeningNow = false;
		m_lock.Unlock();
	}
	return NULL;
}

void SyncReceiverGadget::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
	CTextFrame * ParamText = pParamFrame->GetTextFrame(DEFAULT_LABEL);
	if (ParamText == NULL)
	{
		pParamFrame->Release(pParamFrame);		
		return;
	}
	
	FXString str = ParamText->GetString();
	if (str.Find("SET") > -1)
	{
		if (m_ListeningNow)
			SendData("Listener is busy", 1,false);
		else
		{
			if (ParseSetMsg(str))
			{
				m_lock.Lock();
				m_TimerId = timeSetEvent(m_timeToWait, 0, TimerFunction, (DWORD_PTR)this, TIME_ONESHOT);
				SendData(m_msgToSend, 0);
				if (m_TimerId != NULL)
					m_ListeningNow = true;
				else
				{
					SendData("Timer launch error", 1, false);
				}
				m_lock.Unlock();
			}
			else
			{
				SendData("Invalid Input", 1, false);
			}
		}
	}
  pParamFrame->Release( pParamFrame );
};
void SyncReceiverGadget::PropertiesRegistration()
{

};
void SyncReceiverGadget::ConnectorsRegistration()
{
	addInputConnector(text, "Input");
	addOutputConnector( text , "Data");
	addOutputConnector(text, "AckData");
	addDuplexConnector(transparent, text, "Setup");
};
void SyncReceiverGadget::StopListener()
{
	m_lock.Lock();	
	if (m_ListeningNow)
	{
		SendData("Time out", 1, false);
		m_ListeningNow = false;
	}
	m_lock.Unlock();
}
bool SyncReceiverGadget::ParseSetMsg(FXString msg)
{
	FXString s = msg.Trim("SET");
	FXSIZE iTok = 0;
	int iter = 0;

	FXString msgToSend;
	FXString suffix;
	FXString prefix;
	FXString length;
	FXString timeToWait;

	while (iTok != -1 && iter < 5)
	{
		FXString str = s.Tokenize(">", iTok);
		switch (iter)
		{
		case 0:
		{
			msgToSend = str.Trim("<");
			break;
		}
		case 1:
		{
			prefix = str.Trim("<");
			break;
		}
		case 2:
		{
			suffix = str.Trim("<");
			break;
		}
		case 3:
		{
			length = str.Trim("<");
			break;
		}
		case 4:
		{
			timeToWait = str.Trim("<");
			break;
		}
		default:
			break;
		}
		iter++;
	}
	if (msgToSend.GetLength() > 0 && suffix.GetLength() > 0 && prefix.GetLength() > 0 && length.GetLength() > 0 && timeToWait.GetLength() >0)
	{
		m_msgToSend = msgToSend;
		try
		{
			Split(suffix);
		   //m_suffix = suffix;
			m_prefix = char(atoi(prefix));
			m_length = atoi(length);
			m_timeToWait = atoi(timeToWait);
		}
		catch (std::exception& e)
		{
			FXString Msg ;
			Msg.Format("Invalid SET of Data: %s" , e.what() ) ;
			SendData((LPCTSTR)Msg, 1, false);
			return false;
		}
		return true;
	}
	else
	{
		SendData("Invalid SET data", 1, false);
		return false;
	}
}
bool SyncReceiverGadget::AnalyzeData(FXString str)
{
	bool success = false;
	int length = (int)str.GetLength();
	int prefixLoc = (int) str.Find(m_prefix);
	//int suffixLoc = str.Find(m_suffix);

	FXString debugStr;
	debugStr.Format("Recieved length = %d", (int) str.GetLength());
	SendData(debugStr, 1, false);
	
	debugStr.Format("Pattern length = %d", m_length);
	SendData(debugStr, 1, false);

	if (str.GetLength() > 0)
	{
		int c = str[0];
		debugStr.Format("[0] index char %d",c );
		SendData(debugStr, 1, false);
	}

	if (str.GetLength() > 1)
	{
		int c1 = str[str.GetLength() - 2];
		int c2 = str[str.GetLength() - 1];
		debugStr.Format("2 last chars %d - %d", c2,c1);
		SendData(debugStr, 1, false);
	}

	FXString out;
	if (length != m_length)
		out = ("Invalid length");
	else if (prefixLoc != 0)
		out += ("Invalid prefix location");
	else if (!VerifySuffix(str))
	{
		out += ("Invalid suffix location");
	}
	else
	{
		SendData("1:" + str, 1, false);
		FXString res = str.Trim(m_prefix);
		SendData("2:" + res, 1, false);
		for (int i = (int) m_asciiSufix.GetSize()-1; i >-1; i--)
		{
			int ch = m_asciiSufix.GetAt(i);
			res = res.Trim(ch);
			SendData("3:" + res, 1, false);
		}
		SendData("4:" + res, 1, false);
		out += res ;
		success = true;
	}
	SendData(out, 1, false);
	return success;
}
bool SyncReceiverGadget::VerifySuffix(FXString str)
{
	try
	{
		for (int indx = 0; indx < m_asciiSufix.GetSize(); indx++)
		{
			int ch1 = str[str.GetLength() - indx - 1];
			int ch2 = m_asciiSufix.GetAt(m_asciiSufix.GetSize() - indx - 1);
			if (ch1 != ch2)
			{
				return false;
			}
		}
		return true;
	}
	catch (std::exception& e)
	{
		FXString Msg ;
		Msg.Format("Invalid SUFFIX data: %s" , e.what() ) ;
		SendData((LPCTSTR)Msg, 1, false);
		return false;
	}

}
int SyncReceiverGadget::Split(FXString str)
{
	m_asciiSufix.RemoveAll();
	FXSIZE tok = 0;
	while (tok!=-1)
	{
		int i = atoi(str.Tokenize(",", tok));
		if (tok!=-1)
			m_asciiSufix.Add(i);
	}
	return (int) m_asciiSufix.GetSize();
}
void SyncReceiverGadget::SendData(FXString msg,int pinN,bool raw)
{
	FXString fx;
	if (!raw)
	{
		time_t rawtime;
		time(&rawtime);
		FXString time;
		time.Format("%d", rawtime);	
		fx.Format("%s",msg);
		fx = "<" + time + "><" + m_msgToSend + ">" + "<" + fx + ">";
	}
	else
	{
		fx = m_msgToSend;
	}
	CTextFrame * ViewText = CTextFrame::Create(fx);
	if (!GetOutputConnector(pinN)->Put(ViewText))
		ViewText->Release(ViewText);
}




