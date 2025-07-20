// PolySetupDialog.cpp : implementation file
//

#include "stdafx.h"
#include "PolySetupDialog.h"

#define THIS_MODULENAME "CPolySetupDialog"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPolySetupDialog dialog


CPolySetupDialog::CPolySetupDialog(CGadget* pGadget, CWnd* pParent /*=NULL*/)
	: CGadgetSetupDialog(pGadget, CPolySetupDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPolySetupDialog)
	m_ax = 0.0;
	m_ay = 0.0;
	m_bx = 0.0;
	m_by = 0.0;
	m_cx = 0.0;
	m_cy = 0.0;
	m_dx = 0.0;
	m_dy = 0.0;
	m_ex = 0.0;
	m_ey = 0.0;
	m_fx = 0.0;
	m_fy = 0.0;
	m_xC = 0.0;
	m_yC = 0.0;
	//}}AFX_DATA_INIT
	FXString text;
	pGadget->PrintProperties(text);
	FXPropertyKit pk(text);
	FXString val;
	if (pk.GetString("x0", val))
		m_ax = atof(val);
	if (pk.GetString("x1", val))
		m_bx = atof(val);
	if (pk.GetString("x2", val))
		m_cx = atof(val);
	if (pk.GetString("x3", val))
		m_dx = atof(val);
	if (pk.GetString("x4", val))
		m_ex = atof(val);
	if (pk.GetString("x5", val))
		m_fx = atof(val);
	if (pk.GetString("y0", val))
		m_ay = atof(val);
	if (pk.GetString("y1", val))
		m_by = atof(val);
	if (pk.GetString("y2", val))
		m_cy = atof(val);
	if (pk.GetString("y3", val))
		m_dy = atof(val);
	if (pk.GetString("y4", val))
		m_ey = atof(val);
	if (pk.GetString("y5", val))
		m_fy = atof(val);
	if (pk.GetString("xC", val))
		m_xC = atof(val);
	if (pk.GetString("yC", val))
		m_yC = atof(val);
}


void CPolySetupDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPolySetupDialog)
	DDX_Text(pDX, IDC_AX, m_ax);
	DDX_Text(pDX, IDC_AY, m_ay);
	DDX_Text(pDX, IDC_BX, m_bx);
	DDX_Text(pDX, IDC_BY, m_by);
	DDX_Text(pDX, IDC_CX, m_cx);
	DDX_Text(pDX, IDC_CY, m_cy);
	DDX_Text(pDX, IDC_DX, m_dx);
	DDX_Text(pDX, IDC_DY, m_dy);
	DDX_Text(pDX, IDC_EX, m_ex);
	DDX_Text(pDX, IDC_EY, m_ey);
	DDX_Text(pDX, IDC_FX, m_fx);
	DDX_Text(pDX, IDC_FY, m_fy);
	DDX_Text(pDX, IDC_XC, m_xC);
	DDX_Text(pDX, IDC_YC, m_yC);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPolySetupDialog, CDialog)
	//{{AFX_MSG_MAP(CPolySetupDialog)
	ON_BN_CLICKED(IDC_LOAD, OnLoad)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPolySetupDialog::UploadParams(CGadget* Gadget)
{
	FXPropertyKit pk;
	FXString text;
    bool Invalidate=false;
	text.Format("%f", m_ax);
	pk.WriteString("x0", text);
	text.Format("%f", m_bx);
	pk.WriteString("x1", text);
	text.Format("%f", m_cx);
	pk.WriteString("x2", text);
	text.Format("%f", m_dx);
	pk.WriteString("x3", text);
	text.Format("%f", m_ex);
	pk.WriteString("x4", text);
	text.Format("%f", m_fx);
	pk.WriteString("x5", text);
	text.Format("%f", m_ay);
	pk.WriteString("y0", text);
	text.Format("%f", m_by);
	pk.WriteString("y1", text);
	text.Format("%f", m_cy);
	pk.WriteString("y2", text);
	text.Format("%f", m_dy);
	pk.WriteString("y3", text);
	text.Format("%f", m_ey);
	pk.WriteString("y4", text);
	text.Format("%f", m_fy);
	pk.WriteString("y5", text);
	text.Format("%f", m_xC);
	pk.WriteString("xC", text);
	text.Format("%f", m_yC);
	pk.WriteString("yC", text);
	Gadget->ScanProperties(pk, Invalidate);
}

/////////////////////////////////////////////////////////////////////////////
// CPolySetupDialog message handlers

void CPolySetupDialog::OnLoad() 
{
	CFileDialog fd(TRUE, "txt", "calibration.txt", OFN_FILEMUSTEXIST, "txt files (*.txt)|*.txt|all files (*.*)|*.*||");
	if (fd.DoModal() == IDOK)
	{
		FILE* file = fopen(fd.GetPathName(), "r");
		if (!file)
		{
			SENDERR_0("Failed to open calibration file");
			return;
		}
		char buf[80];
		fseek(file, 0, SEEK_END);
		int size = ftell(file);
		if (!size)
		{
			SENDERR_0("Failed to parse calibration file");
			return;
		}
		fseek(file, 0, SEEK_SET);
		int i = fscanf(file, "%s%s%lf%lf %lf %lf %lf %lf\r\n%s%s %lf %lf %lf %lf %lf %lf\r\n%s%s %lf\r\n%s%s %lf",
			buf, buf, &m_ax, &m_bx, &m_cx, &m_dx, &m_ex, &m_fx, buf, buf, &m_ay, &m_by, &m_cy, &m_dy, &m_ey, &m_fy, buf, buf, &m_xC, buf, buf, &m_yC);
		fclose(file);
		UpdateData(FALSE);
	}
}
bool CPolySetupDialog::Show(CPoint point, LPCTSTR uid)
{
  HINSTANCE hOldResource=AfxGetResourceHandle();
  AfxSetResourceHandle(pThisDll->m_hResource);

  FXString DlgHead;
  DlgHead.Format("%s Setup Dialog", uid);
  if (!m_hWnd)
  {
    if (!Create(IDD_POLY_SETUP_DLG, NULL))
    {
      SENDERR_0("Failed to create Setup Dialog");
      AfxSetResourceHandle(hOldResource);
      return false;
    }
  }
  SetWindowText(DlgHead);
  SetWindowPos(NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
  ShowWindow(SW_SHOWNORMAL);
  AfxSetResourceHandle(hOldResource);
  return true;
}
