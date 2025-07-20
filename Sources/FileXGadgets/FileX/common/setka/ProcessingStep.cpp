// ProcessingStep.cpp: implementation of the CProcessingStep class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProcessingStep.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProcessingStep::CProcessingStep(LPCTSTR strDescr):
m_bComplete(FALSE),
m_strDescr(strDescr),
m_bKeyStep(FALSE)
{
}

CProcessingStep::~CProcessingStep()
{
}
