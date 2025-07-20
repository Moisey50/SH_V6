// UserExampleGadget.h : Declaration of the UserExampleGadget class

#include <time.h> 
#ifndef __INCLUDE__VControlGadget_H__
#define __INCLUDE__VControlGadget_H__

#define  WAITING_EVENT             10
#define  SET_NAME_EVENT            11
#define  GET_LENGHT_EVENT          12
#define  GET_VFILE_POSITION_EVENT  13
#define  SET_FRAME_POSITION_EVENT  14
#define  SET_NAME_FROM_ARR_EVENT   15
#pragma once
#include "helpers\UserBaseGadget.h"
//#include <files\futils.h>

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


class VControlGadget : public UserBaseGadget
{
protected:
  typedef struct
  {
    FXString sFileName;
    int      iFileLength;
    time_t   TimeStamp;
    time_t   StartWrite;
    BOOL     bAlert;
  } VFile ;

  VControlGadget();
  AsyncOutThread * m_pAsyncOutThread ;

  FXArray <VFile> m_VFilesArr;
public:

  typedef struct PARAMS
  {
	FXString sSource;
	FXString sDestanation;
  }CopyParam;
  FXArray <CopyParam> m_CopyArr;


  int		   m_iMinutesToSave;
  int      m_iAlertMinutesToSave;
  FXString m_sFileSavePath;
  FXString m_sAlertFileSavePath;
  FXString m_sLabel;
	int      m_iCurrentState;
	
	FXString  m_sLastFName;
	int       m_iLastFLength;
	time_t    LastTime;
       
	int       m_iNCurrentFile;
	int       m_VFileCurrentPos;
	int       m_VFileToGo;
  int       m_iPlaySpeed;

	int       m_iDesirePosition;

  BOOL      m_bAlert;
  time_t    lastAlertTime;
  BOOL      m_bAskPosition;

  BOOL      m_bPlaybackRunning;
  BOOL      m_bContinuesPlayMode;

  FXLockObject m_ArrLock ;

  BOOL m_bFirstCleanDone;

  bool m_bErrorCoccured;
FXString m_fxAlertID;
	CInputConnector* m_pInputs[2];
	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
	int AnalyzeVFiles();
	void GetVFileNumAndPosition( );
	void SaveVFileNamesToFile();
	int  LoadVFileNames();
  void ShutDown(); 
  void SwitchToPlayback(bool bPlayback);
  void SwitchToContinuesPlayMode(bool bMode);
  void CloseFileStream();
  void StartStopRecord(bool bStartRecord);
  void GetFileDateandTime(VFile &vfile);
  time_t  ConvertStringToSytemTimeSt(FXString fxTime);
  FXString ConvertTimeToString(time_t _time);
  void ChangeViewWindow(FXString fxWindowHandle);
  void GetRelevantFileIndex(time_t st );
  DWORD GetTimeDifference( SYSTEMTIME &st1, SYSTEMTIME &st2 );
  int VControlGadget::SaveAlertFiles();
 void RemoveFileFormArray( FXString sLastFName );
 void ClearOldRecords();


 virtual bool ScanProperties(LPCTSTR text, bool& Invalidate) ;


  static friend void CALLBACK fn_AsyncPutToDuplex(CDataFrame* pDataFrame, void* pGadget, CConnector* pOutConnector = NULL )
  {
    //Added check for null CTextFrame
    VControlGadget * pThisGadget = (VControlGadget *)pGadget ; 

    if ( !pOutConnector )
      pOutConnector = pThisGadget->GetDuplexConnector(0) ;
    if ( pOutConnector )
    {
      if ( ((CDuplexConnector*)pOutConnector)->Put( pDataFrame ) )
        return ;
    }
    pDataFrame->Release() ;
  }

  DECLARE_RUNTIME_GADGET(VControlGadget);
};

#endif	// __INCLUDE__UserExampleGadget_H__

