// AviCaptureDialog.cpp : implementation file
//

#include "stdafx.h"
#include "TVAvi.h"
#include "AviCaptureDialog.h"
#include "AviCaptureGadget.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "AviCaptureSetupDialog"

////
INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData) 
{
    AviCaptureDialog *acd=(AviCaptureDialog*)pData;
    switch(uMsg) 
    {
    case BFFM_INITIALIZED: 
        {
            FXString apppath=acd->GetFileName();
            //FXString apppath=FxGetAppPath();
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)(LPCTSTR)apppath);
            break;
        }
    case BFFM_SELCHANGED: 
        {
            TCHAR szDir[MAX_PATH];
            if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
                SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
            break;
        }
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// AviCaptureDialog dialog

AviCaptureDialog::AviCaptureDialog(CGadget* pGadget, CWnd* pParent):
        CGadgetSetupDialog(pGadget, AviCaptureDialog::IDD, pParent),
        m_ReservePins(FALSE),
        m_ReservePinsNumber(0)
        , m_ChunkMode(FALSE)
{
	//{{AFX_DATA_INIT(AviCaptureDialog)
	m_LoopFilm = FALSE;
	//}}AFX_DATA_INIT
	FXString text;
	m_pGadget->PrintProperties(text);
	FXPropertyKit pk(text);
    FXString tmpS;
	if (!pk.GetString("FileName", tmpS))
		m_FileName = "";
    else
        m_FileName=tmpS;
	if (!pk.GetInt("FrameRate", (int&)m_FrameRate))
		m_FrameRate = 10;
	if (!pk.GetBool("SWTrigger", m_bSoftwareTrigger))
		m_bSoftwareTrigger = FALSE;
	if (!pk.GetBool("ChunkMode", m_ChunkMode))
		m_ChunkMode = FALSE;
    pk.GetBool("LoopFilm", m_LoopFilm);
}

void AviCaptureDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    CString tmpFN=m_FileName;
    //{{AFX_DATA_MAP(AviCaptureDialog)
    DDX_Text(pDX, IDC_FILENAME, tmpFN);
    DDX_Check(pDX, IDC_SOFTWARE_TRIGGER, m_bSoftwareTrigger);
    DDX_Text(pDX, IDC_FRAMERATE, m_FrameRate);
    DDX_Check(pDX, IDC_LOOP, m_LoopFilm);
    //}}AFX_DATA_MAP
    DDX_Check(pDX, IDC_RESERVE_OUTPUTS, m_ReservePins);
    DDX_Text(pDX, IDC_PINS_NMB, m_ReservePinsNumber);
	DDV_MinMaxInt(pDX, m_ReservePinsNumber, 1, 10);
    m_FileName=tmpFN;
    DDX_Check(pDX, IDC_CHANKMODE, m_ChunkMode);
}


BEGIN_MESSAGE_MAP(AviCaptureDialog, CDialog)
	//{{AFX_MSG_MAP(AviCaptureDialog)
	ON_BN_CLICKED(IDC_BROWSE_FILENAME, OnBrowseFilename)
	ON_BN_CLICKED(IDC_SOFTWARE_TRIGGER, OnSoftwareTrigger)
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_RESERVE_OUTPUTS, &AviCaptureDialog::OnBnClickedReserveOutputs)
    ON_BN_CLICKED(IDC_CHANKMODE, &AviCaptureDialog::OnBnClickedChankmode)
END_MESSAGE_MAP()

void AviCaptureDialog::UploadParams()
{
	FXPropertyKit pk;
    bool Invalidate=false;
	pk.WriteInt("FrameRate", (int)m_FrameRate);
	pk.WriteString("FileName", m_FileName);
    pk.WriteBool("LoopFilm", m_LoopFilm);
    pk.WriteBool("ReserveOutputs", m_ReservePins);
    pk.WriteBool("ReserveOutputs", m_ReservePins);
    pk.WriteInt("OutputsNumber",m_ReservePinsNumber);
    pk.WriteBool("ChunkMode",m_ChunkMode);
    m_pGadget->ScanProperties(pk, Invalidate);
    CGadgetSetupDialog::UploadParams();
}

/////////////////////////////////////////////////////////////////////////////
// AviCaptureDialog message handlers

bool AviCaptureDialog::Show(CPoint point, LPCTSTR uid)
{
    FXString DlgHead;
    
    DlgHead.Format("%s Setup Dialog", uid);
    if (!m_hWnd)
    {
        FX_UPDATERESOURCE fur(pThisDll->m_hResource);
        if (!Create(IDD_AVI_CAPTURE_DLG, NULL))
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

void AviCaptureDialog::OnBrowseFilename() 
{
    UpdateData(TRUE);
    if (m_ChunkMode)
    {
        BROWSEINFO bi;
        TCHAR szDir[MAX_PATH];
        LPITEMIDLIST pidl;

        CoInitialize(NULL);
        ZeroMemory(&bi,sizeof(bi));

        bi.hwndOwner=this->m_hWnd;
        bi.pidlRoot=NULL;
        bi.lpszTitle=_T("Select directory with chunk avi files");
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_USENEWUI;
        bi.lpfn = BrowseCallbackProc;
        bi.lParam=(LPARAM)this;
        pidl = SHBrowseForFolder(&bi);
        if (pidl)
        {
            if (SHGetPathFromIDList(pidl,szDir))
                //SetRootPath(szDir);
                m_FileName=szDir;
                if ((m_FileName.GetLength()) && (m_FileName[m_FileName.GetLength()-1]!='\\'))
                    m_FileName+='\\';
                GetDlgItem(IDC_FILENAME)->SetWindowText(m_FileName);
        }
        CoTaskMemFree(pidl);
        CoUninitialize();
    }
    else
    {
	    CFileDialog fd(TRUE, "avi", m_FileName, OFN_FILEMUSTEXIST, "videos (*.avi)|*.avi|all files (*.*)|*.*||", this);
	    if (fd.DoModal() == IDOK)
	    {
		    m_FileName = fd.GetPathName();
		    GetDlgItem(IDC_FILENAME)->SetWindowText(m_FileName);
	    }
    }
}

BOOL AviCaptureDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	FXString text;
    m_pGadget->PrintProperties(text);
	FXPropertyKit pk(text);
    FXString tmpS;
	if (!pk.GetString("FileName", tmpS))
		m_FileName = "";
    else
        m_FileName=tmpS;
	if (!pk.GetInt("FrameRate", (int&)m_FrameRate))
		m_FrameRate = 10;
	if (!pk.GetBool("SWTrigger", m_bSoftwareTrigger))
		m_bSoftwareTrigger = FALSE;
    pk.GetBool("LoopFilm", m_LoopFilm);
    pk.GetBool("ReserveOutputs", m_ReservePins);
    pk.GetInt("OutputsNumber",m_ReservePinsNumber);
    pk.GetBool("ChunkMode",m_ChunkMode);
    if (m_ChunkMode)
        m_FileName=FxExtractPath(m_FileName);
	GetDlgItem(IDC_FRAMERATE)->EnableWindow(!m_bSoftwareTrigger);
    GetDlgItem(IDC_PINS_NMB)->EnableWindow(m_ReservePins);

    UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void AviCaptureDialog::OnSoftwareTrigger() 
{
	m_bSoftwareTrigger = !m_bSoftwareTrigger;
	FXPropertyKit pk;
    bool Invalidate=false;
	pk.WriteBool("SWTrigger", m_bSoftwareTrigger);
    m_pGadget->ScanProperties(pk, Invalidate);
	((CButton*)GetDlgItem(IDC_SOFTWARE_TRIGGER))->SetCheck((m_bSoftwareTrigger ? MF_CHECKED : MF_UNCHECKED));
	GetDlgItem(IDC_FRAMERATE)->EnableWindow(!m_bSoftwareTrigger);
}

void AviCaptureDialog::OnBnClickedReserveOutputs()
{
    UpdateData(TRUE);
    GetDlgItem(IDC_PINS_NMB)->EnableWindow(m_ReservePins);
}

void AviCaptureDialog::OnBnClickedChankmode()
{
    UpdateData(TRUE);
    if (m_ChunkMode)
        m_FileName=FxExtractPath(m_FileName);
    else
        m_FileName="";
    UpdateData(FALSE);
}

FXString AviCaptureDialog::GetFileName() 
{ 
    if (m_ChunkMode)
        return FxExtractPath(m_FileName);
    return m_FileName; 
}
