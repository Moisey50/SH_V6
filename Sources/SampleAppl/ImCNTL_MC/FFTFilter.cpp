// FFTFilter.cpp : implementation file
//

#include "stdafx.h"
#include "ImCNTL.h"
#include "FFTFilter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFFTFilter dialog


CFFTFilter::CFFTFilter(CWnd* pParent /*=NULL*/)
	: CDialog(CFFTFilter::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFFTFilter)
	m_Rect1 = _T("");
	m_Rect2 = _T("");
	//}}AFX_DATA_INIT
}


void CFFTFilter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFFTFilter)
	DDX_Text(pDX, IDC_RECTANGLE1, m_Rect1);
	DDX_Text(pDX, IDC_RECTANGLE2, m_Rect2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFFTFilter, CDialog)
	//{{AFX_MSG_MAP(CFFTFilter)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFFTFilter message handlers
