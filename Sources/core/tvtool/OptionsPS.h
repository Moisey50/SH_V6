#if !defined(AFX_OPTIONSPS_H__3B87C87E_C61B_476C_A97B_8CD3B83A2532__INCLUDED_)
#define AFX_OPTIONSPS_H__3B87C87E_C61B_476C_A97B_8CD3B83A2532__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsPS.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsPS
#include "LogPP.h"
#include "WrkBnchPP.h"
#include "SketchViewPP.h"

class COptionsPS : public CPropertySheet
{
	DECLARE_DYNAMIC(COptionsPS)
protected:
    CWrkBnchPP m_WrkBnchPP;
    CLogPP     m_LogPP;
    CSketchViewPP m_ViewPP;
public:
	COptionsPS(CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
public:
    void InitLogLevel(int i) { m_LogPP.InitLogLevel(i);}
    int  GetLogLevel()       { return m_LogPP.GetLogLevel(); }
    void SetWriteFile(bool set) { m_LogPP.SetWriteFile(set); }
    bool GetWriteFile() { return m_LogPP.GetWriteFile(); }
    void SetLogFileName(LPCTSTR name) { m_LogPP.SetLogFileName(name);}
    LPCTSTR GetLogFileName() { return m_LogPP.GetLogFileName(); };
    void InitGadgetTreeShowType(bool show) { m_WrkBnchPP.m_GadgetTreeShowType=show; }
    bool GetGadgetTreeShowType()           { return (m_WrkBnchPP.m_GadgetTreeShowType!=FALSE); }
    void InitViewType(ViewType vt) { m_ViewPP.InitViewType(vt); }
    ViewType GetViewType() { return m_ViewPP.GetViewType(); }
    void SetViewActivity(bool set) { m_ViewPP.m_ViewActivity=set; }
    bool GetViewActivity() { return (m_ViewPP.m_ViewActivity!=FALSE); }
    void SetUseMasterExecutionStatus(bool set) { m_WrkBnchPP.m_bUseMasterExecutionStatus=set; }
    bool GetUseMasterExecutionStatus() { return (m_WrkBnchPP.m_bUseMasterExecutionStatus!=FALSE); }
    void SetSaveGadgetPositions(bool set) { m_WrkBnchPP.m_SaveGadgetPositions=set; }
    bool GetSaveGadgetPositions() { return (m_WrkBnchPP.m_SaveGadgetPositions!=FALSE); }
    void SetSaveFltWindowsPos(bool set) { m_WrkBnchPP.m_SaveFltWindowsPos=set; }
    bool GetSaveFltWindowsPos() { return (m_WrkBnchPP.m_SaveFltWindowsPos!=FALSE); }
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionsPS)
	//}}AFX_VIRTUAL
public:
	~COptionsPS();
protected:
	//{{AFX_MSG(COptionsPS)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSPS_H__3B87C87E_C61B_476C_A97B_8CD3B83A2532__INCLUDED_)

