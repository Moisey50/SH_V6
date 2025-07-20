#include "stdafx.h"
#include <math\intf_sup.h>
#include <FLOAT.H>
#include <userinterface\DataFrameView.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDataFrameView

BEGIN_MESSAGE_MAP(CDataFrameView, CWnd)
	//{{AFX_MSG_MAP(CDataFrameView)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDataFrameView message handlers

CDataFrameView::CDataFrameView()	 :
				m_Pen(PS_SOLID,1,RGB(0,255,0)),m_RedPen(PS_SOLID,1,RGB(255,0,0)) 
{
	m_min=0; 
	m_max=1; 
	m_Mode=nulltype;
}

CDataFrameView::~CDataFrameView()
{
	if (m_pWaveData)
	{
		free(m_pWaveData->lpData);
		free(m_pWaveData);
		m_pWaveData=NULL;
	}
}


BOOL CDataFrameView::Create(CWnd* pParentWnd)
{
	if (m_hWnd) return(TRUE);
    BOOL RESULT;
    RECT rc;
    pParentWnd->GetClientRect(&rc);
    m_ScaleAuto=false;
	 m_pWaveData=NULL;
    RESULT=CWnd::Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, rc, pParentWnd, AFX_IDW_PANE_FIRST, NULL);
    m_MousePnt=CPoint(-1,-1);
	UpdateWindow();
	return (RESULT);
}

void CDataFrameView::SetDataFrame(CDataFrame *pDataFrame)
{
	m_Busy.Lock();
   if (Tvdb400_TypesCompatible(pDataFrame->GetDataType(), wave))
	{
		m_Mode=wave;
		if (m_pWaveData)    
		{
			free(m_pWaveData->lpData);
			free(m_pWaveData);
			m_pWaveData=NULL;
		}
		CWaveFrame *pWaveFrame = (CWaveFrame*)pDataFrame;
		m_pWaveData=(LPWAVEHDR)malloc(sizeof(WAVEHDR));
		memset(m_pWaveData, 0, sizeof(WAVEHDR));
		m_pWaveData->lpData=(char*)malloc(pWaveFrame->data->dwBufferLength);
		memcpy(m_pWaveData->lpData, pWaveFrame->data->lpData, pWaveFrame->data->dwBufferLength);
		m_nChannels=pWaveFrame->waveformat->nChannels;
		m_wBitsPerSample=pWaveFrame->waveformat->wBitsPerSample;
		m_pWaveData->dwBufferLength = pWaveFrame->data->dwBufferLength;
		m_pWaveData->dwBytesRecorded = pWaveFrame->data->dwBytesRecorded;
		m_pWaveData->dwUser = pWaveFrame->data->dwUser;
		m_pWaveData->dwFlags = pWaveFrame->data->dwFlags;
		m_pWaveData->dwLoops = pWaveFrame->data->dwLoops;
	}
	m_Busy.Unlock();
}

void CDataFrameView::OnPaint() 
{
	 CPaintDC dc(this); 
    m_Busy.Lock();
	 GetClientRect(&m_RC);
	 dc.FillSolidRect(&m_RC, RGB(0,0,15));

	 if (Tvdb400_TypesCompatible(m_Mode, wave))
	 {
		 if (!m_pWaveData)
		 {
			 m_Busy.Unlock(); 
          return; 
		 }
		 if (!GetSize())
		 { 
			 m_Busy.Unlock(); 
          return; 
		 }
    
		 if ((m_ScaleAuto) && (GetSize()))
		 {
          m_min=GetAt(0);
          m_max=GetAt(0);
		 }
       int i, j;
		 double avg;
       for (i=0; i<GetSize(); i++)
		 {
          double data=GetAt(i);
          if (data<m_min) m_min=data;
          if (data>m_max) m_max=data;
		 }
		 int gapx=(int)(GetSize()/((m_RC.right-m_RC.left)));    
		 m_scalex=((double)(m_RC.right-m_RC.left))/(GetSize());
		 m_scaley=((double)(m_RC.bottom-m_RC.top))/(m_max-m_min);
		 CPen *oPen;
       oPen=(CPen*)dc.SelectObject(&m_Pen);
		 dc.MoveTo(0,m_RC.bottom+(int)(m_min*m_scaley+0.5));

		 gapx=1;//TODO

		 for (i=0; i<GetSize(); i+=gapx)
		 {
			 avg=0;
			 for(j=0; j<gapx; j++)
				 avg+=GetAt(i+j);
			 avg/=gapx;
			 dc.SetPixelV((int)(i*m_scalex+0.5),m_RC.bottom-(int)((avg-m_min)*m_scaley+0.5),RGB(0,255,0));
//			 dc.LineTo((int)(i*m_scalex+0.5),m_RC.bottom-(int)((avg-m_min)*m_scaley+0.5));
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
        dc.TextOut(10,10,(LPCTSTR)tmpS);
		 }
       dc.SelectObject(oPen);
	 }
    m_Busy.Unlock();
}

void CDataFrameView::OnLButtonDown(UINT nFlags, CPoint point) 
{
    m_MousePnt=point;
    Invalidate();
	 CWnd::OnLButtonDown(nFlags, point);
}

void CDataFrameView::OnMouseMove(UINT nFlags, CPoint point) 
{
    if (nFlags & MK_LBUTTON)
    {
        m_MousePnt=point;
        Invalidate();
    }
	CWnd::OnMouseMove(nFlags, point);
}

CPoint CDataFrameView::GetMousePnt()     
{ 
    CPoint pnt; 
    if (m_MousePnt.x>=0) 
    {
        pnt.x=(int)(m_MousePnt.x/m_scalex);
        pnt.y=(int)((m_RC.bottom-m_MousePnt.y)/m_scaley);
    } 
    return pnt;
}

