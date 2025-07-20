// DIBRender.cpp: implementation of the CDIBRender class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DIBRender3D.h"
#include <gadgets\ContainerFrame.h>
#include <gadgets\RectFrame.h>
#include <gadgets\VideoFrame.h>
#include <imageproc\draw_over.h>


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDIBRender3D::CDIBRender3D(LPCTSTR name):
	CDIBView3D(name)
{
	m_Rect.SetRectEmpty();
    m_RedPen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	m_PurplePen.CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
}

CDIBRender3D::~CDIBRender3D()
{
	m_RedPen.DeleteObject();
}

BOOL CDIBRender3D::Create(CWnd* pParentWnd, DWORD dwAddStyle)
{
	BOOL res=CDIBView3D::Create(pParentWnd, dwAddStyle);
	return res;
}

bool CDIBRender3D::Draw(HDC hdc,RECT& rc)
{
	bool res=CDIBView3D::Draw(hdc,rc);
	return res;
}

void CDIBRender3D::Render(const CDataFrame* pDataFrame)
{
	if ((pDataFrame) && (!Tvdb400_IsEOS(pDataFrame)))
	{
		CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(vframe);
		if (Iterator)
		{
			const CRectFrame* rectFrame = pDataFrame->GetRectFrame(DEFAULT_LABEL);
			const CVideoFrame* cutFrame = (CVideoFrame*)Iterator->Next(DEFAULT_LABEL);
			const CVideoFrame* bgFrame = (CVideoFrame*)Iterator->Next(DEFAULT_LABEL);
			if (rectFrame && cutFrame && bgFrame)
			{
				m_Rect = *rectFrame;
				CVideoFrame* Frame = CVideoFrame::Create(_draw_over(bgFrame, cutFrame, m_Rect.left, m_Rect.top));
				if ((Frame) && (Frame->lpBMIH))
					LoadFrame(Frame);
				else
					LoadFrame(NULL);
				Frame->Release(Frame);
			}
			else if (cutFrame && cutFrame->lpBMIH)
			{
                if (rectFrame)
                     m_Rect = *rectFrame;
                else
                    m_Rect.SetRectEmpty();
				LoadFrame(cutFrame);
			}
			else
			{
				m_Rect.SetRectEmpty();
				LoadFrame(NULL);
			}
			delete Iterator;
		}
		else
		{
			m_Rect.SetRectEmpty();
			const CVideoFrame* Frame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
			if ((Frame) && (Frame->lpBMIH))
				LoadFrame(Frame);
			else
				LoadFrame(NULL);
		}
	}
	else
		LoadFrame(NULL);
}
