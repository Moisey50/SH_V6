#pragma once


// CConnectToDlg dialog

class CConnectToDlg : public CDialog
{
	DECLARE_DYNAMIC(CConnectToDlg)

public:
	CConnectToDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConnectToDlg();

// Dialog Data
	enum { IDD = IDD_CONNECT_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_Host;
	short m_Port;
};
