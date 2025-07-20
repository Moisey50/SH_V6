// WaveSpeakerSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "wavgadgets.h"
#include "WaveSpeakerSetupDlg.h"
#include "WavGadgetsImpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// WaveSpeakerSetupDlg dialog


WaveSpeakerSetupDlg::WaveSpeakerSetupDlg(CGadget* pGadget, CWnd* pParent):
CGadgetSetupDialog(pGadget,WaveSpeakerSetupDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(WaveSpeakerSetupDlg)
    m_ComboIndex = -1;
    //}}AFX_DATA_INIT
}

void WaveSpeakerSetupDlg::DoDataExchange(CDataExchange* pDX)
{
    CGadgetSetupDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(WaveSpeakerSetupDlg)
    DDX_CBIndex(pDX, IDC_SPEAKERCOMBO, m_ComboIndex);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(WaveSpeakerSetupDlg, CGadgetSetupDialog)
    //{{AFX_MSG_MAP(WaveSpeakerSetupDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// WaveSpeakerSetupDlg message handlers

BOOL WaveSpeakerSetupDlg::OnInitDialog() 
{
    CGadgetSetupDialog::OnInitDialog();

    WAVEOUTCAPS OutDevCaps;
    CComboBox* pBox;
    pBox = (CComboBox*)GetDlgItem(IDC_SPEAKERCOMBO);

    //	 TRACE("%d",waveOutGetNumDevs());
    for (unsigned int j=0; j<waveOutGetNumDevs(); j++)
    {
        waveOutGetDevCaps(j, &OutDevCaps, sizeof(WAVEOUTCAPS));
        pBox->AddString(OutDevCaps.szPname);
    }
    pBox->SetCurSel(m_ComboIndex);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void WaveSpeakerSetupDlg::UploadParams()
{
    //WaveSpeaker* Gadget = (WaveSpeaker*)m_pBuilder->GetGadget(m_UID);
    WaveSpeaker* Gadget = (WaveSpeaker*)m_pGadget;
    Gadget->m_wMCIDeviceID = m_ComboIndex;
    CGadgetSetupDialog::UploadParams();
}

