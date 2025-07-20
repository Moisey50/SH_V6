// WatchDogGadget.h : Declaration of the WatchDogGadget class



#ifndef __INCLUDE__WatchDogGadget_H__
#define __INCLUDE__WatchDogGadget_H__

#pragma once
#include "helpers\UserBaseGadget.h"

class WatchDogGadget : public UserBaseGadget
{
protected:

	WatchDogGadget();

public:
	~WatchDogGadget();
	CExecutionStatus* m_pStatus;
	int m_ihowMuchDelay_ms;
  int m_iPeriod_ms ;
	int m_iOutFramesCount;
	bool m_bSingleShotMode;
	BOOL m_bFlagTimeOut;
	HANDLE m_hTimer;
	LPCTSTR m_stringMessage ;  // tringMessage use for DoTextFrame() messages

	void DoTextFrame();
	BOOL DeleteTimerQueueFromWatchDogGadget();
	void  InitExecutionStatus(CExecutionStatus* Status);
	static const char* pList;

	//	Mandatory functions

	void PropertiesRegistration();
	void ConnectorsRegistration();


	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

	DECLARE_RUNTIME_GADGET(WatchDogGadget);
};



#endif	// __INCLUDE__WatchDogGadget_H__

