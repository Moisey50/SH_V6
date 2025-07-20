#if !defined(AFX_FLWRENDERSTP_H__D2DC8D69_935E_43BD_8F57_9E64F4E6FB78__INCLUDED_)
#define AFX_FLWRENDERSTP_H__D2DC8D69_935E_43BD_8F57_9E64F4E6FB78__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FlwRenderStp.h : header file
//

#include <Gadgets\gadbase.h>
#include <gadgets\stdsetup.h>

/////////////////////////////////////////////////////////////////////////////
// FLWRenderStp dialog
class FLWRender;
class FLWRenderStp : public CGadgetSetupDialog
{
protected:
  UINT_PTR   m_Timer;
  BOOL       m_bDoLog;
public:
  FLWRenderStp( CGadget* pGadget , CWnd* pParent = NULL );
  virtual void UploadParams();
  // Dialog Data
    //{{AFX_DATA(FLWRenderStp)
  enum
  {
    IDD = IDD_FLW_RENDER_DLG
  };
  CString	m_FileName;
  //}}AFX_DATA
// Overrides
  virtual bool Show( CPoint point , LPCTSTR uid );
  BOOL IsLoggerOn() { return m_bDoLog ; }
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(FLWRenderStp)
protected:
  virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(FLWRenderStp)
//	afx_msg void OnFnhelp();
  afx_msg void OnBrowseFilename();
  afx_msg void OnCloseFile();
  afx_msg void OnTimer( UINT_PTR nIDEvent );
  virtual BOOL OnInitDialog();
  afx_msg void OnDestroy();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
public:
  // If not zero then preallocated buffers will be used  // If not zero then preallocated buffers will be used
  int m_NPreallocatedBuffers;
  int m_BuffSize_KB;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLWRENDERSTP_H__D2DC8D69_935E_43BD_8F57_9E64F4E6FB78__INCLUDED_)
