//  $File : HistogramView.cpp : implementation file
//  (C) Copyright HomeBuilt Group and File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)

#include "stdafx.h"
#include "tuner.h"
#include "HistogramView.h"
#include <imageproc\statistics.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHistogramView dialog


CHistogramView::CHistogramView(CWnd* pParent /*=NULL*/)
	: CDispDlg(CHistogramView::IDD, pParent)
{
    m_Frame.lpBMIH=NULL;	
    m_Frame.lpData=NULL;
	//{{AFX_DATA_INIT(CHistogramView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CHistogramView::DoDataExchange(CDataExchange* pDX)
{
	CDispDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHistogramView)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHistogramView, CDispDlg)
	//{{AFX_MSG_MAP(CHistogramView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHistogramView message handlers

BOOL CHistogramView::OnInitDialog() 
{
	CDispDlg::OnInitDialog();
	m_Display.Create(GetDlgItem(IDC_DISPVIEW));
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

bool CHistogramView::LoadDIB(BITMAPINFOHEADER *bmih)
{
    m_Frame.lpBMIH=bmih;
    RECT rc={0,0,bmih->biWidth,bmih->biHeight};  
    GetHistogram(&m_Frame, rc, m_Display.GetData());
    m_Display.Invalidate();
    return true;
}

