#pragma once


// PasswordDialog dialog

class PasswordDialog : public CDialogEx
{
	DECLARE_DYNAMIC(PasswordDialog)

public:
	PasswordDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~PasswordDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TECHNICIAN_PW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
  CString m_Password;
  virtual BOOL OnInitDialog();
};
