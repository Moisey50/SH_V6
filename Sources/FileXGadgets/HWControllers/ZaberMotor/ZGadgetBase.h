#pragma once
#include "helpers\UserBaseGadget.h"
// #include "helpers\Locker.h"
#include "ZCommandsHeadersRepository.h"
#include "ZCommandFactory.h"
#include "ZDevicesRepository.h"


#pragma region | Logger Defines |
#define ZBR_SENDLOG_3(vrbsty,sz,a,b,c)  FxSendLogMsg(vrbsty,THIS_MODULENAME,0,sz,a,b,c)

#define ZBR_SENDINFO_3(sz,a,b,c)   ZBR_SENDLOG_3(MSG_INFO_LEVEL,sz,a,b,c)
#define ZBR_SENDINFO_2(sz,a,b)     ZBR_SENDINFO_3(sz,a,b,"")
#define ZBR_SENDINFO_1(sz,a)       ZBR_SENDINFO_2(sz,a,"")
#define ZBR_SENDINFO_0(sz)         ZBR_SENDINFO_1(sz,"")

#define ZBR_SENDTRACE_3(sz,a,b,c)  ZBR_SENDLOG_3(MSG_DEBUG_LEVEL,sz,a,b,c)
#define ZBR_SENDTRACE_2(sz,a,b)    ZBR_SENDTRACE_3(sz,a,b,"")
#define ZBR_SENDTRACE_1(sz,a)      ZBR_SENDTRACE_2(sz,a,"")
#define ZBR_SENDTRACE_0(sz)        ZBR_SENDTRACE_1(sz,"")

#define ZBR_SENDWARN_3(sz,a,b,c)   ZBR_SENDLOG_3(MSG_WARNING_LEVEL,sz,a,b,c)
#define ZBR_SENDWARN_2(sz,a,b)     ZBR_SENDWARN_3(sz,a,b,"")
#define ZBR_SENDWARN_1(sz,a)       ZBR_SENDWARN_2(sz,a,"")
#define ZBR_SENDWARN_0(sz)         ZBR_SENDWARN_1(sz,"")
								   
#define ZBR_SENDERR_3(sz,a,b,c)    ZBR_SENDLOG_3(MSG_ERROR_LEVEL,sz,a,b,c)
#define ZBR_SENDERR_2(sz,a,b)      ZBR_SENDERR_3(sz,a,b,"")
#define ZBR_SENDERR_1(sz,a)        ZBR_SENDERR_2(sz,a,"")
#define ZBR_SENDERR_0(sz)          ZBR_SENDERR_1(sz,"")
								   
#define ZBR_SENDFAIL_3(sz,a,b,c)   ZBR_SENDLOG_3(MSG_CRITICAL_LEVEL,sz,a,b,c)
#define ZBR_SENDFAIL_2(sz,a,b)     ZBR_SENDFAIL_3(sz,a,b,"")
#define ZBR_SENDFAIL_1(sz,a)       ZBR_SENDFAIL_2(sz,a,"")
#define ZBR_SENDFAIL_0(sz)         ZBR_SENDFAIL_1(sz,"")
#pragma endregion | Logger Defines |


class ZGadgetBase
	: public UserBaseGadget
{
private:
protected:
	static const ZCommandsHeadersRepository m_CommandsHeaders; //self-destructible repository
	static const ZCommandFactory            m_CommandsFactory;
  static       /*Locker*/ FXLockObject    m_Locker;
	static       ZDevicesRepozitory         m_Devices;         //self-destructible repository

public:


private:
protected:
public:
	ZGadgetBase(void);
	virtual ~ZGadgetBase(void);

private:
protected:
	void SendOut( COutputConnector* pOutConnector, const FXString &label, const FXString& data, long frameId = NOSYNC_FRAME )
	{
		if (pOutConnector)
		{
			CTextFrame* pTxtFrm=CTextFrame::Create(data);
			pTxtFrm->SetLabel(label);
			pTxtFrm->SetTime(GetGraphTime() * 1.e-3);
			pTxtFrm->ChangeId(frameId);

			if (!pOutConnector->Put(pTxtFrm))
				pTxtFrm->RELEASE(pTxtFrm);
		}
	}
public:
};

