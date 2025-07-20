#if !defined(AFX_WDMSETUP_H__4376085C_2979_4411_B79B_2CDCA961F786__INCLUDED_)
#define AFX_WDMSETUP_H__4376085C_2979_4411_B79B_2CDCA961F786__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WdmSetup.h : header file
//

//#include <gadgets\gadbase.h>
#include <gadgets\stdsetup.h>
#include "afxwin.h"

#define BUTTONS_NMB 10

// they repeat VfwCaptureDialogs Enumeration, but skipped "_" in a name
typedef enum
{
  VfwNope ,
  VfwCaptureDialogSource ,
  VfwCaptureDialogFormat ,
  VfwCaptureDialogDisplay ,
  //    VfwCatureAudioFilter,
  WdmVideoCaptureFilter ,
  WdmVideoCapturePin ,
  WdmVideoPreviewPin ,
  WdmVideoCrossbar ,
  WdmSecondCrossbar ,
  WdmTVTuner ,
  WdmVideoInput0 ,
  WdmVideoInput1 ,
  WdmVideoInput2
} SetupID;

typedef struct _tagButtons
{
  DWORD       s_ID;
  bool        s_Enable;
  CString     s_Name;
  SetupID     s_SetupID;
}Buttons;


/////////////////////////////////////////////////////////////////////////////
// CWdmSetup dialog

class CWdmSetup : public CGadgetSetupDialog
{
  Buttons m_Buttons[ BUTTONS_NMB ];
  bool    m_UpdateReq;
  UINT_PTR  m_Timer;
private:
  void OnSetup( int nmb );
  void GetCmdList();
  void UpdateButtons();
public:
  CWdmSetup( CGadget* pGadget , CWnd* pParent = NULL );   // standard constructor
  void OnChangeStatus();
  // Dialog Data
    //{{AFX_DATA(CWdmSetup)
  enum
  {
    IDD = IDD_SETUPDIALOG
  };
  CComboBox	m_DeviceList;
  //}}AFX_DATA


// Overrides
  virtual bool Show( CPoint point , LPCTSTR uid );
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CWdmSetup)
protected:
  virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
  //}}AFX_VIRTUAL
  virtual void UploadParams();
  // Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(CWdmSetup)
  afx_msg void OnShowWindow( BOOL bShow , UINT nStatus );
  afx_msg void OnSetup1()
  {
    OnSetup( 0 );
  };
  afx_msg void OnSetup2()
  {
    OnSetup( 1 );
  };
  afx_msg void OnSetup3()
  {
    OnSetup( 2 );
  };
  afx_msg void OnSetup4()
  {
    OnSetup( 3 );
  };
  afx_msg void OnSetup5()
  {
    OnSetup( 4 );
  };
  afx_msg void OnSetup6()
  {
    OnSetup( 5 );
  };
  afx_msg void OnSetup7()
  {
    OnSetup( 6 );
  };
  afx_msg void OnSetup8()
  {
    OnSetup( 7 );
  };
  afx_msg void OnSetup9()
  {
    OnSetup( 8 );
  };
  afx_msg void OnSetup10()
  {
    OnSetup( 9 );
  };
  afx_msg void OnSelchangeCapDevicelist();
  afx_msg void OnTimer( UINT_PTR nIDEvent );
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
public:
  int m_OutputFormat;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WDMSETUP_H__4376085C_2979_4411_B79B_2CDCA961F786__INCLUDED_)
