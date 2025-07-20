#pragma once


// CSplashWnd dialog

class CSplashWnd : public CDialog
{
	DECLARE_DYNAMIC(CSplashWnd)
protected:
	static BOOL             c_bShowSplashWnd;
	static CSplashWnd*   c_pSplashWnd;
    UINT_PTR                m_hTimer;
    bool                    m_IsAboutToClose;
	BITMAP                  m_Bitmap;
	HBITMAP                 m_hBitmap;
    CFont                   m_VersionFont, m_LicenseFont;
public:
	CSplashWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSplashWnd();

    static void EnableSplashScreen(BOOL bEnable = TRUE);
	static void ShowSplashScreen(bool Demo, CWnd* pParentWnd = NULL);
	//static BOOL PreTranslateAppMessage(MSG* pMsg);
    static void WaitEnd();

// Dialog Data
	enum { IDD = IDD_NEW_SPLASHDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
public:
	virtual void OnOK();
	virtual void OnCancel();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
};
