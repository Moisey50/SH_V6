#include "StdAfx.h"
#include "QuantityToText.h"
#include <gadgets\quantityframe.h>
#include <gadgets\textframe.h>

IMPLEMENT_RUNTIME_GADGET_EX(QuantityToText, CFilterGadget, "Helpers", TVDB400_PLUGIN_NAME);

#define PASSTHROUGH_NULLFRAME(rfr, fr)			\
{												\
	if (!(rfr) )	                            \
    {	                                        \
		return NULL;		                    \
    }                                           \
}


QuantityToText::QuantityToText(void)
{
	m_pInput = new CInputConnector(quantity);
	m_pOutput = new COutputConnector(text);
	Resume();
}

void QuantityToText::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDataFrame* QuantityToText::DoProcessing(const CDataFrame* pDataFrame)
{
	const CQuantityFrame* quantFrame = pDataFrame->GetQuantityFrame(DEFAULT_LABEL);
	PASSTHROUGH_NULLFRAME(quantFrame, pDataFrame);
    FXString tmpS=quantFrame ->ToString();
    CTextFrame* retVal= CTextFrame::Create(tmpS);
    retVal->CopyAttributes(pDataFrame);
    return retVal;
}
