// FLWPlayerDlg.h : header file
//

#if !defined(AFX_FLWPLAYERDLG_H__63612FBB_E420_4BFB_B504_8DCE64E1BD07__INCLUDED_)
#define AFX_FLWPLAYERDLG_H__63612FBB_E420_4BFB_B504_8DCE64E1BD07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\shkernel.h>
#include <shbase\shbase.h>

/////////////////////////////////////////////////////////////////////////////
// CFLWPlayerDlg dialog

class CFLWPlayerDlg : public CResizebleDialog
{
private:
  IGraphbuilder*	m_pBuilder;
  IPluginLoader*	m_PluginLoader;
  CString         m_TVGFileName;
  CString         m_FLWName;
  CRenderGadget*  m_RenderGadget;
  unsigned        m_SliderPos;
  double          m_LastFrameTime;
  CString			m_Message;
  bool			m_PrintMessage;
  CString         m_FrameRate;
  bool            m_RangeChangeed;
  DWORD           m_NewRange;
  bool            m_RunBeforeInspect;
  //CDebugWnd*		m_pDebugWnd;
public:
  CFLWPlayerDlg( CWnd* pParent = NULL );	// standard constructor
  void    SetFileName( LPCTSTR name );
  void    OnAsyncTransaction( CDataFrame* pParamFrame );
  void	PrintMessage( LPCTSTR message );
  // Dialog Data
    //{{AFX_DATA(CFLWPlayerDlg)
  enum
  {
    IDD = IDD_FLWPLAYER_DIALOG
  };
  CSliderCtrl	m_VideoPos;
  //}}AFX_DATA

  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CFLWPlayerDlg)
protected:
  virtual void DoDataExchange( CDataExchange* pDX );	// DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  HICON   m_hIcon;
  HICON   m_hOpenFile;
  HICON   m_hRun;
  HICON   m_hStop;
  HICON   m_hSetup;
  HICON	m_hViewGraph;
  // Generated message map functions
  //{{AFX_MSG(CFLWPlayerDlg)
  virtual BOOL OnInitDialog();
  afx_msg void OnSysCommand( UINT nID , LPARAM lParam );
  afx_msg void OnPaint();
  afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void OnRun();
  afx_msg void OnStop();
  afx_msg void OnSetup();
  afx_msg void OnDestroy();
  afx_msg void OnOpen();
  afx_msg void OnSize( UINT nType , int cx , int cy );
  afx_msg void OnHScroll( UINT nSBCode , UINT nPos , CScrollBar* pScrollBar );
  afx_msg void OnTimer( UINT_PTR nIDEvent );
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnViewGraph();
  afx_msg void OnParentNotify( UINT message , LPARAM lParam );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLWPLAYERDLG_H__63612FBB_E420_4BFB_B504_8DCE64E1BD07__INCLUDED_)
