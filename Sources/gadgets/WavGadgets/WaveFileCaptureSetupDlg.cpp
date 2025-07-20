// WaveFileCaptureSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "wavgadgets.h"
#include "WaveFileCaptureSetupDlg.h"
#include "WavGadgetsImpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "CWaveFileCaptureSetupDlg"

/////////////////////////////////////////////////////////////////////////////
// CWaveFileCaptureSetupDlg dialog

CWaveFileCaptureSetupDlg::CWaveFileCaptureSetupDlg(CGadget* pGadget, CWnd* pParent):
        CGadgetSetupDialog(pGadget,CWaveFileCaptureSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWaveFileCaptureSetupDlg)
	m_Filename = _T("");
	m_bLoop = FALSE;
	//}}AFX_DATA_INIT
}

void CWaveFileCaptureSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CGadgetSetupDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWaveFileCaptureSetupDlg)
	DDX_Text(pDX, IDC_FILENAME, m_Filename);
	DDX_Check(pDX, IDC_LOOP, m_bLoop);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWaveFileCaptureSetupDlg, CGadgetSetupDialog)
	//{{AFX_MSG_MAP(CWaveFileCaptureSetupDlg)
	ON_BN_CLICKED(IDC_BROWSE_FILENAME, OnBrowseFilename)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaveFileCaptureSetupDlg message handlers
bool CWaveFileCaptureSetupDlg::Show(CPoint point, LPCTSTR uid)
{
    FXString DlgHead;
    DlgHead.Format("%s Setup Dialog", uid);
    if (!m_hWnd)
    {
        FX_UPDATERESOURCE fur(pThisDll->m_hResource);
        if (!Create(IDD_WAVSRCSETUPDLG, NULL))
        {
            SENDERR_0("Failed to create Setup Dialog");
            return false;
        }
    }
    SetWindowText(DlgHead);
    SetWindowPos(NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    ShowWindow(SW_SHOWNORMAL);

    WavFileCapture* Gadget = (WavFileCapture*)m_pGadget;
	m_Filename  = Gadget->m_fName ;
	m_bLoop     = Gadget->m_bLoop;
    UpdateData(FALSE);
    return true;
}

void CWaveFileCaptureSetupDlg::UploadParams()
{
	WavFileCapture* Gadget = (WavFileCapture*)m_pGadget;
	Gadget->m_fName = m_Filename;
	Gadget->m_bLoop = m_bLoop;
	Gadget->OpenWavFile();
    CGadgetSetupDialog::UploadParams();
}

void CWaveFileCaptureSetupDlg::OnBrowseFilename() 
{
	CFileDialog fd(TRUE, "wav", m_Filename, OFN_HIDEREADONLY, "Wave files (*.wav)|*.wav|all files (*.*)|*.*||", this);
	if (fd.DoModal() == IDOK)
	{
		m_Filename = fd.GetPathName();
		GetDlgItem(IDC_FILENAME)->SetWindowText(m_Filename);
	}
}
