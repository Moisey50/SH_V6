// Acquire.h: interface for the CAcquire class.
// (C) Copyright The FileX Ltd, 2000
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACQUIRE_H__5DB3BEE1_29B4_11D4_9ABE_EA7A29796362__INCLUDED_)
#define AFX_ACQUIRE_H__5DB3BEE1_29B4_11D4_9ABE_EA7A29796362__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <twain\twain.h>

typedef void (TW_DataRdy)(LPVOID pParam, LPBYTE Data, int Length);

class CAcquire: public CWnd  
{
private:
	BOOL     DoNativeTransfer();
	BOOL     DoMemTransfer();
    TW_UINT16 CallDSMEntry(pTW_IDENTITY pApp, pTW_IDENTITY pSrc,TW_UINT32 DG, TW_UINT16 DAT, TW_UINT16 MSG, TW_MEMREF pData);
    BOOL     IsSourceManagerReady() {return(m_TWDSMOpen);};  
    BOOL     IsSourceOpen() {return(m_TWDSOpen);};
    BOOL     IsAuiredNow() {return(m_AcquireNow); };
	void     CloseSourceManager();
	BOOL     OpenSourceManager();
protected:
    TW_DataRdy*  m_CallBack;
    LPVOID       m_CallBackParam;
    CString      m_ErrorMessage;   // Here stored last error
    CString      m_DSMName;        // full path to Source manager
    HINSTANCE    m_DSMDLL;         // handle to Source Manager for explicit load
    DSMENTRYPROC m_DSMEntry;       // entry point to the SM
    TW_IDENTITY  m_appID, m_dsID;  // storage for App and DS (Source) states
    TW_STATUS    m_GlobalStatus;
    BOOL         m_TWDSMOpen;      // glue code flag for an Open Source Manager
    BOOL         m_TWDSOpen;       // glue code flag for an Open Source
    BOOL         m_AcquireNow;     // Capturing process are going now
    BOOL         m_ShowInterface;  // Show capture interface
    int          m_CaptureMode;
public:
	         CAcquire();
	virtual  ~CAcquire();
    enum CaptureMode
    {
        Native,File,Memory
    };
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAcquire)
	public:
	virtual BOOL Create(CWnd* Parent,TW_DataRdy* pCallBack, LPVOID pParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
	void     SelectSource(); // Select TWAIN device
    BOOL     DoIt();         // Start acquisition
// Inlines
    CString& GetErrorMessage() {m_ErrorMessage="CAcquire: "+m_ErrorMessage; return(m_ErrorMessage);};// returns last error
    BOOL     Error() {return(m_ErrorMessage.GetLength()!=0);};      // check if error occures
    void     SetShowInterface(BOOL Show) { m_ShowInterface=Show; }; // setup  show or not TWAIN interface
    void     SetCaptureMode(CaptureMode capmode) {m_CaptureMode=capmode;};
protected:
	//{{AFX_MSG(CAcquire)
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_ACQUIRE_H__5DB3BEE1_29B4_11D4_9ABE_EA7A29796362__INCLUDED_)
