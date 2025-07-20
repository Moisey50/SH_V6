// IPCameraGadget.h : Declaration of the IPCameraGadget class


#ifndef __INCLUDE__IPCameraRenderH__
#define __INCLUDE__IPCameraRender_H__


#define MAX_IP_CAMERAS 64
#pragma once
#include "helpers\UserBaseGadget.h"
#include <helpers\SharedMemBoxes.h>
#include <helpers\InternalWatcher.h>

class AsyncOutThread : public CInternalWatcher
{
protected:
  FN_SENDINPUTDATA m_pFnSendInputData ;
  CGadget         * m_pHostGadget  ;
public:
  AsyncOutThread( CGadget * pHost ) ;
  virtual ~AsyncOutThread() ;
  virtual int ProcessData( CDataFrame * pDataFrame ) ;
};

class IPCamRender : public UserBaseGadget 
{
protected:
	AsyncOutThread * m_pAsyncOutThread ;
	IPCamRender();

private:
	 static   FXLockObject  m_GrabLock ;
	 static   FXLockObject  m_ConvertLock ;
     static   FXLockObject  m_SnapshotLock ;
	 static    bool     m_IPIndexes[MAX_IP_CAMERAS] ;
public:

	//	Mandatory functions
	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
	//bool PrintProperties(FXString& text);
    FXLockObject  m_Lock ;
	
	FXString m_sFileSavePath;
	FXString m_sCameraName;
	long    m_sTimeInterval_sec;

	time_t m_lastArrivedPacket;
	//
	unsigned char* m_pImage ;//= new unsigned char[pFrame->dwPacketSize]();
    long m_nPlaydecHandle;	//decode handle
	void Convert(FXString source, FXString dest);
	FXString GetNewFileName(FXString fname);
	int GetNextAvIndex(int index, int playhandle);
	void SaveImage();
	void StartRecord();
	void StopRecord();
	long m_handle;
	bool m_bRecord;
	HANDLE m_hRecThread;
	FXString m_LastRecorderFName;
	 bool			  m_bWasCreatedbyMe;
	 int m_nIndex;
	 int m_iCounter;
	 IPCamInfo  m_IPCamBusInfo;
	CSharedMemBoxes m_IPCamShared;
	void OpenStream();
	void SetHandle(HWND hWnd);
	void ShutDown(); 
	void DriverInit();
    FXString        m_GadgetInfo ; 
  LPCTSTR GetGadgetInfo() { return (LPCTSTR)m_GadgetInfo ; }
  static friend void CALLBACK fn_AsyncPutToDuplex(CDataFrame* pDataFrame, void* pGadget, CConnector* pOutConnector = NULL )
  {
    //Added check for null CTextFrame
    IPCamRender * pThisGadget = (IPCamRender *)pGadget ; 

    if ( !pOutConnector )
      pOutConnector = pThisGadget->GetDuplexConnector(0) ;
    if ( pOutConnector )
    {
      if ( ((CDuplexConnector*)pOutConnector)->Put( pDataFrame ) )
        return ;
    }
    pDataFrame->Release() ;
  }
	DECLARE_RUNTIME_GADGET(IPCamRender);
};

#endif	// __INCLUDE__IPCameraGadget_H__

