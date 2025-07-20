// DataBase.cpp: implementation of the CDataBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pressuremeter.h"
#include "DataBase.h"
#include "PressureRs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
CString DataSourcename="undefined";

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define DATASOURCENAME "pressure"

CString& GetDSN() 
{
    return DataSourcename;
}

CDataBase::CDataBase(bool ReadOnly)
{
	m_IsOpen=true;
    CString cs;
	DataSourcename=AfxGetApp()->GetProfileString("root","DSN",DATASOURCENAME);
    cs.Format("DSN=%s",DataSourcename);
	try
	{
        m_Database.OpenEx(cs, CDatabase::noOdbcDialog /*| ((ReadOnly)?CDatabase::openReadOnly:0)*/); //_MAK_140601
	}
	catch(CDBException *e)
	{
        m_IsOpen=false;
		e->Delete();
		AfxMessageBox("Unable to open data source 'pressure'");
	}
}

CDataBase::~CDataBase()
{
    if (m_IsOpen)
        m_Database.Close();
}

bool CDataBase::AddValue(double value)
{
    CPressureRs pt(&m_Database);
    pt.Open();
    bool res=pt.AddValue(value);
    pt.Close();
    return res;
}