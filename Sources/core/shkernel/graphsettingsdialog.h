#if !defined(AFX_GRAPHSETTINGSDIALOG_H__923E22B6_8FB9_4626_B446_E2600371E859__INCLUDED_)
#define AFX_GRAPHSETTINGSDIALOG_H__923E22B6_8FB9_4626_B446_E2600371E859__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GraphSettingsDialog.h : header file
//
#include "resource.h"
#include <afxtempl.h>
#include "itemdata.h"

#define CALL_FITTEDSETUP -1

/////////////////////////////////////////////////////////////////////////////
// CGraphSettingsDialog dialog
class CGraphBuilder;

class CGraphSettingsDialog : public CDialog
{
    friend void OnGridEvent(int Event, void *wParam, int col, int row, int uData);
private:
    CGraphBuilder*  m_Builder; 
    //CInfoParser    m_Data;
    FXParser        m_Data;
    int             m_ItemsCnt;
    CArray<itemData,itemData> m_ItemsData;
public:
	CGraphSettingsDialog(CGraphBuilder* gb, CWnd* pParent = NULL);   // standard constructor
    void    SetData(LPCTSTR data);
	bool    InsertStdDlg(LPCTSTR name, LPCTSTR key, LPCTSTR params, int pos=-1);
	bool    RemoveStdDlg(LPCTSTR gadgetname);
	//{{AFX_DATA(CGraphSettingsDialog)
	enum { IDD = IDD_SETTINGS_DIALOG };
	CGridListCtrl m_SetupGrid;
	//}}AFX_DATA
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGraphSettingsDialog)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void CheckScrollbar();
	int  GetCurrentGap();
    //bool ParseGadgetItem(CInfoParser& ip, CString cname);
    bool ParseGadgetItem(FXParser& ip, CString cname);
    bool InsertSimple(LPCTSTR name, LPCTSTR key, LPCTSTR params);
	void CallBack(LPCTSTR dlgID, int ctrlID, CString* Data, bool &bVal);
    void OnGridEvent(int Event, int col, int row, int uData);
	// Generated message map functions
	//{{AFX_MSG(CGraphSettingsDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRAPHSETTINGSDIALOG_H__923E22B6_8FB9_4626_B446_E2600371E859__INCLUDED_)
