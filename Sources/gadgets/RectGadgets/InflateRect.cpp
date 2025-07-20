#include "StdAfx.h"
#include "InflateRect.h"
#include <gadgets\RectFrame.h>

IMPLEMENT_RUNTIME_GADGET_EX(InflateRect, CFilterGadget, LINEAGE_RECTANGLES, TVDB400_PLUGIN_NAME)

#define PASSTHROUGH_NULLFRAME(rfr, fr)			\
{												\
	if (!(rfr) || ((rfr)->IsNullFrame()))	    \
    {	                                        \
		return NULL;		                    \
    }                                           \
}


InflateRect::InflateRect(void):
			m_InflateX(5),
			m_InflateY(5)
{
	m_pInput = new CInputConnector(rectangle);
	m_pOutput = new COutputConnector(rectangle);
	Resume();
}

void InflateRect::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDataFrame* InflateRect::DoProcessing(const CDataFrame* pDataFrame)
{
	RECT rc;
	const CRectFrame* rectFrame = pDataFrame->GetRectFrame(DEFAULT_LABEL);
	PASSTHROUGH_NULLFRAME(rectFrame, pDataFrame);
	memcpy(&rc, LPRECT(rectFrame), sizeof(RECT));
	if (rc.left>m_InflateX) rc.left-=m_InflateX; else rc.left=0;
	if (rc.top>m_InflateY) rc.top-=m_InflateY; else rc.top=0;
	rc.right+=m_InflateX; 
	rc.bottom+=m_InflateY;
    CRectFrame* retVal=CRectFrame::Create(&rc);
    retVal->CopyAttributes( rectFrame );
	return retVal;
}

bool InflateRect::PrintProperties(FXString& text)
{
	FXPropertyKit pk;
	pk.WriteInt("X", m_InflateX);
	pk.WriteInt("Y", m_InflateY);
	text = pk;
	return true;
}

bool InflateRect::ScanProperties(LPCTSTR text, bool& Invalidate)
{
	FXPropertyKit pk(text);
	pk.GetInt("X", m_InflateX);
	pk.GetInt("Y", m_InflateY);
	return true;
}

bool InflateRect::ScanSettings(FXString& text)
{
    text="template(Spin(X,-50,50),  Spin(Y,-50,50))";
	return true;
}
