// CameraLink.h: interface for the CCameraLink class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAMERALINK_H__345652EF_D278_11D7_B2D4_0002DD30AC57__INCLUDED_)
#define AFX_CAMERALINK_H__345652EF_D278_11D7_B2D4_0002DD30AC57__INCLUDED_
#include "clserial.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define	CLINK_OP_CHAR_1	0xA5
#define	CLINK_READ_OP_CHAR	0x55
#define	CLINK_WRITE_OP_CHAR	0x5A
#define BIT_BOOT_FROM_USR		0x01

#define ENABLE  1
#define DISABLE 0

#define NORMAL  0
#define WINDOW  1
#define BINNING 2

class CCLSer
{
public:
	int Init(LPCTSTR szFileName);
	int Close();
	int CheckPort(UINT port);
	virtual int CheckConnectedCamera() = 0;
  virtual int GetExtTrigger( int& nExtTrigger) = 0;
  virtual int GetStrobePos( int& nStrobePos) = 0;
  //virtual int GetVertWin( int& nVWin);
  //virtual int GetLongInteg(int& m_LongIntTimeLow);
  //  virtual int GetVertWin(int& nHWin);
  virtual int GetDualMode(int& nDualMode) = 0;
  virtual int GetAnalogOffset(int& iTap,int& nAnalogOffSet)=0;
  virtual int GetBitDepth( int& nBitDepth) = 0;
  //virtual int GetNoiseCorr( int& nNoiseCorr);
  virtual int GetHorizMode( int& nHorizMode) = 0;
  virtual int GetVertMode( int& nVertMode) = 0;
  virtual int GetTestMode( int& nTestMode) = 0;
  virtual int GetTrigDuration( int& nTrigDuration) = 0;
  //virtual int GetCCInteg( int& nCCInteg);
  virtual int GetPreExp( int& nPreExp) = 0;
  //virtual int GetNegImage( int& nNegImage);
  //virtual int GetCurrTemper( int& nTemperature);
  //virtual int GetTemperAlarm( int& nTemperAlarm);
  virtual int GetFrameRate( int& nFrameRate) = 0;
  virtual int GetCamSpeed( double& nSpeed) = 0;
  //virtual int GetExp( int nExposure) = 0;
  virtual int SetExtTrigger( int nExtTrigger) = 0;
  virtual int SetStrobePos( int nStrobePos) = 0;
  //virtual int SetVertWin( int nVWin);
  //virtual int SetLongInteg(int m_LongIntTimeLow);
  // virtual int SetVertWin(int nHWin);
  virtual int SetDualMode(int nDualMode) = 0;
  virtual int SetAnalogOffset(int iTap,int nAnalogOffSet)=0;
  virtual int SetBitDepth( int nBitDepth) = 0;
  //virtual int SetNoiseCorr( int nNoiseCorr);
  virtual int SetHorizMode( int nHorizMode) = 0;
  virtual int SetVertMode( int nVertMode)   = 0;
  virtual int SetTestMode( int nTestMode)   = 0;
  virtual int SetTrigDuration( int nTrigDuration) = 0;
  //virtual int SetCCInteg( int nCCInteg);
  virtual int SetPreExp( int nPreExp) = 0;
  //virtual int SetNegImage( int nNegImage);
  //virtual int SetCurrTemper( int nTemperature);
  //virtual int SetTemperAlarm( int nTemperAlarm);
  virtual int SetFrameRate( int nFrameRate) = 0;
  
// 	virtual int InitCameraReg() = 0;
	virtual CString GetCameraName();
	virtual int SetShutterEnable(bool bEnable)=0;
	virtual int GetShutterEnable(bool& bEnable)=0;
	virtual int SetShutter(int nShut , char * cError)=0;
	virtual int GetShutter(int& nShut)=0;
	virtual int SetGain1(float nGain)=0;
	virtual int GetGain1(float& nGain)=0;
  virtual int ReadReg(BYTE Address,BYTE * Reg)=0;
  virtual int WriteReg(BYTE Address,BYTE Reg)=0;
	CCLSer();
	virtual ~CCLSer();

protected:
public:
	ptclSerWriteProc m_pfWrite;
	ptclSerReadProc	m_pfRead;
	ptclSerInitProc	m_pfInit;
	ptclSerCloseProc m_pfClose;
	HINSTANCE m_hSerialLib;
	CString m_dll;
	void * m_pInstance;
	ULONG m_ulPortNum;
	HANDLE m_HandleCOM;
protected:
	int Write(char *pcBuffer, unsigned long* pulBufSize);
	int Read(char* pcBuffer, unsigned long* pulBufSize);
	ULONG m_ulTimeout;
	bool m_bInitialized;
	DWORD m_CamID;
public:
	CString m_CamName;
};

#endif // !defined(AFX_CAMERALINK_H__345652EF_D278_11D7_B2D4_0002DD30AC57__INCLUDED_)
