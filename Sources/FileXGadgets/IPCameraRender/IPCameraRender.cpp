// IPCamRender.h.h : Implementation of the IPCamRender class


#include "StdAfx.h"
#include "IPCameraRender.h"
#define FPS 25
//USER_FILTER_RUNTIME_GADGET(IPCamRender,"IPCamRender");	//	Mandatory

#define THIS_MODULENAME "IPCameraRender.cpp"
IMPLEMENT_RUNTIME_GADGET_EX(IPCamRender, CFilterGadget, "Files.Render", TVDB400_PLUGIN_NAME);

UINT WINAPI RecordWacher(void* parameter);
FXLockObject                IPCamRender::m_GrabLock ;
FXLockObject                IPCamRender::m_ConvertLock ;
FXLockObject                IPCamRender::m_SnapshotLock ;
bool						IPCamRender::m_IPIndexes[MAX_IP_CAMERAS] = {FALSE};



AsyncOutThread::AsyncOutThread( CGadget * pHost )
{
  m_pFnSendInputData = fn_AsyncPutToDuplex ;
  m_pHostGadget = pHost ;
  Create() ;
  SetThreadName( _T("IPCameraToolAsyncThread") ) ;
  Resume() ;
}

AsyncOutThread::~AsyncOutThread()
{

}

int AsyncOutThread::ProcessData( CDataFrame * pDataFrame )
{
  if ( m_pFnSendInputData )
  {
    m_pFnSendInputData( pDataFrame , m_pHostGadget , NULL ) ;
    return TRUE ;
  }
  return FALSE ;
}

IPCamRender::IPCamRender()
{
	//	Mandatory
	m_nPlaydecHandle = -1;
	m_sFileSavePath = "";
	m_LastRecorderFName = "";
	m_sTimeInterval_sec = 30;
	m_bRecord = false;
	m_bWasCreatedbyMe = false;
	m_hRecThread=NULL;
	m_iCounter = 0;
	m_GadgetInfo = (_T("IPCameraRender"));
	init();	
    m_pAsyncOutThread = new AsyncOutThread( this ) ;
	HANDLE ghSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS,false,_T(H264SemaphoreName));
	if (ghSemaphore == NULL)
    {
		ghSemaphore = 
			CreateSemaphore( 
			NULL,           
			0,  
			MAX_SEM_COUNT,  
			_T("H264Semaphore")); 
	 }
	 else
	 {
	   DWORD err;
	    if (!ReleaseSemaphore( 
							ghSemaphore,  // handle to semaphore
							1,            // increase count by one
							NULL) )       // not interested in previous count
					{
					   err = GetLastError();
					}

 }
}

void IPCamRender::StartRecord()
{	
	if ( m_nPlaydecHandle < 0 || m_bRecord)
	{
		return;
	}
	m_GrabLock.Lock();
	FXString  cFilename;	
	CTime time = CTime::GetCurrentTime();
	int year = time.GetYear();
	int shortYear = year % 100; 
	cFilename.Format("%s\\%02d-%02d-%02d_%02d.%02d.%02d_%05d.h264", 
		m_sFileSavePath, 
		time.GetHour(), 
		time.GetMinute(), 
		time.GetSecond(), 
		time.GetMonth(), 
		time.GetDay(), 
		shortYear,
		m_iCounter);

	    m_iCounter++;
		//	m_sCameraName,
		//time.GetYear(), 
		//time.GetMonth(), 
		//time.GetDay(), 
		//time.GetHour(), 
		//time.GetMinute(), 
		//time.GetSecond());
	  
	    m_LastRecorderFName = cFilename;
		if ( H264_PLAY_StartDataRecord(m_nPlaydecHandle, cFilename.GetBuffer(0), MEDIA_FILE_NONE))
		{
			m_bRecord = TRUE;
			SEND_GADGET_TRACE("File '%s' was open for writing",m_LastRecorderFName);
		}
	m_GrabLock.Unlock();
}
UINT ConvertVideoFile(LPVOID parm)
{
  IPCamRender *pThis = ( IPCamRender* ) parm;
  pThis->Convert(pThis->m_LastRecorderFName, pThis->GetNewFileName(pThis->m_LastRecorderFName));
  return 0;
}
UINT WINAPI RecordWacher(void* parameter)
{
  IPCamRender* params = (IPCamRender*)parameter;
  while (params->m_bRecord)
  {
	    int secToSleep = (1000 * params->m_sTimeInterval_sec)+1;
	 	Sleep(secToSleep);
		params->StopRecord();
		Sleep(250);

		time_t now;
		time(&now);
		double seconds = difftime(now, params->m_lastArrivedPacket);
		if(seconds < 5)		
			params->StartRecord();
  }
  params->m_hRecThread = NULL;
  return 0;
}
void IPCamRender::StopRecord()
{	
	if ( m_bRecord )
	{
		m_GrabLock.Lock();
		if ( H264_PLAY_StopDataRecord(m_nPlaydecHandle) )
		{
			m_bRecord = FALSE;
			FXString sRequest;
			int iLength = (FPS * m_sTimeInterval_sec)+1;
		    sRequest.Format("Length=%d; Finished=%s;",iLength,m_LastRecorderFName);
            CTextFrame * ViewText = CTextFrame::Create( sRequest ); 
		    if ( m_pAsyncOutThread )
				m_pAsyncOutThread->Put( ViewText ) ;

			SEND_GADGET_TRACE("File '%s' was closed",m_LastRecorderFName);
			//Convert(m_LastRecorderFName, GetNewFileName(m_LastRecorderFName));
		}
		m_GrabLock.Unlock();
	}	
}
void IPCamRender::SaveImage()
{	
	if ( m_nPlaydecHandle < 0 )
	{
		return;
	}

    m_SnapshotLock.Lock();
	char cFilename[256];

	CTime time = CTime::GetCurrentTime();
	sprintf(cFilename, "%s\\%s_%4d%02d%02d_%02d%02d%02d.bmp", 
		(LPCTSTR)m_sFileSavePath, 
		(LPCTSTR)m_sCameraName,
		time.GetYear(), 
		time.GetMonth(), 
		time.GetDay(), 
		time.GetHour(), 
		time.GetMinute(), 
		time.GetSecond());

	if ( H264_PLAY_CatchPic(m_nPlaydecHandle, cFilename) )
	{
	}
	else
	{
		//MessageBox(("Desktop.SnapshotFail"));
	}
	m_SnapshotLock.Unlock();
}


FXString IPCamRender::GetNewFileName(FXString fname)
{
	FXString tempName = fname.MakeLower();
	FXString s = fname.TrimRight(".h264") + ".avi";
	return s;
}
void IPCamRender::Convert(FXString source, FXString dest)
{
	if ( source == "" || dest =="")
	{
		return;
	}
	
	if (strstr(source.GetBuffer(0), ".h264") || strstr(source.GetBuffer(0), ".H264"))
	{
		int Ret = H264_PLAY_ConvertFile((const char*)source.GetBuffer(0),(const char*)dest.GetBuffer(0),MEDIA_FILE_AVI,NULL,NULL);
	}
}
CDataFrame* IPCamRender::DoProcessing(const CDataFrame* pDataFrame) 
{	
	time(&m_lastArrivedPacket);
	if ((Tvdb400_IsEOS(pDataFrame)) )
	{		
		if(m_bRecord)
		{
			StopRecord();
		    DWORD dwExitcode=0;
			TerminateThread(m_hRecThread,dwExitcode);
			m_hRecThread=NULL;
			m_iCounter = 0;
		}
		if (m_nPlaydecHandle !=-1)
		{
			H264_PLAY_Stop(m_nIndex);
			H264_PLAY_CloseStream(m_nIndex);	
			m_IPIndexes[m_nPlaydecHandle]=false;
			m_IPCamShared.BoxClear(m_nIndex);
			m_nPlaydecHandle = -1;
			m_nIndex = -1;
		}			
		return nullptr;
	}	
    const CVideoFrame * pvf = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
	if(pvf==NULL)
		return nullptr;
	if(m_nPlaydecHandle == -1)
	{	
		int inIndex = pvf->lpBMIH->biClrUsed ;
		int iPlayhandle = pvf->lpBMIH->biClrImportant;
		m_nIndex = GetNextAvIndex(inIndex,iPlayhandle);
		SetHandle(0);
	
	}
	  StartRecord();
	  if (m_hRecThread==NULL)
	  {
		DWORD dwThreadID;
		m_hRecThread = CreateThread( 0, 0, (LPTHREAD_START_ROUTINE)RecordWacher, this , 0, &dwThreadID );
	  }


     //const CVideoFrame* Frame = pDataFrame->GetVideoFrame();
	 if (m_bRecord && (pvf) && (pvf->lpBMIH))
	 {
		  if (pvf->lpBMIH->biCompression == BI_H264)
		  {
			DWORD dwPacketSize =  pvf->lpBMIH->biSizeImage;
			unsigned char*  pImage = new unsigned char[dwPacketSize]();
			memcpy(pImage, (unsigned char*)&pvf->lpBMIH[1], dwPacketSize);
			H264_PLAY_InputData( m_nIndex , pImage, dwPacketSize);
			delete pImage;
		  }
	 }

	return nullptr;
}
int	 IPCamRender::GetNextAvIndex(int index, int playhandle)
{
	int sz = m_IPCamShared.GetBoxSize();
	m_IPCamBusInfo.m_ID = -1;
	m_IPCamBusInfo.m_iIndex = playhandle;
	int nextFree = -1;
	m_IPCamShared.SetToFirstFree( nextFree ,(BYTE*)&m_IPCamBusInfo, sz );
	m_IPCamBusInfo.m_ID = nextFree;
	m_IPCamBusInfo.m_iIndex = playhandle;
	m_IPCamShared.SetBoxContent(nextFree,(BYTE*)&m_IPCamBusInfo,sz);
	return nextFree;
	/*
	//int npos = atoi(fxLabel.MakeLower().Trim("cam"));
	//if(npos >= 0 && npos < MAX_IP_CAMERAS)
	//	return npos;
	for (int i = 0; i < MAX_IP_CAMERAS; i++)
	{
		if(m_IPIndexes[i] == false)
		{
			m_IPIndexes[i] = true;
			return i;
		}
	}
	return -1;
	*/
}

void IPCamRender::SetHandle(HWND hWnd)
{
	BYTE byFileHeadBuf;
	if (H264_PLAY_OpenStream(m_nIndex, &byFileHeadBuf, 1, SOURCE_BUF_MIN*50))
	{	
		H264_PLAY_SetStreamOpenMode(m_nIndex, STREAME_REALTIME);	
		if ( H264_PLAY_Play(m_nIndex,hWnd))
		{
			m_nPlaydecHandle = m_nIndex;
		}
		m_nPlaydecHandle = m_nIndex;
	}
}
void IPCamRender::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame) 
{
 if (!pParamFrame)
    return;

  CTextFrame * ParamText = pParamFrame->GetTextFrame(DEFAULT_LABEL);
  if (ParamText == NULL)
  {
    pParamFrame->Release( pParamFrame ) ;
    return;
  }
  /*
  if (ParamText->GetString().Find("Start_Record") > -1)
  {
	  StartRecord();
	  DWORD dwThreadID;
	  m_hRecThread = CreateThread( 0, 0, (LPTHREAD_START_ROUTINE)RecordWacher, this , 0, &dwThreadID );
  }
  else 
	  */
  if (ParamText->GetString().Find("Stop_Record") > -1 || ParamText->GetString().Find("set close(All)") > -1)
  {
	  StopRecord();
	  DWORD dwExitcode=0;	 
	  TerminateThread(m_hRecThread,dwExitcode);
	  m_hRecThread=NULL;
	  m_iCounter = 0;
  }
  else if (ParamText->GetString().Find("SaveImage") > -1)
  {
	  SaveImage();
  }
  else
  {
	//OSD_INFO_TXT osd2;
	//osd2.bkColor = RGB(255,0,0);
	//osd2.color = RGB(0,255,0);
	//osd2.pos_x = 10;
	//osd2.pos_y = 40;
	//osd2.isTransparent = 0;
	//osd2.isBold = 1;
	//strcpy(osd2.text, "test222 osd info"); 
	//H264_PLAY_SetOsdTex(m_nPlaydecHandle, &osd2);
  }
  pParamFrame->Release( pParamFrame );
};

void IPCamRender::PropertiesRegistration() 
{
	
	addProperty(SProperty::SPIN			,	_T("Interval")		,	&m_sTimeInterval_sec		,	SProperty::Long	,	1	,	5000	);
	addProperty(SProperty::EDITBOX  ,	_T("File")		,	&m_sFileSavePath		,	SProperty::String	);
	addProperty(SProperty::EDITBOX  ,	_T("CameraName")		,	&m_sCameraName		,	SProperty::String	);
}

void IPCamRender::ConnectorsRegistration() 
{
	addInputConnector( transparent, "StreamData");
	addDuplexConnector( transparent, transparent, "CameraControl");
};

void IPCamRender::ShutDown()
{	
	StopRecord();
	DWORD dwExitcode=0;	 
	TerminateThread(m_hRecThread,dwExitcode);
	 m_hRecThread=NULL;
	HANDLE ghSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS,false,_T(H264SemaphoreName));
    DWORD dwWaitResult = WaitForSingleObject( ghSemaphore, 0L);
    if (dwWaitResult == WAIT_TIMEOUT)
    {
		H264_DVR_Cleanup();
    }
	m_pAsyncOutThread->ShutDown() ;  
	delete m_pAsyncOutThread ;	
  UserGadgetBase::ShutDown();
}