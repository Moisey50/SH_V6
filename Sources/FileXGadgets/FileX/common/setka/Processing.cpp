// Processing.cpp: implementation of the CProcessing class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Processing.h"
#include <imageproc\fstfilter.h>
#include <imageproc\simpleip.h>
//#include <files\imgfiles.h>
#include <specfuncs.h>
#include <imageproc\clusters\clusterop.h>
#include <lineops.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProcessing::CProcessing():
m_nCurStep(-1)
{
	m_ProcSteps.SetSize(0, 1);
	AddStep(new CProcStepLoader, TRUE);
	AddStep(new CProcStepFilter);
	AddStep(new CProcStepRefine, TRUE);
	AddStep(new CProcStepRectify);
	AddStep(new CProcStepVert);
	AddStep(new CProcStepVFlt);
	AddStep(new CProcStepRefineVLines);
	AddStep(new CProcStepApproxVLines, TRUE);
	AddStep(new CProcStepHorz);
	AddStep(new CProcStepHFlt);
	AddStep(new CProcStepRefineHLines);
	AddStep(new CProcStepApproxHLines, TRUE);
	AddStep(new CProcStepShowLines);
	AddStep(new CProcStepNodes);
	AddStep(new CProcStepRefNodes, TRUE);
	AddStep(new CProcStepMapper, TRUE);
}

CProcessing::~CProcessing()
{
	while (GetStepsCount())
		RemoveStep(0);
}

BOOL CProcessing::IsLastStep()
{
	return (m_nCurStep == GetStepsCount() - 1);
}

BOOL CProcessing::IsFirstStep()
{
	return !m_nCurStep;
}

BOOL CProcessing::IsKeyStep()
{
	return GetStep(m_nCurStep)->IsKeyStep();
}

BOOL CProcessing::IsProcessing()
{
	return (m_nCurStep != -1);
}

BOOL CProcessing::Next(CData* Data, void* params)
{
	ASSERT(!IsLastStep());
	CProcessingStep* Step = GetStep(++m_nCurStep);
	if (!Step->Process(Data, params))
		return FALSE;
	return TRUE;
}

BOOL CProcessing::Back()
{
	ASSERT(!IsFirstStep());
	m_nCurStep--;
	return TRUE;
}

void CProcessing::Reset()
{
	m_nCurStep = GetStepsCount();
	while (m_nCurStep--)
		GetStep(m_nCurStep)->SetComplete(FALSE);
}

void CProcessing::DrawResults(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view)
{
	if (IsProcessing())
        GetStep(m_nCurStep)->Draw(Data, hdc, rc, view);
}

FXString CProcessing::GetCurComment()
{
	FXString comment;
	if (IsProcessing())
		comment.Format("%d/%d: \"%s\"", m_nCurStep + 1, m_ProcSteps.GetSize(), GetStep(m_nCurStep)->GetDescription());
	else
		comment.Empty();
	return comment;
}


void CProcessing::AddStep(CProcessingStep* Step, BOOL bKeyStep)
{
	Step->SetKeyStep(bKeyStep);
	m_ProcSteps.Add(Step);
}

CProcessingStep* CProcessing::GetStep(int i)
{
	return m_ProcSteps.GetAt(i);
}

void CProcessing::RemoveStep(int i)
{
	CProcessingStep* Step = GetStep(i);
	if (Step)
		delete Step;
	m_ProcSteps.RemoveAt(i);
}

int CProcessing::GetStepsCount()
{
	return (int) m_ProcSteps.GetSize();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
// Processing steps

BOOL CProcStepLoader::Process(CData* Data, void* params)
{
	return Data->LoadImage((LPCTSTR)params);
}

void CProcStepLoader::Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view)
{
	if (!view->GetFramePntr() || memcmp(GetData(view->GetFramePntr()), GetData(Data->m_pSrcFrame), Data->m_pSrcFrame->lpBMIH->biSizeImage))
		view->LoadFrame(Data->m_pSrcFrame);
}

BOOL CProcStepFilter::Process(CData* Data, void* params)
{
	if (IsComplete())
		return TRUE;
    pTVFrame pTemp0=_hpass(Data->m_pSrcFrame, 800); _normalize(pTemp0);
	pTVFrame pTemp1= _lpass(pTemp0,500); _normalize(pTemp1);
    Data->m_pDstFrame =_hpass(pTemp1, 800);
    freeTVFrame(pTemp0);
    freeTVFrame(pTemp1);
	int hist[256];
	memset(hist, 0, sizeof(hist));
	LPBYTE Dst = GetData(Data->m_pDstFrame);
	int size = Data->m_pDstFrame->lpBMIH->biWidth * Data->m_pDstFrame->lpBMIH->biHeight;
	LPBYTE End = Dst + size;
	while (Dst < End)
	{
		hist[*Dst]++;
		Dst++;
	}
	int histMax = 0;
	for (int i = 1; i < 256; i++)
	{
		if (hist[histMax] < hist[i])
			histMax = i;
	}
	_simplebinarize(Data->m_pDstFrame, histMax-3);
    _clear_frames(Data->m_pDstFrame, 255, 15);
	SetComplete(Data->m_pDstFrame != NULL);
	return IsComplete();
}

void CProcStepFilter::Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view)
{
	if (!view->GetFramePntr() || memcmp(GetData(view->GetFramePntr()), GetData(Data->m_pDstFrame), Data->m_pDstFrame->lpBMIH->biSizeImage))
		view->LoadFrame(Data->m_pDstFrame);
}

BOOL CProcStepRefine::Process(CData* Data, void* params)
{
	if (IsComplete())
		return TRUE;
	Data->m_pRefFrame = makecopyTVFrame(Data->m_pDstFrame);
	_negative(Data->m_pRefFrame);
	_skeletize(Data->m_pRefFrame);
	_negative(Data->m_pRefFrame);
	SetComplete(Data->m_pRefFrame != NULL);
	return IsComplete();
}

void CProcStepRefine::Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view)
{
	if (!view->GetFramePntr() || memcmp(GetData(view->GetFramePntr()), GetData(Data->m_pRefFrame), Data->m_pRefFrame->lpBMIH->biSizeImage))
		view->LoadFrame(Data->m_pRefFrame);
}

BOOL CProcStepRectify::Process(CData* Data, void* params)
{
	if (IsComplete())
		return TRUE;
	Data->m_pRectFrame = makecopyTVFrame(Data->m_pRefFrame);
	_rectify(Data->m_pRectFrame);
	SetComplete(Data->m_pRectFrame != NULL);
	return IsComplete();
}

void CProcStepRectify::Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view)
{
	if (!view->GetFramePntr() || memcmp(GetData(view->GetFramePntr()), GetData(Data->m_pRectFrame), Data->m_pRectFrame->lpBMIH->biSizeImage))
		view->LoadFrame(Data->m_pRectFrame);
}

BOOL CProcStepVert::Process(CData* Data, void* params)
{
	if (IsComplete())
		return TRUE;
	Data->m_pVertFrame = makecopyTVFrame(Data->m_pRectFrame);
	_v_erode(Data->m_pVertFrame);
	SetComplete(Data->m_pVertFrame != NULL);
	return IsComplete();
}

void CProcStepVert::Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view)
{
	if (!view->GetFramePntr() || memcmp(GetData(view->GetFramePntr()), GetData(Data->m_pVertFrame), Data->m_pVertFrame->lpBMIH->biSizeImage))
		view->LoadFrame(Data->m_pVertFrame);
}

BOOL CProcStepVFlt::Process(CData* Data, void* params)
{
	if (IsComplete())
		return TRUE;
	pTVFrame Copy = makecopyTVFrame(Data->m_pVertFrame);
    _clear_frames(Copy,255);
    //saveDIB("C:\\intermediate.bmp",Copy);
	_negative(Copy);
	Data->m_VertLinesClusters.SetClusterColors(255, 255);
//	Data->m_VertLinesClusters.SetBWImage();
	Data->m_VertLinesClusters.DiagonalConections();
	Data->m_VertLinesClusters.ParseFrame(Copy);
	pClustersInfo ci = Data->m_VertLinesClusters.GetClusterInfo();
	Cluster_DimsFilter(ci->m_Clusters, ci->m_ClustersNmb, 20, 100000);
	Cluster_XYRatioFilter(ci->m_Clusters, ci->m_ClustersNmb, 0, 0.1);
	ci = Data->m_VertLinesClusters.GetClusterInfo();
	Data->m_pVFltFrame = Cluster_KeepUndelClustersOnFrame(ci->m_Clusters, ci->m_ClustersNmb, Copy);
	_negative(Data->m_pVFltFrame);
	freeTVFrame(Copy);
	SetComplete(Data->m_pVFltFrame != NULL);
	return IsComplete();
}

void CProcStepVFlt::Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view)
{
	if (!view->GetFramePntr() || memcmp(GetData(view->GetFramePntr()), GetData(Data->m_pVFltFrame), Data->m_pVFltFrame->lpBMIH->biSizeImage))
		view->LoadFrame(Data->m_pVFltFrame);
}

BOOL CProcStepHorz::Process(CData* Data, void* params)
{
	if (IsComplete())
		return TRUE;
	Data->m_pHorzFrame = makecopyTVFrame(Data->m_pRectFrame);
	_h_erode(Data->m_pHorzFrame);
	SetComplete(Data->m_pHorzFrame != NULL);
	return IsComplete();
}

void CProcStepHorz::Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view)
{
	if (!view->GetFramePntr() || memcmp(GetData(view->GetFramePntr()), GetData(Data->m_pHorzFrame), Data->m_pHorzFrame->lpBMIH->biSizeImage))
		view->LoadFrame(Data->m_pHorzFrame);
}

BOOL CProcStepHFlt::Process(CData* Data, void* params)
{
	if (IsComplete())
		return TRUE;
	pTVFrame Copy = makecopyTVFrame(Data->m_pHorzFrame);
    _clear_frames(Copy,255);
	_negative(Copy);
	Data->m_HorzLinesClusters.SetClusterColors(255, 255);
//	Data->m_HorzLinesClusters.SetBWImage();
	Data->m_HorzLinesClusters.DiagonalConections();
	Data->m_HorzLinesClusters.ParseFrame(Copy);
	pClustersInfo ci = Data->m_HorzLinesClusters.GetClusterInfo();
	Cluster_DimsFilter(ci->m_Clusters, ci->m_ClustersNmb, 20, 100000);
	Cluster_XYRatioFilter(ci->m_Clusters, ci->m_ClustersNmb, 10, 100000);
	ci = Data->m_HorzLinesClusters.GetClusterInfo();
	Data->m_pHFltFrame = Cluster_KeepUndelClustersOnFrame(ci->m_Clusters, ci->m_ClustersNmb, Copy);
	_negative(Data->m_pHFltFrame);
	freeTVFrame(Copy);
	SetComplete(Data->m_pHFltFrame != NULL);
	return IsComplete();
}

void CProcStepHFlt::Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view)
{
	if (!view->GetFramePntr() || memcmp(GetData(view->GetFramePntr()), GetData(Data->m_pHFltFrame), Data->m_pHFltFrame->lpBMIH->biSizeImage))
		view->LoadFrame(Data->m_pHFltFrame);
}

BOOL CProcStepRefineVLines::Process(CData* Data, void* params)
{
	if (IsComplete())
		return TRUE;
	Data->m_pVLinesFrame = makecopyTVFrame(Data->m_pSrcFrame);
	memset(GetData(Data->m_pVLinesFrame), 255, Data->m_pVLinesFrame->lpBMIH->biWidth * Data->m_pVLinesFrame->lpBMIH->biHeight);
	pClustersInfo ci = Data->m_VertLinesClusters.GetClusterInfo();
	for (int i = 0; i < ci->m_ClustersNmb; i++)
	{
		if (ci->m_Clusters[i].deleted)
			continue;
		CLine* Line = Data->m_VertLines.AddLine();
		for (int y = 0; y < ci->m_Clusters[i].size.cy; y++)
		{
			int xc = 0, count = 0;
			for (int x = 0; x < ci->m_Clusters[i].size.cx; x++)
			{
				if (ci->m_Clusters[i].clstBM[x + y * ci->m_Clusters[i].size.cx] != (BYTE)ci->m_Clusters[i].color)
					continue;
				xc += x;
				count++;
			}
            if (!count) continue;
			POINT pt = { xc / count + ci->m_Clusters[i].offset.x, y + ci->m_Clusters[i].offset.y };
			_mark_hmin(Data->m_pVLinesFrame, Data->m_pSrcFrame, pt);
			POINT dot = { pt.y, pt.x };
			Line->Add(dot);
		}
	}
	LPBYTE Dst = GetData(Data->m_pVLinesFrame);
	LPBYTE Src = GetData(Data->m_pSrcFrame);
	LPBYTE End = Src + Data->m_pSrcFrame->lpBMIH->biWidth * Data->m_pSrcFrame->lpBMIH->biHeight;
	while (Src < End)
	{
		if (*Dst)
			*Dst = 128 + (*Src) / 2;
		Src++;
		Dst++;
	}
	SetComplete(Data->m_pVLinesFrame != NULL);
	return IsComplete();
}

void CProcStepRefineVLines::Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view)
{
	if (!view->GetFramePntr() || memcmp(GetData(view->GetFramePntr()), GetData(Data->m_pVLinesFrame), Data->m_pVLinesFrame->lpBMIH->biSizeImage))
		view->LoadFrame(Data->m_pVLinesFrame);
}

BOOL CProcStepRefineHLines::Process(CData* Data, void* params)
{
	if (IsComplete())
		return TRUE;
	Data->m_pHLinesFrame = makecopyTVFrame(Data->m_pSrcFrame);
	memset(GetData(Data->m_pHLinesFrame), 255, Data->m_pHLinesFrame->lpBMIH->biWidth * Data->m_pHLinesFrame->lpBMIH->biHeight);
	pClustersInfo ci = Data->m_HorzLinesClusters.GetClusterInfo();
	for (int i = 0; i < ci->m_ClustersNmb; i++)
	{
		if (ci->m_Clusters[i].deleted)
			continue;
		CLine* Line = Data->m_HorzLines.AddLine();
		for (int x = 0; x < ci->m_Clusters[i].size.cx; x++)
		{
			int yc = 0, count = 0;
			for (int y = 0; y < ci->m_Clusters[i].size.cy; y++)
			{
				if (ci->m_Clusters[i].clstBM[x + y * ci->m_Clusters[i].size.cx] != (BYTE)ci->m_Clusters[i].color)
					continue;
				yc += y;
				count++;
			}
			if (!count) continue;
			POINT pt = { x + ci->m_Clusters[i].offset.x, yc / count + ci->m_Clusters[i].offset.y };
			_mark_vmin(Data->m_pHLinesFrame, Data->m_pSrcFrame, pt);
			Line->Add(pt);
		}
	}
	LPBYTE Dst = GetData(Data->m_pHLinesFrame);
	LPBYTE Src = GetData(Data->m_pSrcFrame);
	LPBYTE End = Src + Data->m_pSrcFrame->lpBMIH->biWidth * Data->m_pSrcFrame->lpBMIH->biHeight;
	while (Src < End)
	{
		if (*Dst)
			*Dst = 128 + (*Src) / 2;
		Src++;
		Dst++;
	}
	SetComplete(Data->m_pHLinesFrame != NULL);
	return IsComplete();
}

void CProcStepRefineHLines::Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view)
{
	if (!view->GetFramePntr() || memcmp(GetData(view->GetFramePntr()), GetData(Data->m_pHLinesFrame), Data->m_pHLinesFrame->lpBMIH->biSizeImage))
		view->LoadFrame(Data->m_pHLinesFrame);
}

BOOL CProcStepApproxVLines::Process(CData* Data, void* params)
{
	if (IsComplete())
		return TRUE;
	for (int i = 0; i < Data->m_VertLines.GetCount(); i++)
		Line_EstimateParameters(Data->m_VertLines.GetAt(i));
	_index_lines(&Data->m_VertLines);
	SetComplete(TRUE);
	return IsComplete();
}

void CProcStepApproxVLines::Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view)
{
	if (!view->GetFramePntr() || memcmp(GetData(view->GetFramePntr()), GetData(Data->m_pVLinesFrame), Data->m_pVLinesFrame->lpBMIH->biSizeImage))
		view->LoadFrame(Data->m_pVLinesFrame);
	HGDIOBJ hFont = ::SelectObject(hdc, m_Font.m_hObject);
	int bkMode = ::SetBkMode(hdc, TRANSPARENT);
	COLORREF color = ::SetTextColor(hdc, RGB(255, 0, 0));
	for (int i = 0; i < Data->m_VertLines.GetCount(); i++)
	{
		CLine* Line = Data->m_VertLines.GetAt(i);
		LPLINEAPPROXPARAMS lap = Line->GetParams();
		if (!lap->valid)
			continue;
		CPoint pt;
		for (int j = (int)lap->argMin; j <= (int)lap->argMax; j++)
		{
			pt.x = (int)(lap->a * (double)(j * j) + lap->b * (double)j + lap->c);
			pt.y = j;
			view->Pic2Scr(pt);
			::SetPixel(hdc, pt.x, pt.y, RGB(255, 0, 0));
		}
		pt.x = (int)(lap->a * lap->argMin * lap->argMin + lap->b * lap->argMin + lap->c);
		pt.y = (int)lap->argMin;
		view->Pic2Scr(pt);
		FXString text;
		text.Format("%d", lap->index);
		SIZE sz;
		::GetTextExtentPoint32(hdc, text, (int) text.GetLength(), &sz);
		RECT rc;
		rc.top = pt.y - sz.cy - 1;
		rc.left = pt.x + 5;
		rc.right = rc.left + sz.cx;
		rc.bottom = rc.top + sz.cy;
		::DrawText(hdc, text, (int) text.GetLength(), &rc, DT_CENTER);
	}
	::SetBkMode(hdc, bkMode);
	::SelectObject(hdc, hFont);
	::SetTextColor(hdc, color);
}

BOOL CProcStepApproxHLines::Process(CData* Data, void* params)
{
	if (IsComplete())
		return TRUE;
	for (int i = 0; i < (int) Data->m_HorzLines.GetCount(); i++)
		Line_EstimateParameters(Data->m_HorzLines.GetAt(i));
	_index_lines(&Data->m_HorzLines);
	SetComplete(TRUE);
	return IsComplete();
}

void CProcStepApproxHLines::Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view)
{
	if (!view->GetFramePntr() || memcmp(GetData(view->GetFramePntr()), GetData(Data->m_pHLinesFrame), Data->m_pHLinesFrame->lpBMIH->biSizeImage))
		view->LoadFrame(Data->m_pHLinesFrame);
	HGDIOBJ hFont = ::SelectObject(hdc, m_Font.m_hObject);
	int bkMode = ::SetBkMode(hdc, TRANSPARENT);
	COLORREF color = ::SetTextColor(hdc, RGB(255, 0, 0));
	for (int i = 0; i < Data->m_HorzLines.GetCount(); i++)
	{
		CLine* Line = Data->m_HorzLines.GetAt(i);
		LPLINEAPPROXPARAMS lap = Line->GetParams();
		if (!lap->valid)
			continue;
		CPoint pt;
		for (int j = (int)lap->argMin; j <= (int)lap->argMax; j++)
		{
			pt.x = j;
			pt.y = (int)(lap->a * (double)(j * j) + lap->b * (double)j + lap->c);
			view->Pic2Scr(pt);
			::SetPixel(hdc, pt.x, pt.y, RGB(255, 0, 0));
		}
		pt.x = (int)lap->argMin;
		pt.y = (int)(lap->a * lap->argMin * lap->argMin + lap->b * lap->argMin + lap->c);
		view->Pic2Scr(pt);
		FXString text;
		text.Format("%d", lap->index);
		SIZE sz;
		::GetTextExtentPoint32(hdc, text, (int) text.GetLength(), &sz);
		RECT rc;
		rc.top = pt.y - sz.cy - 1;
		rc.left = pt.x + 5;
		rc.right = rc.left + sz.cx;
		rc.bottom = rc.top + sz.cy;
		::DrawText(hdc, text, (int) text.GetLength(), &rc, DT_CENTER);
	}
	::SetBkMode(hdc, bkMode);
	::SelectObject(hdc, hFont);
	::SetTextColor(hdc, color);
}

void CProcStepShowLines::Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view)
{
	if (!view->GetFramePntr() || memcmp(GetData(view->GetFramePntr()), GetData(Data->m_pSrcFrame), Data->m_pSrcFrame->lpBMIH->biSizeImage))
		view->LoadFrame(Data->m_pSrcFrame);
    int i;
	for (i = 0; i < Data->m_HorzLines.GetCount(); i++)
	{
		CLine* Line = Data->m_HorzLines.GetAt(i);
		LPLINEAPPROXPARAMS lap = Line->GetParams();
		if (!lap->valid)
			continue;
		for (int j = (int)lap->argMin; j <= (int)lap->argMax; j++)
		{
			CPoint pt;
			pt.x = j;
			pt.y = (int)(lap->a * (double)(j * j) + lap->b * (double)j + lap->c);
			view->Pic2Scr(pt);
			::SetPixel(hdc, pt.x, pt.y, RGB(255, 0, 0));
		}
	}
	for (i = 0; i < Data->m_VertLines.GetCount(); i++)
	{
		CLine* Line = Data->m_VertLines.GetAt(i);
		LPLINEAPPROXPARAMS lap = Line->GetParams();
		if (!lap->valid)
			continue;
		for (int j = (int)lap->argMin; j <= (int)lap->argMax; j++)
		{
			CPoint pt;
			pt.x = (int)(lap->a * (double)(j * j) + lap->b * (double)j + lap->c);
			pt.y = j;
			view->Pic2Scr(pt);
			::SetPixel(hdc, pt.x, pt.y, RGB(0, 255, 0));
		}
	}
}

BOOL CProcStepNodes::Process(CData* Data, void* params)
{
	if (IsComplete())
		return TRUE;
	for (int i = 0; i < Data->m_VertLines.GetCount(); i++)
	{
		CLine* VLine = Data->m_VertLines.GetAt(i);
		for (int j = 0; j < Data->m_HorzLines.GetCount(); j++)
		{
			CLine* HLine = Data->m_HorzLines.GetAt(j);
			double x, y;
			if (Line_FindCross(VLine, HLine, x, y))
				Data->m_Nodes.AddNode(x, y, VLine, HLine);
		}
	}
	SetComplete(TRUE);
	return IsComplete();
}

void CProcStepNodes::Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view)
{
	if (!view->GetFramePntr() || memcmp(GetData(view->GetFramePntr()), GetData(Data->m_pSrcFrame), Data->m_pSrcFrame->lpBMIH->biSizeImage))
		view->LoadFrame(Data->m_pSrcFrame);
	for (int i = 0; i < Data->m_Nodes.GetCount(); i++)
	{
		NODE node = Data->m_Nodes.GetAt(i);
		CPoint pt;
		pt.x = (int)node.m_x;
		pt.y = (int)node.m_y;
		view->Pic2Scr(pt);
		::SetPixel(hdc, pt.x, pt.y, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x - 1, pt.y - 1, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x - 1, pt.y + 1, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x + 1, pt.y - 1, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x + 1, pt.y + 1, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x - 2, pt.y - 2, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x - 2, pt.y + 2, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x + 2, pt.y - 2, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x + 2, pt.y + 2, RGB(255, 0, 0));
	}
}

BOOL CProcStepRefNodes::Process(CData* Data, void* params)
{
	if (IsComplete())
		return TRUE;
    int i=0;
	int* vInd = (int*)calloc(Data->m_VertLines.GetCount(), sizeof(int));
	int* hInd = (int*)calloc(Data->m_HorzLines.GetCount(), sizeof(int));

    for (i = 0; i < Data->m_Nodes.GetCount(); i++)
	{
		NODE node = Data->m_Nodes.GetAt(i);
		vInd[Line_GetIndex(node.m_VLine)] = 1;
		hInd[Line_GetIndex(node.m_HLine)] = 1;
	}
	for (i = 1; i < Data->m_VertLines.GetCount(); i++)
		vInd[i] += vInd[i - 1];
	for (i = 1; i < Data->m_HorzLines.GetCount(); i++)
		hInd[i] += hInd[i - 1];
	for (i = 0; i < Data->m_Nodes.GetCount(); i++)
	{
		NODE node = Data->m_Nodes.GetAt(i);
		const double step = 5; //mm between nodes
		double x = step * (double)(vInd[Line_GetIndex(node.m_VLine)] - 1);
		double y = step * (double)(hInd[Line_GetIndex(node.m_HLine)] - 1);
		node.m_refX = x;
		node.m_refY = y;
		Data->m_Nodes.SetAt(i, node);
	}
	free(vInd);
	free(hInd);
	SetComplete(TRUE);
	return IsComplete();
}

void CProcStepRefNodes::Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view)
{
	if (!view->GetFramePntr() || memcmp(GetData(view->GetFramePntr()), GetData(Data->m_pSrcFrame), Data->m_pSrcFrame->lpBMIH->biSizeImage))
		view->LoadFrame(Data->m_pSrcFrame);
	for (int i = 0; i < Data->m_Nodes.GetCount(); i++)
	{
		NODE node = Data->m_Nodes.GetAt(i);
		CPoint pt;
		pt.x = (int)node.m_x;
		pt.y = (int)node.m_y;
		view->Pic2Scr(pt);
		::SetPixel(hdc, pt.x, pt.y, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x - 1, pt.y - 1, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x - 1, pt.y + 1, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x + 1, pt.y - 1, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x + 1, pt.y + 1, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x - 2, pt.y - 2, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x - 2, pt.y + 2, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x + 2, pt.y - 2, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x + 2, pt.y + 2, RGB(255, 0, 0));
		HGDIOBJ hFont = ::SelectObject(hdc, m_Font.m_hObject);
		int bkMode = ::SetBkMode(hdc, TRANSPARENT);
		COLORREF color = ::SetTextColor(hdc, RGB(255, 0, 0));
		FXString text;
		text.Format("(%.f, %.f)", node.m_refX, node.m_refY);
		SIZE sz;
		::GetTextExtentPoint32(hdc, text, (int) text.GetLength(), &sz);
		RECT rc;
		rc.top = pt.y - sz.cy - 1;
		rc.left = pt.x + 1;
		rc.right = rc.left + sz.cx;
		rc.bottom = rc.top + sz.cy;
		::DrawText(hdc, text, (int) text.GetLength(), &rc, DT_CENTER);
		::SetBkMode(hdc, bkMode);
		::SelectObject(hdc, hFont);
		::SetTextColor(hdc, color);
	}
}

BOOL CProcStepMapper::Process(CData* Data, void* params)
{
	if (IsComplete())
		return TRUE;
	if (Data->m_Mapper.BuildMap(&Data->m_Nodes))
	{
		SetComplete(TRUE);
#ifdef _DEBUG
		_diagnose_nodes(&Data->m_Mapper, &Data->m_Nodes);
		_calculate_distortion(&Data->m_Mapper, &Data->m_Dist, Data->m_pSrcFrame);
#endif
	}
	return IsComplete();
}

void CProcStepMapper::Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view)
{
	if (!view->GetFramePntr() || memcmp(GetData(view->GetFramePntr()), GetData(Data->m_pSrcFrame), Data->m_pSrcFrame->lpBMIH->biSizeImage))
		view->LoadFrame(Data->m_pSrcFrame);
    int i;
	for (i = 0; i < Data->m_Nodes.GetCount(); i++)
	{
		NODE node = Data->m_Nodes.GetAt(i);
		CPoint pt;
		pt.x = (int)node.m_x;
		pt.y = (int)node.m_y;
		view->Pic2Scr(pt);
		::SetPixel(hdc, pt.x, pt.y, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x - 1, pt.y - 1, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x - 1, pt.y + 1, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x + 1, pt.y - 1, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x + 1, pt.y + 1, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x - 2, pt.y - 2, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x - 2, pt.y + 2, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x + 2, pt.y - 2, RGB(255, 0, 0));
		::SetPixel(hdc, pt.x + 2, pt.y + 2, RGB(255, 0, 0));
	}
	if (Data->m_bRulerSet)
	{
		double x1 = (double)Data->m_Ruler.left;
		double y1 = (double)Data->m_Ruler.top;
		double x2 = (double)Data->m_Ruler.right;
		double y2 = (double)Data->m_Ruler.bottom;
		if (Data->m_Mapper.Map(x1, y1) && Data->m_Mapper.Map(x2, y2))
		{
			double dist = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
			FXString text;
			text.Format(" %5.2f mm ", dist);
			::TextOut(hdc, 1, 1, text, (int) text.GetLength());
		}
	}
#ifdef _DEBUG
	for (i = 0; i < (int) Data->m_Dist.GetCount(); i++)
	{
		NODE node = Data->m_Dist.GetAt(i);
		CPoint pt;
		pt.x = (int)node.m_x;
		pt.y = (int)node.m_y;
		view->Pic2Scr(pt);
		::SetPixel(hdc, pt.x, pt.y, RGB(255, 0, 0));
	}
    if (Data->m_Dist.GetCount())
    {
	    NODE node = Data->m_Dist.GetPoleN();
	    CPoint pt;
	    pt.x = (int)node.m_x;
	    pt.y = (int)node.m_y;
	    view->Pic2Scr(pt);
	    ::MoveToEx(hdc, pt.x, pt.y, NULL);
	    HGDIOBJ hPen = ::GetCurrentObject(hdc, OBJ_PEN);
	    ::SelectObject(hdc, m_pen1.m_hObject);
	    node = Data->m_Dist.GetPoleS();
	    pt.x = (int)node.m_x;
	    pt.y = (int)node.m_y;
	    view->Pic2Scr(pt);
	    ::LineTo(hdc, pt.x, pt.y);
	    node = Data->m_Dist.GetPoleW();
	    pt.x = (int)node.m_x;
	    pt.y = (int)node.m_y;
	    view->Pic2Scr(pt);
	    ::MoveToEx(hdc, pt.x, pt.y, NULL);
	    node = Data->m_Dist.GetPoleE();
	    pt.x = (int)node.m_x;
	    pt.y = (int)node.m_y;
	    view->Pic2Scr(pt);
	    ::SelectObject(hdc, m_pen2.m_hObject);
	    ::LineTo(hdc, pt.x, pt.y);
	    ::SelectObject(hdc, hPen);
    }
#endif
}