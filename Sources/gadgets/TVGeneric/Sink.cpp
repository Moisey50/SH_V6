// Sink.cpp: implementation of the Sink class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tvgeneric.h"
#include "Sink.h"
#include <gadgets\ContainerFrame.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_RUNTIME_GADGET_EX(Sink, CGadget, LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Sink::Sink():
    m_InputCnt(1)
{
	m_pOutput = new COutputConnector(transparent);
}

void Sink::ShutDown()
{
    CGadget::ShutDown();
    if (m_pOutput)
    {
	    delete m_pOutput;
	    m_pOutput = NULL;
    }
	RemoveInputs();
}

int Sink::GetInputsCount()
{
    return (int)m_Inputs.GetSize();
}

int Sink::GetOutputsCount()
{
    return (m_pOutput!=NULL)?1:0;
}

CInputConnector*    Sink::GetInputConnector(int n)
{
	CInputConnector* pInput = NULL;
	FXAutolock lock(m_Lock);
	if ((n >= 0) && (n < m_Inputs.GetSize()))
		pInput = (CInputConnector*)m_Inputs.GetAt(n);
	return pInput;
}

COutputConnector*   Sink::GetOutputConnector(int n)
{
    return (n<GetOutputsCount()) ? m_pOutput:NULL;
}

void Sink::CreateInputs(int n, basicdatatype type)
{
	FXAutolock lock(m_Lock);
    while (m_Inputs.GetSize()<n)
		m_Inputs.Add(new CInputConnector(type, Sink_SendData, this));
	while (m_Inputs.GetSize()>n)
	{
		CInputConnector* pInput = (CInputConnector*)m_Inputs.GetAt(m_Inputs.GetSize()-1);
		if (pInput)
		{
			pInput->Disconnect();
			delete pInput;
		}
		m_Inputs.RemoveAt(m_Inputs.GetSize()-1);
	}
}

void Sink::RemoveInputs()
{
    FXAutolock lock(m_Lock);

    while (m_Inputs.GetSize())
	{
		CInputConnector* pInput = (CInputConnector*)m_Inputs.GetAt(0);
		m_Inputs.RemoveAt(0);
		delete pInput;
	}
}

void Sink::Input(CDataFrame* pDataFrame, CConnector* lpInput)
{
    FXAutolock lock(m_OutputLock);
    int i;
    ASSERT(m_Inputs.GetUpperBound()!=-1);
    for (i=0; i<=m_Inputs.GetUpperBound(); i++)
    {
        if (m_Inputs[i]==lpInput)
            break;
    }
    //CString pinName; 
    //pinName.Format("Pin=%d",i);
    //pDataFrame->SetLabel(pinName);
    
	CContainerFrame* pContainer = CContainerFrame::Create();
    pContainer->CopyAttributes(pDataFrame);
	pContainer->AddFrame(pDataFrame);
	CQuantityFrame* pQuanFrame = CQuantityFrame::Create(i); 
    pQuanFrame->CopyAttributes(pDataFrame);
	pQuanFrame->SetLabel("Sink");
	pContainer->AddFrame(pQuanFrame);
	if ((!m_pOutput) || (!m_pOutput->Put(pContainer)))
                pContainer->Release(pContainer);
}

bool Sink::PrintProperties(FXString& text)
{
    CGadget::PrintProperties(text);
    FXPropertyKit pc;
    pc.WriteInt("InputPins", m_InputCnt);
    text+=pc;
    return true;

}

bool Sink::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    CGadget::ScanProperties(text, Invalidate);
    FXPropertyKit pc(text);
    pc.GetInt("InputPins", m_InputCnt);  
    CreateInputs(m_InputCnt,transparent);
    Status().WriteBool(STATUS_REDRAW, true);
    return true;
}

bool Sink::ScanSettings(FXString& text)
{
    text="template(Spin(InputPins,1,999))";
    return true;
}
