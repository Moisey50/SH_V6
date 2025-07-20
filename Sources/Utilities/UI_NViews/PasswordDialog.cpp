// PasswordDialog.cpp : implementation file
//

#include "stdafx.h"
#include "UI_NViews.h"
#include "PasswordDialog.h"
#include "afxdialogex.h"


// PasswordDialog dialog

IMPLEMENT_DYNAMIC(PasswordDialog, CDialogEx)

PasswordDialog::PasswordDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TECHNICIAN_PW, pParent)
  , m_Password( _T( "" ) )
{

}

PasswordDialog::~PasswordDialog()
{
}

void PasswordDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange( pDX );
  DDX_Text( pDX , IDC_PW_AS_TEXT , m_Password );
}


BEGIN_MESSAGE_MAP(PasswordDialog, CDialogEx)
END_MESSAGE_MAP()


// PasswordDialog message handlers


BOOL PasswordDialog::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  ::SetFocus( GetDlgItem( IDC_PW_AS_TEXT )->GetSafeHwnd() ) ;
  return FALSE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}
