// ComplexGadgetDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ComplexGadgetDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ComplexDlg dialog


ComplexDlg::ComplexDlg(IGraphbuilder* pBuilder, FXString& uid, CWnd* pParent /*=NULL*/)
//	: CGadgetSetupDialog(pBuilder, uid, ComplexDlg::IDD, pParent),
    : CGadgetSetupDialog(pBuilder->GetGadget(uid),ComplexDlg::IDD,pParent),
	m_Cmd(CMD_NONE)
{
	//{{AFX_DATA_INIT(ComplexDlg)
	CGadget* Gadget = pBuilder->GetGadget(uid);
	m_LoadPath = ((Complex*)Gadget)->m_LoadPath;
	//}}AFX_DATA_INIT
}


void ComplexDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ComplexDlg)
	DDX_Text(pDX, IDC_PATH, m_LoadPath);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ComplexDlg, CDialog)
	//{{AFX_MSG_MAP(ComplexDlg)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_BN_CLICKED(IDC_LOAD, OnLoad)
	ON_BN_CLICKED(IDC_INCORPORATE, OnIncorporate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void ComplexDlg::UploadParams(CGadget* Gadget)
{
    bool Invalidate=false;
	UpdateData(TRUE);
	if (((Complex*)Gadget)->m_LoadPath == (LPCTSTR)m_LoadPath)
		return;
	switch (m_Cmd)
	{
	case CMD_NONE:
		return;
	case CMD_LOAD:
		((Complex*)Gadget)->ScanProperties(m_LoadPath,Invalidate);
		break;
	case CMD_SAVE:
		((Complex*)Gadget)->SetLoadPath(m_LoadPath);
		if (!m_LoadPath.IsEmpty())
			((Complex*)Gadget)->Builder()->Save(m_LoadPath);
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////
// ComplexDlg message handlers

void ComplexDlg::OnBrowse() 
{
	CFileDialog fd(TRUE, "tvg", m_LoadPath, 0, "graphs (*.tvg)|*.tvg|all files (*.*)|*.*||");
	if (fd.DoModal() == IDOK)
	{
		m_LoadPath = fd.GetPathName();
		UpdateData(FALSE);
	}
}

void ComplexDlg::OnSave() 
{
	m_Cmd = CMD_SAVE;
	OnOK();
}

void ComplexDlg::OnLoad() 
{
	m_Cmd = CMD_LOAD;
	OnOK();
}

void ComplexDlg::OnIncorporate() 
{
	m_LoadPath.Empty();
	m_Cmd = CMD_SAVE;
	OnOK();
}
