//  $File : FeatureDetectorDsp.cpp : implementation file
//  (C) Copyright HomeBuilt Group and File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#include "stdafx.h"
#include "tuner.h"
#include "FeatureDetectorDsp.h"
#include <imageproc\simpleIP.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFeatureDetectorDsp dialog


CFeatureDetectorDsp::CFeatureDetectorDsp(CWnd* pParent /*=NULL*/)
	: CDispDlg(CFeatureDetectorDsp::IDD, pParent)
{
    m_Frame.lpBMIH=NULL;	
    m_Frame.lpData=NULL;
    //{{AFX_DATA_INIT(CFeatureDetectorDsp)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CFeatureDetectorDsp::DoDataExchange(CDataExchange* pDX)
{
	CDispDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFeatureDetectorDsp)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFeatureDetectorDsp, CDispDlg)
	//{{AFX_MSG_MAP(CFeatureDetectorDsp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFeatureDetectorDsp message handlers

bool CFeatureDetectorDsp::LoadDIB(BITMAPINFOHEADER *bmih)
{
    pTVFrame pfr=makeTVFrame(bmih);
    
    if (false) //(seekSpot(pfr, rc, 0))
        m_Display.LoadFrame(pfr);
    else
        m_Display.LoadFrame(pfr);
    freeTVFrame(pfr);
    return true;
}

BOOL CFeatureDetectorDsp::OnInitDialog() 
{
	CDispDlg::OnInitDialog();
	m_Display.Create(GetDlgItem(IDC_FEATUREDISPLAY));
    m_Display.SetScale(-1);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
