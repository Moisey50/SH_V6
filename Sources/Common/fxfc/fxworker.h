#ifndef FXWORKER_INCLUDE
#define FXWORKER_INCLUDE
// FXWorker.h : header file
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DEFAULT_TICKS_IDLE	1000

class CThreads;
class FXWorker;

typedef struct	tagWORKERTHREAD
{
  HANDLE		hThread;
  DWORD		  dwAffinity;
  DWORD     dwPriority ;
  DWORD		  dwUID;
  HANDLE    evReady;
  HANDLE		evExit;
  DWORD     dwThreadId; // Thread system ID
  FXWorker  *pWorker;
}WORKERTHREAD;

typedef struct	tagWORKERINITINFO
{
  HANDLE		hProcess;
  SECURITY_ATTRIBUTES SA;
  BOOL		bHasSA;
  DWORD		dwStack;
}WORKERINITINFO;

class FXFC_EXPORT FXWorker
{
protected:
  HANDLE			m_evExit;
  HANDLE			m_evResume;
  FXString    m_Name;
  volatile DWORD	m_dwTicksIdle;
  double			m_CPUUsage;
  double      m_LastProcTime;
  double			m_LastCheck;
  bool        m_MultyCoreAllowed;
  DWORD       m_Affinity ;
  DWORD       m_Priority ;
  DWORD       m_ThreadsNmb;
  WORKERINITINFO	m_WIInfo;
  FXLockObject	m_ThreadsLock;
  CThreads*		m_Threads;
public:
  const static DWORD m_ProcessorMask ;
  FXWorker();
  virtual  ~FXWorker();

  DWORD 		    SetTicksIdle( DWORD dwTicksIdle = DEFAULT_TICKS_IDLE );
  virtual BOOL 	Create( HANDLE hProcess = NULL , LPSECURITY_ATTRIBUTES lpSA = NULL , DWORD dwStack = 0 , DWORD dwFlags = CREATE_SUSPENDED , LPDWORD lpThreadID = NULL , int nCores = 1 );
  BOOL 			Suspend();
  BOOL 			Resume();
  virtual void 	Destroy();
  int			    GetCoresNumber();
  BOOL		    SetCoresNumber( int nCores );
  bool		    SetAffinity( DWORD Affinity );
  DWORD		    GetAffinity();
  bool		    SetPriority( DWORD Priority );
  DWORD		    GetPriority();
  void		    SetThreadName( LPCTSTR name );
  virtual DWORD   GetThreadId( int nCore = 0 );
  double          GetCPUUsage();
  void		    AddCPUUsage( double ms )
  {
    m_LastProcTime = ms; 
    m_CPUUsage += ms;
  }
  double		    GetLastProcTime()
  {
    return m_LastProcTime;
  }
  bool          IsMultyCoreAllowed()
  {
    return m_MultyCoreAllowed;
  }
protected:
  enum
  {
    WR_EXIT , WR_CONTINUE
  }WORK_RESULT;
private:
  virtual int		DoJob();
  virtual void  OnThreadStart();
  virtual void  OnThreadEnd();
  void		    Work( WORKERTHREAD* wrkthread );
  bool		    SetCoreThreadAffinity( DWORD Affinity , int nCore = 0 , bool Lock = true );
  DWORD		    GetCoreThreadAffinity( int nCore = 0 );
  bool		    SetCoreThreadPriority( DWORD Priority , int nCore = 0 );
  DWORD		    GetCoreThreadPriority( int nCore = 0 );
  void		    SetThreadsNames();
  static void _workerFunc( void* pwrkthread );
};

#define MS_VC_EXCEPTION (0x406D1388)

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
  DWORD dwType; // Must be 0x1000.
  LPCSTR szName; // Pointer to name (in user addr space).
  DWORD dwThreadID; // Thread ID (-1=caller thread).
  DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

__forceinline void DoSetThreadName( THREADNAME_INFO& info )
{
  __try
  {
    RaiseException( MS_VC_EXCEPTION , 0 , sizeof( info ) / sizeof( ULONG_PTR ) , (ULONG_PTR*) &info );
  }
  __except ( EXCEPTION_EXECUTE_HANDLER )
  {}
}

inline void SetCurrentThreadName( LPCSTR pName )
{
  HANDLE h = GetCurrentThread() ;
  THREADNAME_INFO info ;
  info.dwType = 0x1000 ;
  info.dwFlags = 0 ;
  info.dwThreadID = GetThreadId( h ) ;
  info.szName = pName ;

  DoSetThreadName( info ) ;
}

inline void SetThreadName( LPCSTR pName , DWORD dwThredId )
{
  THREADNAME_INFO info ;
  info.dwType = 0x1000 ;
  info.dwFlags = 0 ;
  info.dwThreadID = dwThredId ;
  info.szName = pName ;

  DoSetThreadName( info ) ;
}

#endif //#ifndef FXWORKER_INCLUDE
