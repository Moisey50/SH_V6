#if !defined(AFX_AVIRENDERDIALOG_H__BE88249C_53E7_44CC_AFFC_58A3BD557A9B__INCLUDED_)
#define AFX_AVIRENDERDIALOG_H__BE88249C_53E7_44CC_AFFC_58A3BD557A9B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AviRenderDialog.h : header file
//

#include <Gadgets\gadbase.h>
#include <Helpers\CodecEnumerator.h>
#include "Resource.h"
#include "afxwin.h"
#include <gadgets\stdsetup.h>

/////////////////////////////////////////////////////////////////////////////
// AviRenderDialog dialog

class AviRender;
class AviRenderDialog : public CGadgetSetupDialog
{
  CCodecEnumerator    m_Codecs;
  DWORD               m_VIDC_FOURCC;
  BOOL                m_CycleWritting;
  int                 m_MaxFileNumber;
  int                 m_MaxFileLength;
  BOOL	            m_bOverwrite;
    BOOL                m_CalcFrameRate;
    BOOL                m_OverwriteFrameRate;
    double              m_FrameRate;
    int m_iRenderId;
  // Construction
public:
  AviRenderDialog(CGadget* pGadget, CWnd* pParent = NULL);
  // Dialog Data
  //{{AFX_DATA(AviRenderDialog)
  enum { IDD = IDD_AVI_RENDER_DLG };
  FXString	m_Filename;
  CComboBox m_CodecList;
  //}}AFX_DATA
  // Overrides
  virtual bool Show(CPoint point, LPCTSTR uid);
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(AviRenderDialog)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
protected:
  virtual void UploadParams();
  void    UpdateInterface();
  // Generated message map functions
  //{{AFX_MSG(AviRenderDialog)
  virtual BOOL OnInitDialog();
  afx_msg void OnBrowseFilename();
  afx_msg void OnOverwrite();
  afx_msg void OnCloseFile();
  afx_msg void OnAddInput();
  afx_msg void OnSelchangeCodecList();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedCyclewritting();
    afx_msg void OnBnClickedOverwiteFramerate();
    afx_msg void OnBnClickedCalcFramerate();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AVIRENDERDIALOG_H__BE88249C_53E7_44CC_AFFC_58A3BD557A9B__INCLUDED_)
