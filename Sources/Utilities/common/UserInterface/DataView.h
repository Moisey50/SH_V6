//  $File : DataView.h : header file - Implements simple diagram view
//  (C) Copyright The FileX Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#if !defined(AFX_DATAVIEW_H__CBD40C04_BC89_11D4_9462_000001360793__INCLUDED_)
#define AFX_DATAVIEW_H__CBD40C04_BC89_11D4_9462_000001360793__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxmt.h>
#include <afxtempl.h>
#include <imageproc\statistics.h>

/////////////////////////////////////////////////////////////////////////////
// CDataView window

class CDataView : public CWnd
{
// Construction
private:
    CSData  m_Data;
    CPen    m_Pen;
    CPen    m_RedPen;
    RECT    m_RC;
    double  m_scalex,m_scaley,m_min,m_max;
    int     m_MaxDataStored;
    CPoint  m_MousePnt;
    bool    m_ScaleAuto;
public:
    CDataView():m_Pen(PS_SOLID,1,RGB(0,255,0)),m_RedPen(PS_SOLID,1,RGB(255,0,0)) {m_MaxDataStored=-1; m_min=0; m_max=255; };
    int     Add(double Value);
    void    SetMaxDataNmb(int DataNmb) {m_MaxDataStored=DataNmb;};
    void    SetMinMaxData(double min, double max) { m_ScaleAuto=false;m_min=min;m_max=max;};
    void    GetMinMax(double &min, double &max, int* MinVal=NULL, int* MaxVal=NULL);
    void    Reset()           { m_Data.RemoveAll();};
    CSData& GetData()         { return m_Data;};
    CPoint  GetMousePnt();
    int     GetLastVal()      { if (!m_Data.GetSize()) return 0; return (int)m_Data.GetAt(m_Data.GetSize()-1);};
    void    SetScaleAuto(bool val=true) {m_ScaleAuto=val; };
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataView)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
public:
	void GetMinMaxVal(double &min, double &max);
	void GetAverage(double &Val);
	void GetAverageVal(double& Val);
	// Generated message map functions
protected:
	//{{AFX_MSG(CDataView)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DATAVIEW_H__CBD40C04_BC89_11D4_9462_000001360793__INCLUDED_)
