#pragma once
#include "clser.h"

const DWORD LYNX_ID = 'LYNX';

class CCLSerLynx :
	public CCLSer
{
public:
	CCLSerLynx(void);
	virtual ~CCLSerLynx(void);
	virtual int CheckConnectedCamera();
// 	virtual int InitCameraReg() ;
	int WriteASCII(char* ascii, char* responce);
  virtual int GetExtTrigger( int& nExtTrigger);
  virtual int GetStrobePos( int& nStrobePos);
  //virtual int GetVertWin( int& nVWin);
  //virtual int GetLongInteg(int& m_LongIntTimeLow);
//  virtual int GetVertWin(int& nHWin);
  virtual int GetDualMode(int& nDualMode);
  virtual int GetAnalogOffset(int& iTap,int& nAnalogOffSet);
  virtual int GetBitDepth( int& nBitDepth);
  //virtual int GetNoiseCorr( int& nNoiseCorr);
  virtual int GetHorizMode( int& nHorizMode);
  virtual int GetVertMode( int& nVertMode);
  virtual int GetTestMode( int& nTestMode);
  virtual int GetTrigDuration( int& nTrigDuration);
  //virtual int GetCCInteg( int& nCCInteg);
  virtual int GetPreExp( int& nPreExp);
  //virtual int GetNegImage( int& nNegImage);
  //virtual int GetCurrTemper( int& nTemperature);
  //virtual int GetTemperAlarm( int& nTemperAlarm);
  virtual int GetFrameRate( int& nFrameRate);
  virtual int GetCamSpeed( double& nSpeed);
  //virtual int GetExp( int& nExposure);

  virtual int SetExtTrigger( int nExtTrigger);
  virtual int SetStrobePos( int nStrobePos);
  //virtual int SetVertWin( int nVWin);
  //virtual int SetLongInteg(int m_LongIntTimeLow);
 // virtual int SetVertWin(int nHWin);
  virtual int SetDualMode(int nDualMode);
  virtual int SetAnalogOffset(int iTap,int nAnalogOffSet);
  virtual int SetBitDepth( int nBitDepth);
  //virtual int SetNoiseCorr( int nNoiseCorr);
  virtual int SetHorizMode( int nHorizMode);
  virtual int SetVertMode( int nVertMode);
  virtual int SetTestMode( int nTestMode);
  virtual int SetTrigDuration( int nTrigDuration);
  //virtual int SetCCInteg( int nCCInteg);
  virtual int SetPreExp( int nPreExp);
  //virtual int SetNegImage( int nNegImage);
  //virtual int SetCurrTemper( int nTemperature);
  //virtual int SetTemperAlarm( int nTemperAlarm);
  virtual int SetFrameRate( int nFrameRate);
  //virtual int GetExp( int nExposure);
 
	virtual int SetShutterEnable(bool bEnable);
	virtual int GetShutterEnable(bool& bEnable);
	virtual int SetShutter(int nShut , char * cError);
	virtual int GetShutter(int & nShut);
	virtual int SetGain1(float nGain);
	virtual int GetGain1(float& nGain);
  virtual int ReadReg(BYTE Address,BYTE  *Reg);
  virtual int WriteReg(BYTE Address,BYTE Reg);
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


private:
	void SkipEscSequence(char *pcBuffer,int& ind);
};
