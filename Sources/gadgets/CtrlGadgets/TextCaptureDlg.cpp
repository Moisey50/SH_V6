// TextCaptureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ctrlgadgets.h"
#include "TextCaptureDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DlgItem DI[]=
{
    {IDC_EDIT_CTRL,BOTTOM_ALIGN|RIGHT_ALIGN,{0,0,0,0},(DWORD)-1},
    {IDC_SEND,BOTTOM_MOVE|RIGHT_MOVE,{0,0,0,0},(DWORD) -1},
    {IDC_FROM_FILE,BOTTOM_MOVE/*|RIGHT_MOVE*/,{0,0,0,0},(DWORD) -1}
};

extern CDynLinkLibrary* pThisDll;

/////////////////////////////////////////////////////////////////////////////
// CTextCaptureDlg dialog


CTextCaptureDlg::CTextCaptureDlg(CWnd* pParent /*=NULL*/)
	: CResizebleDialog(CTextCaptureDlg::IDD, pParent),
    m_Callback(NULL)
{
	//{{AFX_DATA_INIT(CTextCaptureDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTextCaptureDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizebleDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTextCaptureDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTextCaptureDlg, CResizebleDialog)
	//{{AFX_MSG_MAP(CTextCaptureDlg)
	ON_BN_CLICKED(IDC_SEND, OnSend)
	ON_EN_CHANGE(IDC_EDIT_CTRL, OnChangeEditCtrl)
  ON_MESSAGE( UM_SEND_DATA , OnSendData )
  ON_BN_CLICKED(IDC_FROM_FILE, &CTextCaptureDlg::OnBnClickedFromFile)
	//}}AFX_MSG_MAP
//    ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextCaptureDlg message handlers

void CTextCaptureDlg::OnSend() 
{
    if (m_Callback)
    {
        m_Callback(IDC_SEND, BN_CLICKED, m_UserParam,0);
    }
}

afx_msg LRESULT CTextCaptureDlg::OnSendData( WPARAM wParam , LPARAM lParam )
{
  OnSend() ;
  return 1 ;
}

void CTextCaptureDlg::OnChangeEditCtrl() 
{
    if (m_Callback)
    {
        m_Callback(IDC_EDIT_CTRL, EN_CHANGE, m_UserParam,0);
    }
}

BOOL CTextCaptureDlg::OnInitDialog() 
{
     CResizebleDialog::OnInitDialog(DI, sizeof(DI)/sizeof(DlgItem));
     EnableFitting();
     return TRUE;
}

BOOL CTextCaptureDlg::Create(int iTemplate, CWnd* pParentWnd)
{
     LPTSTR lpszTemplateName=MAKEINTRESOURCE(iTemplate);
     ASSERT(IS_INTRESOURCE(lpszTemplateName) ||
	     AfxIsValidString(lpszTemplateName));

     m_lpszTemplateName = lpszTemplateName;  // used for help
     if (IS_INTRESOURCE(m_lpszTemplateName) && m_nIDHelp == 0)
	     m_nIDHelp = LOWORD((DWORD_PTR)m_lpszTemplateName);

     HINSTANCE hInst = pThisDll->m_hResource;
     HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
     HGLOBAL hTemplate = LoadResource(hInst, hResource);
     BOOL bResult = CreateIndirect(hTemplate, pParentWnd, hInst);
     FreeResource(hTemplate);

     return bResult;
}


void CTextCaptureDlg::OnBnClickedFromFile()
{
  CFileDialog fd(TRUE, "txt", m_FileName, OFN_FILEMUSTEXIST, "txt (*.txt)|*.txt|all files (*.*)|*.*||", this);
  if (fd.DoModal() == IDOK)
  {
    m_FileName = fd.GetPathName();
    ReadFile( m_FileName ) ;
  }
}


int CTextCaptureDlg::ReadFile(LPCTSTR pFileName)
{
  CFile TextFile ;
  if ( TextFile.Open( m_FileName , CFile::modeRead ) )
  {
    int iLen = (int) TextFile.GetLength() ;
    if ( iLen > 0 )
    {
      char * pBuf = new char[ iLen + 1 ] ;
      TextFile.Read( pBuf , iLen ) ;

      pBuf[iLen] = 0 ;
      FXString Encoded ;
      ConvBinStringToView( pBuf , Encoded ) ;

      GetDlgItem( IDC_EDIT_CTRL )->SetWindowText( (LPCTSTR)Encoded ) ;
      delete pBuf ;
      if ( m_Callback )
      {
        m_Callback(IDC_FROM_FILE, EN_CHANGE, m_UserParam,0);
      }
    }
    TextFile.Close() ;
    return ( iLen > 0 ) ;
  }
  return 0;
}
