//  $File : DataView.cpp : implementation file - Implements simple diagram view
//  (C) Copyright The FileX Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)

#include "stdafx.h"
#include <MATH.H>
#include <FLOAT.H>
#include <userinterface\DataView.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDataView

BEGIN_MESSAGE_MAP(CDataView, CWnd)
	//{{AFX_MSG_MAP(CDataView)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDataView message handlers

BOOL CDataView::Create(CWnd* pParentWnd)
{
	if (m_hWnd) return(TRUE);
    BOOL RESULT;
    RECT rc;
    pParentWnd->GetClientRect(&rc);
    m_ScaleAuto=false;
    RESULT=CWnd::Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, rc, pParentWnd, AFX_IDW_PANE_FIRST, NULL);
    m_MousePnt=CPoint(-1,-1);
	UpdateWindow();
	return (RESULT);
}


void CDataView::OnPaint() 
{
	CPaintDC dc(this); 
    m_Data.m_Busy.Lock();
	GetClientRect(&m_RC);
    dc.FillSolidRect(&m_RC, RGB(0,0,15));
    if (!m_Data.GetSize()) 
    { 
        m_Data.m_Busy.Unlock(); 
        return; 
    }
    double min=m_min,max=m_max;
    if ((m_ScaleAuto) && (m_Data.GetSize()))
    {
        min=m_Data.GetAt(0);
        max=m_Data.GetAt(0);
    }
    int i;
    for (i=0; i<m_Data.GetSize(); i++)
    {
        double data=m_Data.GetAt(i);
        if (data<min) min=data;
        if (data>max) max=data;
    }
    m_scalex=((double)(m_RC.right-m_RC.left))/m_Data.GetSize();
    m_scaley=((double)(m_RC.bottom-m_RC.top))/(max-min);
    CPen *oPen;
    oPen=(CPen*)dc.SelectObject(&m_Pen);
    dc.MoveTo(0,m_RC.bottom-(int)((m_Data.GetAt(0)-min)*m_scaley+0.5));
    for (i=1; i<m_Data.GetSize(); i++)
    {
        dc.LineTo((int)(i*m_scalex+0.5),m_RC.bottom-(int)((m_Data.GetAt(i)-min)*m_scaley+0.5));
    }
    if (m_MousePnt.x>=0)
    {
        dc.SelectObject(&m_RedPen);
        dc.MoveTo(m_MousePnt.x,m_MousePnt.y-5);
        dc.LineTo(m_MousePnt.x,m_MousePnt.y+5);
        dc.MoveTo(m_MousePnt.x-5,m_MousePnt.y);
        dc.LineTo(m_MousePnt.x+5,m_MousePnt.y);

        CString tmpS;
        tmpS.Format("%d:%#4.2f",(int)(m_MousePnt.x/m_scalex),((m_RC.bottom-m_MousePnt.y)/m_scaley));
        dc.SetTextColor(RGB(220,255,0));
        dc.SetBkColor( RGB(0,0,0));
        dc.TextOut(10,10,tmpS);
    }
    m_Data.m_Busy.Unlock();
    dc.SelectObject(oPen);
}

int CDataView::Add(double Value) 
{
    if (m_MaxDataStored>0)
    {
        if (m_Data.GetSize()==m_MaxDataStored)
        {
            m_Data.RemoveAt(0);
        }
    }
    return( (int)m_Data.Add(Value));
}


void CDataView::OnLButtonDown(UINT nFlags, CPoint point) 
{
    m_MousePnt=point;
    Invalidate();
	CWnd::OnLButtonDown(nFlags, point);
}


void CDataView::OnMouseMove(UINT nFlags, CPoint point) 
{
    if (nFlags & MK_LBUTTON)
    {
        m_MousePnt=point;
        Invalidate();
    }
	CWnd::OnMouseMove(nFlags, point);
}

CPoint CDataView::GetMousePnt()     
{ 
    CPoint pnt; 
    if (m_MousePnt.x>=0) 
    {
        pnt.x=(int)(m_MousePnt.x/m_scalex);
        pnt.y=(int)((m_RC.bottom-m_MousePnt.y)/m_scaley);
    } 
    return pnt;
}

void CDataView::GetMinMaxVal(double &min, double &max)
{
    int i;
    min=m_Data[0];
    max=m_Data[0];
    for (i=1; i<m_Data.GetSize(); i++)
    {
        if (min>m_Data[i]) min=m_Data[i];
        if (max<m_Data[i]) max=m_Data[i];
    }
}

void    CDataView::GetMinMax(double &min, double &max, int* MinVal, int* MaxVal)
{
    int i;
    min=-1;
    max=-1;
    for (i=0; i < (int) m_Data.GetSize(); i++)
    {
        if (m_Data[i]>0) 
        {
            min=i; 
            if (MinVal) *MinVal=(int)m_Data[i];
            break; 
        }
    }
    for (i= (int) m_Data.GetSize()-1; i>=0; i--)
    {
        if (m_Data[i]>0) 
        { 
            max=i; 
            if (MaxVal) *MaxVal=(int)m_Data[i];
            break; 
        }
    }
}


void CDataView::GetAverage(double &Val)
{
    int i;
    int cntr=0;
    Val=0.0;
    for (i=0; i< (int) m_Data.GetSize(); i++)
    {
        Val+=m_Data[i]*i;
        cntr+=(int)m_Data[i];
    }
    Val/=cntr;
}

void CDataView::GetAverageVal(double &Val)
{
    int i;
    int cntr=0;
    Val=0.0;
    for (i=0; i< (int) m_Data.GetSize(); i++)
    {
        Val+=m_Data[i];
        cntr++;
    }
    Val/=cntr;
}

