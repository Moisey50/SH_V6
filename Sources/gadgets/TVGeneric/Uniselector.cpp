// Uniselector.cpp: implementation of the Uniselector class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TVGeneric.h"
#include "Uniselector.h"
#include <gadgets\QuantityFrame.h>

IMPLEMENT_RUNTIME_GADGET_EX(Uniselector, CFilterGadget, LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Uniselector::Uniselector():
        m_OutputCnt(0)
{
	m_pInput = new CInputConnector(quantity);
    Resume();
}

void Uniselector::ShutDown()
{
    CFilterGadget::ShutDown();
    if (m_pInput)
        delete m_pInput;
	m_pInput = NULL;
    DeleteOutputs();
}

void Uniselector::InitOutputs()
{
	m_Lock.Lock();
	while ((m_Outputs.GetSize() > m_OutputCnt) && (m_Outputs.GetSize() > 0))
	{
		int n = (int)m_Outputs.GetSize() - 1;
		COutputConnector* pOutput = (COutputConnector*)m_Outputs.GetAt(n);
		ASSERT(pOutput);
		pOutput->Disconnect();
		delete pOutput;
		m_Outputs.RemoveAt(n);
	}
	while (m_Outputs.GetSize() < m_OutputCnt)
		m_Outputs.Add(new COutputConnector(logical));
	m_OutputCnt = (int)m_Outputs.GetSize();
	m_Lock.Unlock();
}

void Uniselector::DeleteOutputs()
{
    m_Lock.Lock();
    while (m_Outputs.GetSize())
    {
		COutputConnector* pOutput = (COutputConnector*)m_Outputs.GetAt(0);
		m_Outputs.RemoveAt(0);
		delete pOutput ;
    }
    m_Lock.Unlock();
}

int Uniselector::GetInputsCount() 
{ 
    return (m_pInput)?1:0;
}

CInputConnector*  Uniselector::GetInputConnector(int n) 
{ 
	return (n) ? NULL : m_pInput;
}

int Uniselector::GetOutputsCount() 
{ 
	m_Lock.Lock();
	int count = (int)m_Outputs.GetSize();
	m_Lock.Unlock();
	return count;
}

COutputConnector* Uniselector::GetOutputConnector(int n) 
{ 
	COutputConnector* pOutput = NULL;
	m_Lock.Lock();
	if ((n >= 0) && (n < m_Outputs.GetSize()))
		pOutput = (COutputConnector*)m_Outputs.GetAt(n);
	m_Lock.Unlock();
	return pOutput;
}

bool Uniselector::PrintProperties(FXString& text)
{
    CFilterGadget::PrintProperties(text);
    FXPropertyKit pc;
    pc.WriteInt("OutputPins", m_OutputCnt);
    text+=pc;
    return true;

}

bool Uniselector::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    CString tmpS;
    CFilterGadget::ScanProperties(text, Invalidate);
    FXPropertyKit pc(text);
    m_OutputCnt=1;
    pc.GetInt( "OutputPins", m_OutputCnt);  
    InitOutputs();
    Status().WriteBool(STATUS_REDRAW, true);
    return true;
}

bool Uniselector::ScanSettings(FXString& text)
{
    text="template(Spin(OutputPins,1,50))";
    return true;
}

CDataFrame* Uniselector::DoProcessing(const CDataFrame* pDataFrame)
{
    if (pDataFrame->GetQuantityFrame(DEFAULT_LABEL))
	{
		DWORD cnt=0;
		m_Lock.Lock();
        DWORD nFrameCntr = (DWORD)(pDataFrame->GetQuantityFrame(DEFAULT_LABEL)->operator int());
		while (cnt<(DWORD)m_Outputs.GetSize())
		{
			if ((nFrameCntr%m_Outputs.GetSize())==cnt)
			{
				CBooleanFrame* FrameTrue  = CBooleanFrame::Create(true);
                FrameTrue->CopyAttributes(pDataFrame);
				if (!((COutputConnector*)m_Outputs.GetAt(cnt))->Put(FrameTrue))
					FrameTrue->RELEASE(FrameTrue);
			}
			else
			{
				CBooleanFrame* FrameFalse = CBooleanFrame::Create(false);
                FrameFalse->CopyAttributes(pDataFrame);
				if (!((COutputConnector*)m_Outputs.GetAt(cnt))->Put(FrameFalse))
					FrameFalse->RELEASE(FrameFalse);
			}
			cnt++;
		}
		m_Lock.Unlock();
	}
    return NULL;
}
