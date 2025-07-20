//  $File : DiffrerenceView.cpp : implementation file
//  (C) Copyright HomeBuilt Group and File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#include "stdafx.h"
#include "tuner.h"
#include "DiffrerenceView.h"
#include <imageproc\simpleip.h>
#include <imageproc\motiondetectors.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDiffrerenceView dialog


CDiffrerenceView::CDiffrerenceView(CWnd* pParent /*=NULL*/)
	: CDispDlg(CDiffrerenceView::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDiffrerenceView)
	m_Value = _T("");
	//}}AFX_DATA_INIT
}


void CDiffrerenceView::DoDataExchange(CDataExchange* pDX)
{
	CDispDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDiffrerenceView)
	DDX_Control(pDX, IDC_LEVELVIEW2, m_LevelView2);
	DDX_Control(pDX, IDC_LEVELVIEW, m_LevelView);
	DDX_Text(pDX, IDC_VALUE, m_Value);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDiffrerenceView, CDispDlg)
	//{{AFX_MSG_MAP(CDiffrerenceView)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiffrerenceView message handlers

BOOL CDiffrerenceView::OnInitDialog() 
{
	CDispDlg::OnInitDialog();
	m_Display.Create(GetDlgItem(IDC_DISPVIEW));
    m_Display.SetScale(-1);
    m_LevelView.SetRange(0,100);
    m_LevelView.SetStep(1);

    m_LevelView2.SetRange(0,100);
    m_LevelView2.SetStep(1);

    m_TimeLine.Create(GetDlgItem(IDC_TIMELINE));
    m_TimeLine.SetMaxDataNmb(200);
    m_TimeLine.SetMinMaxData(0,10);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDiffrerenceView::OnDestroy() 
{
	CDispDlg::OnDestroy();
}

bool CDiffrerenceView::LoadDIB(BITMAPINFOHEADER *bmih)
{
    pTVFrame oldOne=makecopyTVFrame(GetFrameStored());
    pTVFrame newOne=makeTVFrame(bmih);

    double level = GetMotionValue(newOne);

    freeTVFrame(newOne);
    newOne=GetFrameStored();

    m_Value.Format("%4.2f",level);
    m_TimeLine.Add(level);
    m_TimeLine.Invalidate();
    m_LevelView.SetPos((int)(level*10));
    
    level=0;

    if (oldOne) level=compareframes_max(oldOne, newOne);
    m_LevelView2.SetPos((int)(level*10));

    pTVFrame  difFrame = NULL;
    if (oldOne) difFrame = compareframes2(newOne, oldOne);
    m_Display.LoadFrame(difFrame);

    freeTVFrame(difFrame);
    freeTVFrame(oldOne);
    UpdateData(FALSE);
    return true;
}

