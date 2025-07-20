#pragma once
#include "clser.h"

class CCLSerIPX :
	public CCLSer
{
public:
	CCLSerIPX(void);
	virtual ~CCLSerIPX(void);
public:
	int ReadReg(BYTE byteAddress, BYTE *pByteData);
	int WriteReg(BYTE byteAddress, BYTE byteData, DWORD dwDelay);
	virtual int CheckConnectedCamera();
// 	virtual int InitCameraReg();
  virtual int SetShutterEnable(bool bEnable);
  virtual int GetShutterEnable(bool& bEnable);
  virtual int SetShutter(short nShut);
  virtual int GetShutter(short& nShut);
  virtual int SetGain1(float nGain);
  virtual int GetGain1(float& nGain);
	BYTE m_CamCntl1;
	BYTE m_CamCntl2;
	BYTE m_ExtTrigger;
	BYTE m_StrobePosHigh;
	BYTE m_VWinTopLow;
	BYTE m_VWinTopHigh;
	BYTE m_VWinBottomLow;
	BYTE m_VWinBottomHigh;
	BYTE m_ShutTimeLow;
	BYTE m_ShutTimeHigh;
	BYTE m_LongIntTimeLow;
	BYTE m_LongIntTimeHigh;
	BYTE m_StrobePosLow;
	BYTE m_HWinSizeHigh;
	BYTE m_HWinSizeLeftLow;
	BYTE m_HWinSizeRightLow;


protected:
	UCHAR m_cWriteOp;
	UCHAR m_cReadOp;
	UCHAR m_cFirstOp;

};
