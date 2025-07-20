/********************************************************************
	created:	2005/11/09
	created:	9:11:2005   14:55
	filename: 	ProcessThread.h
	file path:	c:\msc_proj\NewComponents\imcntlmv2
	file base:	ProcessThread
	file ext:	h
	author:	Michael Son	
	
	purpose:	 Creating for functions for running command thread,process thread 
	 	and capture thread
*********************************************************************/


#if !defined(AFX_PROCESSTHREAD_H__8056F3EB_E564_46EF_A522_966EB05AE94C__INCLUDED_)
#define AFX_PROCESSTHREAD_H__8056F3EB_E564_46EF_A522_966EB05AE94C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcessThread.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ProcessThread window
typedef struct tagSynchData 
{
  int iSleepTime;
  int iCounter  ;
  int ilimit   ;
  tagSynchData * SynchDataPtr ;
} SynchData;

class ProcessThread 
{
// Construction
public:
  public:
  	ProcessThread( AFX_THREADPROC pCommandFunc, LPVOID pCaptPar ,
                      AFX_THREADPROC pProcessFunc, LPVOID pProcPar ,
                      AFX_THREADPROC  pCaptureFunc, LPVOID pCommPar  );
    ProcessThread();
    void Close();
    AFX_THREADPROC m_pCaptureFunc ;				 
    AFX_THREADPROC m_pProcessFunc ;
    AFX_THREADPROC m_pCommandFunc ;
    LPVOID         m_pCaptPar ;
    LPVOID         m_pProcPar ;
    LPVOID         m_pCommPar ;
    int            m_bStopFlag ;
    SynchData m_Par1,m_Par2,m_Par3;


// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ProcessThread)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ProcessThread();

	// Generated message map functions
protected:
	//{{AFX_MSG(ProcessThread)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCESSTHREAD_H__8056F3EB_E564_46EF_A522_966EB05AE94C__INCLUDED_)
