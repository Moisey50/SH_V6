
// GraphDeployDlg.h : header file
//

#pragma once
#include "habitat.h"
#include "gadgets\stdsetup.h"
#include <gadgets\textframe.h>
#include <gadgets\videoframe.h>
#include <files\futils.h>
#include <helpers\shwrapper.h>
#include <math\intf_sup.h>
#include <string>
#include <mutex>


#define LED_CONTROL_TIMER 10

using namespace std ;

enum SetPropertyGlobal
{
  SPG_ByString = 0 ,
  SPG_BySharedMemory
};

// CGraphDeployDlg dialog
class CGraphDeployDlg : public CDialogEx
{
  ConnectGraph    m_Graph ;
  ConnectGraph    m_LANControlGraph ;
  CWnd *          m_pDebugWnd ;
  string          m_TVGFileName;
  CStatic         m_StatusLED;
  CBitmap         m_BlackLed , m_GreenLed , m_YellowLed , m_BlueLed , m_RedLed ;
  CPoint          m_LastPosition ;

  std::mutex      m_LogMutex ;
  string		    	m_Message;
  bool			      m_bPrintMessage;
  bool            m_bAutoStart ;
  bool            m_bMinimize ;
  FXString        m_LANAccumulator ;
  int             m_iLANPort             ;



// Construction
public:
	CGraphDeployDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GRAPHDEPLOY_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedStart();
  afx_msg void OnBnClickedBrowse();
  void	PrintMessage( LPCTSTR message );
  int LoadAndRunGraph( LPCTSTR pGraphName , bool bStart );
   afx_msg void OnBnClickedUnload();
   afx_msg void OnBnClickedViewGraph();
   afx_msg void OnParentNotify( UINT message , LPARAM lParam );
   bool ParseCommandLine( CCommandLineInfo& rCmdInfo ) ;
   LRESULT OnLANMessage( WPARAM wParam , LPARAM lParam );
   bool SendToLAN( LPCTSTR pMsg );
   // Every  LF will lead command processing; the rest will be hold up to next LF received
   int ProcessLANCommand();
   CStatic m_LANStatusLed;
   afx_msg void OnBnClickedCancel();
   afx_msg LRESULT OnSetGadgetProperties( WPARAM wParam , LPARAM lParam );
   afx_msg void OnMove( int x , int y );
};
