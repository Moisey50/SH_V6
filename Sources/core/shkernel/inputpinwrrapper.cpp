#include "stdafx.h"
#include "InputPinWrrapper.h"

IMPLEMENT_RUNTIME_GADGET(CInputPinWrrapper, CGadget, LINEAGE_DEBUG);

CInputPinWrrapper::CInputPinWrrapper(void)
{
	m_pOutput=new COutputConnector(transparent);
}

void CInputPinWrrapper::ShutDown()
{
    delete m_pOutput;
    m_pOutput = NULL;
	CGadget::ShutDown();
}

BOOL CInputPinWrrapper::Send(CDataFrame* pDataFrame)
{
	return m_pOutput->Put(pDataFrame);
}