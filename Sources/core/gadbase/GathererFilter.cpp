#include "StdAfx.h"
#include <gadgets\gadbase.h>

IMPLEMENT_RUNTIME_GADGET(CGathererFilter, CGadget, "Generic filter");
#undef THIS_MODULENAME 
#define THIS_MODULENAME _T("TvdBase.CGathererFilter")

CGathererFilter::CGathererFilter(void)
{
}

void CGathererFilter::ShutDown()
{
    FXAutolock al(m_InputsLock);
    CGadget::ShutDown();
}

int CGathererFilter::GetInputsCount()
{
    return (int)m_Inputs.GetCount();
}

int CGathererFilter::GetOutputsCount()
{
    return (m_pOutput)?1:0;
}

CInputConnector* CGathererFilter::GetInputConnector(int n)
{
    if (n < (int) m_Inputs.GetCount())
        return m_Inputs[n];
    return NULL;
}

COutputConnector* CGathererFilter::GetOutputConnector(int n)
{
    return (!n) ? m_pOutput : NULL;
}

int CGathererFilter::DoJob()
{
    return WR_CONTINUE;
}

CDataFrame* CGathererFilter::DoProcessing(const CDataFrame* pDataFrame, int pin)
{
    return NULL;
}

#undef THIS_MODULENAME 