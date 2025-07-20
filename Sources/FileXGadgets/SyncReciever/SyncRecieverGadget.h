// UserExampleGadget.h : Declaration of the UserExampleGadget class



#ifndef __INCLUDE__SyncReceiverGadget_H__
#define __INCLUDE__SyncReceiverGadget_H__


#pragma once
#include "helpers\UserBaseGadget.h"
#include <map>
class SyncReceiverGadget : public UserBaseGadget
{
protected:

	SyncReceiverGadget();

public:

	FXLockObject m_lock;
	FXString m_msgToSend;

	char m_prefix;
	//FXString m_suffix;
	
	int m_length;
	int m_timeToWait;

	bool m_ListeningNow;
	UINT m_TimerId;
	HANDLE m_threadHandle;

	FXArray<int> m_asciiSufix;
	//	Mandatory functions

	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
	bool ParseSetMsg(FXString msg);
	void SendData(FXString msg, int pinN,bool raw = true);
	int Split(FXString str);
	
    void StopListener();
	bool AnalyzeData(FXString str);
	bool VerifySuffix(FXString str);
	DECLARE_RUNTIME_GADGET(SyncReceiverGadget);
};

#endif	// __INCLUDE__UserExampleGadget_H__

