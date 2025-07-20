// InstanceCntr.h: interface for the FXInstanceCntr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INSTANCECNTR_H__2BFF638B_FB97_4EC3_9817_629E10F9EF1F__INCLUDED_)
#define AFX_INSTANCECNTR_H__2BFF638B_FB97_4EC3_9817_629E10F9EF1F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class FXFC_EXPORT FXInstanceCntr  
{
private:
    FXString    m_InstanceName;
    HANDLE      m_hMapping;
    int        *m_pInstanceCntr;
    int         m_InstNmb;
public:
	FXInstanceCntr(const char* name=NULL);
	~FXInstanceCntr();
    int      GetInstanceNmb() { return m_InstNmb; };
    static int GetInstanceNmb(const char* name);
};

#endif // !defined(AFX_INSTANCECNTR_H__2BFF638B_FB97_4EC3_9817_629E10F9EF1F__INCLUDED_)
