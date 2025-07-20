#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "resource.h"
#include <gadgets\gadbase.h>
#include <gadgets\stdsetup.h>

// ScriptRunnerDlg dialog

class GenericScriptRunner;
class GenericScriptRunnerDlg : public CGadgetSetupDialog
{
	DECLARE_DYNAMIC(GenericScriptRunnerDlg)
public:
	GenericScriptRunnerDlg(GenericScriptRunner* Gadget, CWnd* pParent = NULL);   // standard constructor
	virtual ~GenericScriptRunnerDlg();
	BOOL Create(UINT nIDTemplate, CWnd* pParentWnd = NULL);
	afx_msg void OnBnClickedBrowse();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();	
	virtual void UploadParams();
  virtual bool Show( CPoint point , LPCTSTR uid ) ;
	DECLARE_MESSAGE_MAP()
	enum { IDD = IDD_SCRIPT_PATH_DLG };
  FXString m_ScriptPath;
  FXString m_GadgetName; 
  int m_iNumOfOutPins;
public:
	afx_msg void OnBnClickedReload();
};
