// DataBase.h: interface for the CDataBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATABASE_H__83915FAF_4ED2_4376_9C7F_AE15822DF2E9__INCLUDED_)
#define AFX_DATABASE_H__83915FAF_4ED2_4376_9C7F_AE15822DF2E9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxdb.h>

extern CString& GetDSN();

class CDataBase  
{
private:
	CDatabase	m_Database;
	bool		m_IsOpen;
public:
			 CDataBase(bool ReadOnly=false); 
	virtual ~CDataBase();
	bool	 AddValue(double value);
};

#endif // !defined(AFX_DATABASE_H__83915FAF_4ED2_4376_9C7F_AE15822DF2E9__INCLUDED_)
