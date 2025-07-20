// LockObject.h: interface for the CSafeAccess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOCKOBJECT_H__A2A5254B_0042_448D_BF48_9493DA2F78CC__INCLUDED_)
#define AFX_LOCKOBJECT_H__A2A5254B_0042_448D_BF48_9493DA2F78CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class FXFC_EXPORT FXLockObject
{
protected:
    #ifdef _DEBUG
        LONG             m_LockCount;
        TCHAR            m_WhoLock[30];
        DWORD            m_dwId ;
    #endif
    CRITICAL_SECTION m_CritSect;
public:
    FXLockObject();
	 ~FXLockObject();
    BOOL	Lock(DWORD dwTimeOut = INFINITE, LPCTSTR WhoLock=NULL);
    BOOL	LockAndProcMsgs(DWORD dwTimeOut = INFINITE , 
      LPCTSTR WhoLock = NULL );
	void	Unlock();
#ifdef _DEBUG
  LONG       GetLockCount() { return  m_LockCount; }
#endif
  BOOL      IsLockedByThisThread() ; // returns true if thread the same
};

class FXFC_EXPORT FXAutolock
{
	FXLockObject* m_pLock;
public:
	FXAutolock(FXLockObject& lock, LPCTSTR WhoLock=NULL);
    ~FXAutolock();
};

#endif // !defined(AFX_LOCKOBJECT_H__A2A5254B_0042_448D_BF48_9493DA2F78CC__INCLUDED_)
