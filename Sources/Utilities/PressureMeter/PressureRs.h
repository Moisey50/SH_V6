#if !defined(AFX_PRESSURERS_H__77CCBC08_BBC1_4E77_9FC5_413C6EBBA395__INCLUDED_)
#define AFX_PRESSURERS_H__77CCBC08_BBC1_4E77_9FC5_413C6EBBA395__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PressureRs.h : header file
//

#include <afxdb.h>


/////////////////////////////////////////////////////////////////////////////
// CPressureRs recordset

class CPressureRs : public CRecordset
{
public:
	CPressureRs(CDatabase* pDatabase = NULL);
	bool AddValue(double value);
	DECLARE_DYNAMIC(CPressureRs)

// Field/Param Data
	//{{AFX_FIELD(CPressureRs, CRecordset)
	float	m_pressure;
	//}}AFX_FIELD
	TIMESTAMP_STRUCT	m_datime;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPressureRs)
	public:
	virtual CString GetDefaultConnect();    // Default connection string
	virtual CString GetDefaultSQL();    // Default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);  // RFX support
	//}}AFX_VIRTUAL

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRESSURERS_H__77CCBC08_BBC1_4E77_9FC5_413C6EBBA395__INCLUDED_)
