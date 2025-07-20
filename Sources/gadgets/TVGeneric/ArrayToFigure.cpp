#include "stdafx.h"
#include "ArrayToFigure.h"
#include <gadgets\arrayframe.h>
#include <gadgets\quantityframe.h>
#include <gadgets\figureframe.h>

IMPLEMENT_RUNTIME_GADGET_EX(ArrayToFigure, CFilterGadget, "Data.conversion", TVDB400_PLUGIN_NAME);

ArrayToFigure::ArrayToFigure()
{
	m_pInput = new CInputConnector(quantity);
	m_pOutput = new COutputConnector(figure);
	Resume();
}

void ArrayToFigure::ShutDown()
{
	CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

bool ArrayToFigure::PrintProperties(FXString& text)
{
	return true;
}

bool ArrayToFigure::ScanProperties(LPCTSTR text, bool& Invalidate)
{
	return true;
}

CDataFrame* ArrayToFigure::DoProcessing(const CDataFrame* pDataFrame)
{
	const CArrayFrame* pArrayFrame = pDataFrame->GetArrayFrame();
	if (!pArrayFrame)
		return NULL;
	CFigureFrame* pFigureFrame = CFigureFrame::Create();
	pFigureFrame->CopyAttributes(pArrayFrame);
	DPOINT dp;
	void* pData = pArrayFrame->GetData();
	int count = pArrayFrame->GetCount();
	UINT szElement = pArrayFrame->GetElementSize();
	if (szElement == sizeof(double))
	{
		double* src = (double*)pData;
		double* end = src + count;
		dp.x = 0;
		while (src < end)
		{
			dp.y = *src;
			pFigureFrame->AddPoint(dp);
			src++;
			dp.x = dp.x + 1;
		}
	}
	else if (szElement == sizeof(GENERICQUANTITY))
	{
		LPGENERICQUANTITY src = (LPGENERICQUANTITY)pData;
		LPGENERICQUANTITY end = src + count;
		dp.x = 0;
		while (src < end)
		{
			CGenericQuantity gq(src);
			dp.y = (double)gq;
			pFigureFrame->AddPoint(dp);
			src++;
			dp.x = dp.x + 1;
		}
	}
    else if (szElement == sizeof(DPOINT))
    {
		DPOINT* src = (DPOINT*)pData;
		DPOINT* end = src + count;
		while (src < end)
		{
			pFigureFrame->AddPoint(*src);
			src++;
		}
    }
	return pFigureFrame;
}
