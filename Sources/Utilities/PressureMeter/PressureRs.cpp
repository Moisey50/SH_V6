// PressureRs.cpp : implementation file
//

#include "stdafx.h"
#include "pressuremeter.h"
#include "PressureRs.h"
#include "DataBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPressureRs

IMPLEMENT_DYNAMIC(CPressureRs, CRecordset)

CPressureRs::CPressureRs(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CPressureRs)
	m_pressure = 0.0f;
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CPressureRs::GetDefaultConnect()
{
	CString sTmp = "ODBC;DSN=" + GetDSN();
	return sTmp;
}

CString CPressureRs::GetDefaultSQL()
{
	return _T("[Pressure]");
}

void CPressureRs::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CPressureRs)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Date(pFX, _T("[datime]"), m_datime);
	RFX_Single(pFX, _T("[pressure]"), m_pressure);
	//}}AFX_FIELD_MAP
}

bool CPressureRs::AddValue(double value)
{
    bool res=true;
    AddNew();
    CTime ts=CTime::GetCurrentTime();
    m_datime.year=ts.GetYear();
    m_datime.month=ts.GetMonth();
    m_datime.day=ts.GetDay();
    m_datime.hour=ts.GetHour();
    m_datime.minute=ts.GetMinute();
    m_datime.second=ts.GetSecond();
    m_pressure=(float)value;
	try
	{
        Update();
    }
	catch(CDBException *e)
	{
        TRACE("!!! ODBC Error: %s\n",(LPCTSTR)e->m_strError);
		e->Delete();
		return false;
	}
    return true; 
}

/////////////////////////////////////////////////////////////////////////////
// CPressureRs diagnostics

#ifdef _DEBUG
void CPressureRs::AssertValid() const
{
	CRecordset::AssertValid();
}

void CPressureRs::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
