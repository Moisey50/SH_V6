// Processing.h: interface for the CProcessing class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCESSING_H__40B54450_2C2C_402C_A323_BA0C2CEE146B__INCLUDED_)
#define AFX_PROCESSING_H__40B54450_2C2C_402C_A323_BA0C2CEE146B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AfxTempl.h"
#include "ProcessingStep.h"

class CProcessing  
{
public:
	CProcessing();
	virtual ~CProcessing();

	BOOL	IsLastStep();
	BOOL	IsFirstStep();
	BOOL	IsKeyStep();
	BOOL	IsProcessing();
	BOOL	Next(CData* Data, void* params);
	BOOL	Back();
	void	Reset();
	void	DrawResults(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view);
	FXString	GetCurComment();

private:
	void				AddStep(CProcessingStep* Step, BOOL bKeyStep = FALSE);
	CProcessingStep*	GetStep(int i);
	void				RemoveStep(int i);
	int					GetStepsCount();

private:
	CArray<CProcessingStep*, CProcessingStep*>	m_ProcSteps;
	int											m_nCurStep;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Processing steps

class CProcStepLoader : public CProcessingStep
{
public:
	CProcStepLoader() : CProcessingStep("file load") { };
	virtual ~CProcStepLoader() { };

	virtual BOOL	Process(CData* Data, void* params);
	virtual void	Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view);
};

class CProcStepFilter : public CProcessingStep
{
public:
	CProcStepFilter() : CProcessingStep("high pass and binarization") { };
	virtual ~CProcStepFilter() { };

	virtual BOOL	Process(CData* Data, void* params);
	virtual void	Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view);
};

class CProcStepRefine : public CProcessingStep
{
public:
	CProcStepRefine() : CProcessingStep("skeletizing") { };
	virtual ~CProcStepRefine() { };

	virtual BOOL	Process(CData* Data, void* params);
	virtual void	Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view);
};

class CProcStepRectify : public CProcessingStep
{
public:
	CProcStepRectify() : CProcessingStep("flatter shivers") { };
	virtual ~CProcStepRectify() { };

	virtual BOOL	Process(CData* Data, void* params);
	virtual void	Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view);
};

class CProcStepVert : public CProcessingStep
{
public:
	CProcStepVert() : CProcessingStep("vertical erode") { };
	virtual ~CProcStepVert() { };

	virtual BOOL	Process(CData* Data, void* params);
	virtual void	Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view);
};

class CProcStepVFlt : public CProcessingStep
{
public:
	CProcStepVFlt() : CProcessingStep("remove small clusters") { };
	virtual ~CProcStepVFlt() { };

	virtual BOOL	Process(CData* Data, void* params);
	virtual void	Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view);
};

class CProcStepHorz : public CProcessingStep
{
public:
	CProcStepHorz() : CProcessingStep("horizontal erode") { };
	virtual ~CProcStepHorz() { };

	virtual BOOL	Process(CData* Data, void* params);
	virtual void	Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view);
};

class CProcStepHFlt : public CProcessingStep
{
public:
	CProcStepHFlt() : CProcessingStep("remove small clusters") { };
	virtual ~CProcStepHFlt() { };

	virtual BOOL	Process(CData* Data, void* params);
	virtual void	Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view);
};

class CProcStepRefineVLines : public CProcessingStep
{
public:
	CProcStepRefineVLines() : CProcessingStep("mark horizontal minima") { };
	virtual ~CProcStepRefineVLines() { };

	virtual BOOL	Process(CData* Data, void* params);
	virtual void	Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view);
};

class CProcStepRefineHLines : public CProcessingStep
{
public:
	CProcStepRefineHLines() : CProcessingStep("mark vertical minima") { };
	virtual ~CProcStepRefineHLines() { };

	virtual BOOL	Process(CData* Data, void* params);
	virtual void	Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view);
};

class CProcStepApproxVLines : public CProcessingStep
{
	CFont m_Font;
public:
	CProcStepApproxVLines() : CProcessingStep("make parabolic approximation")
	{
		m_Font.CreateFont(-12, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH | FF_SWISS, "Arial");
	};
	virtual ~CProcStepApproxVLines() { };

	virtual BOOL	Process(CData* Data, void* params);
	virtual void	Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view);
};

class CProcStepApproxHLines : public CProcessingStep
{
	CFont m_Font;
public:
	CProcStepApproxHLines() : CProcessingStep("make parabolic approximation")
	{
		m_Font.CreateFont(-12, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH | FF_SWISS, "Arial");
	};
	virtual ~CProcStepApproxHLines() { };

	virtual BOOL	Process(CData* Data, void* params);
	virtual void	Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view);
};

class CProcStepShowLines : public CProcessingStep
{
public:
	CProcStepShowLines() : CProcessingStep("show lines") { };
	virtual ~CProcStepShowLines() { };

	virtual BOOL	Process(CData* Data, void* params) { return TRUE; };
	virtual void	Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view);
};

class CProcStepNodes : public CProcessingStep
{
public:
	CProcStepNodes() : CProcessingStep("calculate nodes") { };
	virtual ~CProcStepNodes() { };

	virtual BOOL	Process(CData* Data, void* params);
	virtual void	Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view);
};

class CProcStepRefNodes : public CProcessingStep
{
	CFont m_Font;
public:
	CProcStepRefNodes() : CProcessingStep("reference nodes")
	{
		m_Font.CreateFont(-12, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH | FF_SWISS, "Arial");
	};
	virtual ~CProcStepRefNodes() { };

	virtual BOOL	Process(CData* Data, void* params);
	virtual void	Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view);
};

class CProcStepMapper : public CProcessingStep
{
	CPen	m_pen1, m_pen2;
public:
	CProcStepMapper() : CProcessingStep("build mapper")
	{
		m_pen1.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		m_pen2.CreatePen(PS_SOLID, 1, RGB(255, 128, 0));
	};
	virtual ~CProcStepMapper() { };

	virtual BOOL	Process(CData* Data, void* params);
	virtual void	Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view);
};

#endif // !defined(AFX_PROCESSING_H__40B54450_2C2C_402C_A323_BA0C2CEE146B__INCLUDED_)
