#if !defined(AFX_CComPortSettings_H__479C1FE6_6867_11D2_BC17_D7DE4E9AA304__INCLUDED_)
#define AFX_CComPortSettings_H__479C1FE6_6867_11D2_BC17_D7DE4E9AA304__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CComPortSettings.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CComPortSettings dialog
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13
#include "resource.h"
#include <Gadgets\gadbase.h>
#include <gadgets\stdsetup.h>

#define ARR_SIZE(x) (sizeof(x)/sizeof(x[0]))

typedef struct tagComPortParam
{
    int  Port,BaudRate,DataBits,Parity,StopBits;
    BOOL XonXoff,RtsCts,DtrDsr;
}ComPortParam,*pComPortParam;

class COMCapture;
class CComPortSettings : public CGadgetSetupDialog
{
protected:
    pComPortParam CfgParamData;
public:
	CComPortSettings(CGadget* pGadget, CWnd* pParent = NULL);
    BOOL Create(UINT nIDTemplate, CWnd* pParentWnd = NULL);

// Dialog Data
	//{{AFX_DATA(CComPortSettings)
	enum { IDD = IDD_COMMCNFIG };
	int	    m_PORTCB;
	int	    m_Parity;
	int	    m_DatBits;
	int	    m_BaudRate;
	int	    m_StopBits;
	BOOL	m_DTRDSR;
	BOOL	m_XonXoff;
	BOOL	m_RtsCts;
	BOOL	m_WaitForReturn;
	//}}AFX_DATA


// Overrides
    virtual bool Show(CPoint point, LPCTSTR uid);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CComPortSettings)
	public:
	//virtual int DoModal(pComPortParam param);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
    virtual void UploadParams();
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CComPortSettings)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// const char * GetPortName(int No);
// DWORD        GetBaudRate(int No);
// BYTE         GetParity(int No);
// #define      GetByteSize(a) ((a)+5)
// #define      GetStopBits(a) (a)
// extern const DWORD Bauds[] ;
// extern const char * ParityAsText[5] ;

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CComPortSettings_H__479C1FE6_6867_11D2_BC17_D7DE4E9AA304__INCLUDED_)
