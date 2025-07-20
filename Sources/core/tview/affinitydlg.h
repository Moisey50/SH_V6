#ifndef CAFFINITYDLG_INC
#define CAFFINITYDLG_INC

// CAffinityDlg dialog
#include "resource.h"
#include <gadgets\gadbase.h>

class CAffinityDlg : public CDialog
{
	DECLARE_DYNAMIC(CAffinityDlg)
private:
	CGadget* m_pGadget;
	DWORD	   m_ProcMask;
  DWORD    m_Priority ;
public:
	CAffinityDlg(CGadget* pGadget, CWnd* pParent = NULL);   // standard constructor
	virtual ~CAffinityDlg();

// Dialog Data
	enum { IDD = IDD_AFFINITY_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeAffinityEdt();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedOk();
  afx_msg void OnLbnSelchangeList1();
  CListBox m_PrioritySelect;
};

#endif