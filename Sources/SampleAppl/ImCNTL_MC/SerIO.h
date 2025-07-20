// SerIO.h: interface for the CCSerIO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERIO_H__4621CA55_464F_11D2_ACFA_00A0C9B00E4F__INCLUDED_)
#define AFX_SERIO_H__4621CA55_464F_11D2_ACFA_00A0C9B00E4F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "afxtempl.h"
#include "afxmt.h"
#include "api232.h"
class CCSerIO;

typedef CArray<CCSerIO*,CCSerIO*> SerChanArray;

#define SERIO_FLAG_CHAR_MSG ( WM_USER + 10 )


class CCSerIO: public CObject  
{
public:
	void SetControlWindow( CWnd * pWnd );
	CWnd * m_ControlWindow;
	int SendData( CString& Data );
	int SetFlagChar( char FlagChar , int Set );
	int m_AutoAnswered;
  int m_Close;
  HANDLE m_Event;
  HANDLE m_ThreadHandle;
  DWORD  m_ThreadId ;
//  CCriticalSection m_CriticalSection ; // unused 29.10.09 Alex

	int GetTimeForSend( int BufLen );
	void SetSendAfter( CString& Msg , int TimeOut );
	CString m_SendAfter;
	double m_LastInterruptMoment;
	int WaitForSendFinish( double SleepAfter );
	double m_SleepTime;
	int WaitAcknowledge(double TimeOut_ms);
	double GetAcknowledgeTime();
	double m_AckTime;        // how much time taken for operation
  double m_AckMoment;      // when operation finished
	double m_SendTime;       // how much time taken for send
	double m_SendStartMoment;  // when begin
	double m_SendFinishMoment; // when buffer exchausted
	int GetNSentWithHandShake();
	int GetAutoFlush();
	void SetAutoFlush( int MaxLen ); // if <= 0   -  no flush
	int IsAck();
	int m_LastError;
  int m_IsAck;
  int m_InAckSequence;
	virtual int SetReceiveIrqReaction( 
    int Install , int TimeOut = 0 , 
    CString * In = NULL , CString * Out = NULL , CString * Ack = NULL );
	virtual int SendChar( int Char , int delay_ms = 0 );
	virtual int Reprogram( int Port , 
    int Baud = 9600 , int Mode = 0x03 , int Flow = 0 );
	virtual int GetPort();
	virtual int SendData( const char * buf , int BufLen , int delay = 0 );
	virtual int GetInputData( BYTE * buf , int BufLen );
	virtual int GetInputLen();
	virtual int GetMode();
	virtual int GetBaud();
	CCSerIO( int Port , int Baud = 9600 , int Mode=0x03 , int Flow = 0 );
	virtual ~CCSerIO();
	UINT m_Timer;
	int m_TimeOut;
	int m_SetReceiveReaction;
	CString m_Out;
	CString m_In;
  CString m_Ack;
  CString m_AutoEcho;  // String for sending to new signal source
  int    m_NewSource;  // Indication about new source prence
  double m_BetweenActivityTime; // if no activity for receive in (ms)
                                // then possible new source on opposite 
                                // side; necessary to send m_AutoEcho
	int    m_Port;
  char   m_InBuf[ 100 ];
  int    m_InBufCnt;
  char * m_OutBuf;
	int    m_OutBufLen; 
  int    m_NHandShaked;
  int    m_OutBufCnt; // 0 - finished, >0 - how many sent, <0 - error
  int    m_InOutBuf;

	static SerChanArray * m_Channels;
protected:
	int m_AutoFlush;
};



#endif // !defined(AFX_SERIO_H__4621CA55_464F_11D2_ACFA_00A0C9B00E4F__INCLUDED_)
