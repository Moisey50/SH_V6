// PolyTransform.cpp: implementation of the PolyTransform class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileX.h"
#include "PolyTransform.h"
#include <gadgets\ContainerFrame.h>
#include <gadgets\RectFrame.h>
#include <gadgets\QuantityFrame.h>
#include <gadgets\TextFrame.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "PolyTransform"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_RUNTIME_GADGET_EX(PolyTransform, CControledFilter, LINEAGE_FILEX, TVDB400_PLUGIN_NAME)

PolyTransform::PolyTransform()
{
  m_SetupObject = new CPolySetupDialog( this , NULL ) ;
	memset(m_xPoly, 0, sizeof(m_xPoly));
	memset(m_yPoly, 0, sizeof(m_yPoly));
	m_xC = m_yC = 0;
	Resume();
}

void PolyTransform::ShutDown()
{
    CControledFilter::ShutDown();
}

bool PolyTransform::PrintProperties(FXString& Text)
{
	FXPropertyKit pk;
	pk.WriteDouble("x0", m_xPoly[0]);
	pk.WriteDouble("x1", m_xPoly[1]);
	pk.WriteDouble("x2", m_xPoly[2]);
	pk.WriteDouble("x3", m_xPoly[3]);
	pk.WriteDouble("x4", m_xPoly[4]);
	pk.WriteDouble("x5", m_xPoly[5]);
	pk.WriteDouble("y0", m_yPoly[0]);
	pk.WriteDouble("y1", m_yPoly[1]);
	pk.WriteDouble("y2", m_yPoly[2]);
	pk.WriteDouble("y3", m_yPoly[3]);
	pk.WriteDouble("y4", m_yPoly[4]);
	pk.WriteDouble("y5", m_yPoly[5]);
	pk.WriteDouble("xC", m_xC);
	pk.WriteDouble("yC", m_yC);
	Text = pk;
	return true;
}

bool PolyTransform::ScanProperties(LPCTSTR Text, bool& Invalidate)
{
	FXPropertyKit pk(Text);
	if (!pk.GetDouble("x0", m_xPoly[0]) ||
		!pk.GetDouble("x1", m_xPoly[1]) ||
		!pk.GetDouble("x2", m_xPoly[2]) ||
		!pk.GetDouble("x3", m_xPoly[3]) ||
		!pk.GetDouble("x4", m_xPoly[4]) ||
		!pk.GetDouble("x5", m_xPoly[5]) ||
		!pk.GetDouble("y0", m_yPoly[0]) ||
		!pk.GetDouble("y1", m_yPoly[1]) ||
		!pk.GetDouble("y2", m_yPoly[2]) ||
		!pk.GetDouble("y3", m_yPoly[3]) ||
		!pk.GetDouble("y4", m_yPoly[4]) ||
		!pk.GetDouble("y5", m_yPoly[5]) ||
		!pk.GetDouble("xC", m_xC) ||
		!pk.GetDouble("yC", m_yC))
		return FALSE;
	return true;
}

bool PolyTransform::ScanSettings(FXString& text)
{
	text="calldialog(true)";
	return true;
}


CDataFrame* PolyTransform::DoProcessing(
  const CDataFrame* pDataFrame, const CDataFrame* pParamFrame)
{
	TRACE("sizeof(CRectFrame) = %d\n", sizeof(CRectFrame));
	if (Tvdb400_IsEOS(pDataFrame))
	{
    CDataFrame* retFrame = (CDataFrame*)pDataFrame ;
		retFrame->AddRef();
		return retFrame ;
	}
	if (!pDataFrame)
		return NULL;
	CContainerFrame* retVal = CContainerFrame::Create();
	retVal->ChangeId(pDataFrame->GetId());
	retVal->SetTime(pDataFrame->GetTime());
	if (!Tvdb400_IsEOS(pDataFrame))
	{
		const CTextFrame* pTextFrame = (pParamFrame) ? pParamFrame->GetTextFrame(DEFAULT_LABEL) : NULL;
		if (pTextFrame)
		{
      bool Invalidate=false;
			FXString params = pTextFrame->GetString();
			ScanProperties(params, Invalidate);
		}
		const CRectFrame* rFrame = NULL;
		CDataFrame* refFrame = NULL;
		CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(rectangle);
		if (Iterator)
		{
			rFrame = (CRectFrame*)Iterator->Next(DEFAULT_LABEL);
			while (rFrame)
			{
				CRect rc = *rFrame;
				Transform(rc);
				CRectFrame* RectFrame = CRectFrame::Create(rc);
				RectFrame->ChangeId(retVal->GetId());
				RectFrame->SetTime(retVal->GetTime());
				if (!refFrame)
					retVal->AddFrame(RectFrame);
				else
					retVal->InsertAfter(refFrame, RectFrame);
				refFrame = RectFrame;
				rFrame = (CRectFrame*)Iterator->Next(DEFAULT_LABEL);
			}
			delete Iterator;
		}
		else if ((rFrame = pDataFrame->GetRectFrame(DEFAULT_LABEL)) != NULL)
		{
				CRect rc = *rFrame;
				//pDataFrame->Release(rFrame);
				Transform(rc);
				CRectFrame* RectFrame = CRectFrame::Create(rc);
				RectFrame->ChangeId(retVal->GetId());
				RectFrame->SetTime(retVal->GetTime());
				retVal->AddFrame(RectFrame);
		}
		CQuantityFrame* xFrame = NULL;
		Iterator = pDataFrame->CreateFramesIterator(quantity);
		refFrame = NULL;
		if (Iterator)
		{
			xFrame = (CQuantityFrame*)Iterator->Next(DEFAULT_LABEL);
			while (xFrame)
			{
				double x = *xFrame;
				CQuantityFrame* yFrame = (CQuantityFrame*)Iterator->Next(DEFAULT_LABEL);
				if (!yFrame)
					break;
				double y = *yFrame;
				Transform(x, y);
				CQuantityFrame* qxFrame = CQuantityFrame::Create(x);
				qxFrame->ChangeId(retVal->GetId());
				qxFrame->SetTime(retVal->GetTime());
				CQuantityFrame* qyFrame = CQuantityFrame::Create(y);
				qyFrame->ChangeId(retVal->GetId());
				qyFrame->SetTime(retVal->GetTime());
				if (!refFrame)
					retVal->AddFrame(qxFrame);
				else
					retVal->InsertAfter(refFrame, qxFrame);
				retVal->InsertAfter(qxFrame, qyFrame);
				refFrame = qyFrame;
				xFrame = (CQuantityFrame*)Iterator->Next(DEFAULT_LABEL);
			}
			delete Iterator;
		}
	}
//	pDataFrame->Release();
	return retVal;
}

void PolyTransform::Transform(LPRECT rc)
{
	double x, y;
	x = rc->left;
	y = rc->top;
	Transform(x, y);
	rc->left = (int)(x + .5);
	rc->top = (int)(y + .5);
	x = rc->right;
	y = rc->bottom;
	Transform(x, y);
	rc->right = (int)(x + .5);
	rc->bottom = (int)(y + .5);
}

void PolyTransform::Transform(double& x, double& y)
{
	double ax = m_xPoly[0];
	double bx = m_xPoly[1];
	double cx = m_xPoly[2];
	double dx = m_xPoly[3];
	double ex = m_xPoly[4];
	double fx = m_xPoly[5];
	double ay = m_yPoly[0];
	double by = m_yPoly[1];
	double cy = m_yPoly[2];
	double dy = m_yPoly[3];
	double ey = m_yPoly[4];
	double fy = m_yPoly[5];

	double X = ax * x * x + bx * x * y + cx * y * y + dx * x + ex * y + fx;
	double Y = ay * x * x + by * x * y + cy * y * y + dy * x + ey * y + fy;
	x = X;
	y = Y;
}
