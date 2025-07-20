// VideoCutRectDialog.cpp : implementation file
//

#include "stdafx.h"
#include "TVVideo.h"
#include "VideoCutRectDialog.h"
#include "VideoFilters.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "VideoCutRectDialog"

inline DWORD GetCompression( pTVFrame fr )
{
  if ( fr->lpBMIH )
    return fr->lpBMIH->biCompression ;
  else
    return 0xffffffff ;
}

/////////////////////////////////////////////////////////////////////////////
// VideoCutRectDialog dialog


VideoCutRectDialog::VideoCutRectDialog(CGadget* pGadget, CWnd* pParent):
        CGadgetSetupDialog(pGadget, VideoCutRectDialog::IDD, pParent),
        m_b4PixelsStep(FALSE)
        , m_bAllowROIControlFromVideoInput(FALSE)
{
	//{{AFX_DATA_INIT(VideoCutRectDialog)
	m_Left = 0;
	m_Top = 0;
	m_Width = 50;
	m_Height = 50;
	//}}AFX_DATA_INIT
}

void VideoCutRectDialog::LoadFrame(const pTVFrame frame)
{
	if (!IsWindowVisible())
		return;
	m_View.LoadFrame(frame);
}

void VideoCutRectDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(VideoCutRectDialog)
  DDX_Text(pDX, IDC_LEFT, m_Left);
  DDX_Text(pDX, IDC_TOP, m_Top);
  DDX_Text(pDX, IDC_WIDTH, m_Width);
  DDX_Text(pDX, IDC_HEIGHT, m_Height);
  //}}AFX_DATA_MAP
  DDX_Check(pDX, IDC_4PIXELS_ROUNDING, m_b4PixelsStep);
  DDX_Check(pDX, IDC_ALLOW_CONTROL_FROM_INPUT, m_bAllowROIControlFromVideoInput);
}


BEGIN_MESSAGE_MAP(VideoCutRectDialog, CGadgetSetupDialog)
	//{{AFX_MSG_MAP(VideoCutRectDialog)
	ON_EN_CHANGE(IDC_HEIGHT, OnChangeHeight)
	ON_EN_CHANGE(IDC_LEFT, OnChangeLeft)
	ON_EN_CHANGE(IDC_TOP, OnChangeTop)
	ON_EN_CHANGE(IDC_WIDTH, OnChangeWidth)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_4PIXELS_ROUNDING, &VideoCutRectDialog::OnBnClicked4pixelsRounding)
    ON_BN_CLICKED(IDC_ALLOW_CONTROL_FROM_INPUT, &VideoCutRectDialog::OnBnClickedAllowControlFromInput)
END_MESSAGE_MAP()

void VideoCutRectDialog::UploadParams()
{
	{
        FXPropertyKit pc;
        bool Invalidate=false;
        pc.WriteInt("Offset.x",m_Left);
        pc.WriteInt("Offset.y",m_Top);
        pc.WriteInt("Width",   m_Width);
        pc.WriteInt("Height",  m_Height);
        pc.WriteInt("4PixStep" , m_b4PixelsStep ) ;
        pc.WriteInt("AllowROIInput" , m_bAllowROIControlFromVideoInput ) ;
        m_pGadget->ScanProperties(pc,Invalidate);
        CGadgetSetupDialog::UploadParams();
	}
}

/////////////////////////////////////////////////////////////////////////////
// VideoCutRectDialog message handlers
bool VideoCutRectDialog::Show(CPoint point, LPCTSTR uid)
{
    FXString DlgHead;
    DlgHead.Format("%s Setup Dialog", uid);
    if (!m_hWnd)
    {
        FX_UPDATERESOURCE fur(pThisDll->m_hResource);
        if (!Create(IDD_VIDEO_CUTRECT_DIALOG, NULL))
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
BOOL VideoCutRectDialog::OnInitDialog() 
{
	CGadgetSetupDialog::OnInitDialog();
	if (m_pGadget)
	{
    FXPropertyKit pc;
        m_pGadget->PrintProperties(pc);
    pc.GetInt("Offset.x",m_Left);
    pc.GetInt("Offset.y",m_Top);
    pc.GetInt("Width",   m_Width);
    pc.GetInt("Height",  m_Height);
    pc.GetInt("4PixStep" , m_b4PixelsStep ) ;
    pc.GetInt("AllowROIInput" , m_bAllowROIControlFromVideoInput ) ;
	}
  UpdateData(FALSE);
	m_View.Create(GetDlgItem(IDC_VIEW));
	m_View.SetScale(-1);
	CPoint pt(m_Left, m_Top);
	CSize sz(m_Width, m_Height);
	CRect rc(pt, sz);
	((CDibVSq*)&m_View)->AddSquare(rc, RGB(255, 0, 0));
    OnRectChanged(&rc);
	m_View.SetClient(this);

	return TRUE;
}

void VideoCutRectDialog::OnDestroy() 
{
    m_View.DestroyWindow();
	CGadgetSetupDialog::OnDestroy();
}

void VideoCutRectDialog::OnRectChanged(CRect* rect)
{
	m_Left = rect->left;
	m_Top = rect->top;
	m_Width = rect->Width();
	m_Height = rect->Height();
	m_View.GetSquare(0)->SetRect(*rect);
	UpdateData(FALSE);
  UploadParams();
}

void VideoCutRectDialog::OnChangeHeight() 
{
	UpdateData(TRUE);
	CPoint pt(m_Left, m_Top);
	CSize sz(m_Width, m_Height);
	CRect rect(pt, sz);
	m_View.GetSquare(0)->SetRect(rect);
}

void VideoCutRectDialog::OnChangeLeft() 
{
	UpdateData(TRUE);
	CPoint pt(m_Left, m_Top);
	CSize sz(m_Width, m_Height);
	CRect rect(pt, sz);
	m_View.GetSquare(0)->SetRect(rect);
}

void VideoCutRectDialog::OnChangeTop() 
{
	UpdateData(TRUE);
	CPoint pt(m_Left, m_Top);
	CSize sz(m_Width, m_Height);
	CRect rect(pt, sz);
	m_View.GetSquare(0)->SetRect(rect);
}

void VideoCutRectDialog::OnChangeWidth() 
{
	UpdateData(TRUE);
	CPoint pt(m_Left, m_Top);
	CSize sz(m_Width, m_Height);
	CRect rect(pt, sz);
	m_View.GetSquare(0)->SetRect(rect);
}

void VideoCutRectDialog::OnSize(UINT nType, int cx, int cy) 
{
	CGadgetSetupDialog::OnSize(nType, cx, cy);
	if (::IsWindow(m_View.GetSafeHwnd()))
	{
		CWnd* Frame = GetDlgItem(IDC_VIEW);
		CRect rc;
		Frame->GetWindowRect(rc);
		ScreenToClient(rc);
		rc.right = cx - rc.left;
		rc.bottom = cy - rc.left;
		Frame->MoveWindow(rc);
		Frame->GetClientRect(rc);
		m_View.MoveWindow(rc);
	}
}


void VideoCutRectDialog::OnBnClicked4pixelsRounding()
{
    UpdateData(TRUE);
}


void VideoCutRectDialog::OnBnClickedAllowControlFromInput()
{
  UpdateData(TRUE);
}
