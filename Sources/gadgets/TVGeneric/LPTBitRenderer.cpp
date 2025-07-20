// LPTBitRenderer.cpp: implementation of the LPTBitRender class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tvgeneric.h"
#include "LPTBitRenderer.h"
#include "LPTSetup.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "LPTBitRender"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_RUNTIME_GADGET_EX(LPTBitRender, CRenderGadget, LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);

static int LPTPRTS[]={0x378, 0x278, 0x8C00};


LPTBitRender::LPTBitRender():
    m_Port(0),
    m_directIO(NULL)
{
    m_directIO      = new CDirectIO;
	m_pInput        = new CInputConnector(nulltype);
    m_SetupObject   = new CLPTSetup(this, NULL);
    SetMonitor(SET_INPLACERENDERERMONITOR);
	Resume();
}

void LPTBitRender::ShutDown()
{
    CRenderGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
    if (m_directIO) delete m_directIO; m_directIO=NULL;
}

void LPTBitRender::Render(const CDataFrame* pDataFrame)
{
    if ((m_directIO) && (m_directIO->IsValid()))
    {
	    m_directIO->Out32(LPTPRTS[m_Port],0x01);
        Sleep(1);
        m_directIO->Out32(LPTPRTS[m_Port],0x00); 
        return;
    }
    SENDERR_0("Error: hwinterface is not available");
}

bool LPTBitRender::PrintProperties(FXString& text)
{
    CRenderGadget::PrintProperties(text);
    FXPropertyKit pc;
    pc.WriteInt("LPTPORT", m_Port);
    text+=pc;
    return true;
}

bool LPTBitRender::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    CRenderGadget::ScanProperties(text, Invalidate);
    FXPropertyKit pc(text);
    pc.GetInt("LPTPORT", m_Port);
    return true;
}

bool LPTBitRender::ScanSettings(FXString& text)
{
    text="calldialog(true)";
    return true;
}
