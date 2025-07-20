// LPTSetup.cpp : implementation file
//

#include "stdafx.h"
#include "tvgeneric.h"
#include "LPTSetup.h"
#include "LPTBitRenderer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "CLPTSetup"

/////////////////////////////////////////////////////////////////////////////
// CLPTSetup dialog


CLPTSetup::CLPTSetup(CGadget* pGadget, CWnd* pParent):
    CGadgetSetupDialog(pGadget, CLPTSetup::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLPTSetup)
	//}}AFX_DATA_INIT
}

void CLPTSetup::DoDataExchange(CDataExchange* pDX)
{
	CGadgetSetupDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLPTSetup)
	DDX_Control(pDX, IDC_LPTPORT, m_LptCombo);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLPTSetup, CGadgetSetupDialog)
	//{{AFX_MSG_MAP(CLPTSetup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLPTSetup message handlers

bool CLPTSetup::Show(CPoint point, LPCTSTR uid)
{
    FXString DlgHead;
    DlgHead.Format("%s Setup Dialog", uid);
    if (!m_hWnd)
    {
        FX_UPDATERESOURCE fur(pThisDll->m_hResource);
        if (!Create(IDD_LPTSETTUPDIALOG, NULL))
        {
            SENDERR_0("Failed to create Setup Dialog");
            return false;
        }
    }
    SetWindowText(DlgHead);
    SetWindowPos(NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    ShowWindow(SW_SHOWNORMAL);
    return true;
}

BOOL CLPTSetup::OnInitDialog() 
{
	CGadgetSetupDialog::OnInitDialog();
	FXPropertyKit pc;
    m_pGadget->PrintProperties(pc);
    int port=0;
    pc.GetInt("LPTPORT",port);
	m_LptCombo.SetCurSel(port);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLPTSetup::UploadParams()
{
    bool Invalidate=false;
    UpdateData(TRUE);
    FXPropertyKit pc;
    int pos=m_LptCombo.GetCurSel();
    if (pos!=CB_ERR)
        pc.WriteInt("LPTPORT",pos);
    m_pGadget->ScanProperties(pc, Invalidate);
	CGadgetSetupDialog::UploadParams();
}

