// UserExampleGadget.h.h : Implementation of the UserExampleGadget class


#include "StdAfx.h"
#include "VControlGadget.h"
#include <sys/stat.h>
USER_FILTER_RUNTIME_GADGET(VControlGadget, LINEAGE_FILEX );	//	Mandatory

#define  FRAME_RATE 30
#define  DEFAULT_FRAME_RATE 25
#define  FILES_TO_SAVE_ON_ALERT		3
static int VChannelCounter = 0;

AsyncOutThread::AsyncOutThread( CGadget * pHost )
{
  m_pFnSendInputData = fn_AsyncPutToDuplex ;
  m_pHostGadget = pHost ;
  Create() ;
  SetThreadName( _T("VControlAsyncThread") ) ;
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


DWORD WINAPI CopyThread(void* parameter)
{
  VControlGadget* params = (VControlGadget*)parameter;
  int isz = (int) params->m_CopyArr.GetSize();
  for(int i = 0 ; i < isz ; i++)
    CopyFile(params->m_CopyArr.GetAt(i).sSource,params->m_CopyArr.GetAt(i).sDestanation,TRUE);
  return 0;
}


VControlGadget::VControlGadget()
{
  m_iMinutesToSave = 20;
  m_iNCurrentFile = 0;
  m_bAlert = FALSE;
  m_iCurrentState = WAITING_EVENT;
  VChannelCounter++;
  m_bAskPosition = FALSE;
  m_bPlaybackRunning = FALSE;
  m_bContinuesPlayMode = FALSE;
  m_bErrorCoccured = FALSE;
  m_VFileCurrentPos = 0;
  m_bFirstCleanDone = false;
  //m_sAlertFileSavePath = _T("D:\\Video\\Alert");
  init();

  m_pAsyncOutThread = new AsyncOutThread( this ) ;
}

void VControlGadget::RemoveFileFormArray( FXString sLastFName )
{
   for ( int i = 0 ; i < m_VFilesArr.GetSize(); i++)
   {
     if (!sLastFName.CompareNoCase(m_VFilesArr.GetAt(i).sFileName))
     {  
       m_ArrLock.Lock();
       m_VFilesArr.RemoveAt(i);
       m_ArrLock.Unlock();
     }
   }
}
void VControlGadget::ClearOldRecords()
{
	CFileFind finder;
	BOOL bFound = finder.FindFile(m_sFileSavePath + _T("\\*.*"));
	while (bFound)
	{
	   bFound = finder.FindNextFile();
	   	if( (! finder.IsDots()) && ( !finder.IsDirectory() ) )		
	   {
		   FXString fxstr = m_sFileSavePath + "\\" +(LPCTSTR)finder.GetFileName();
		   int ret = remove( fxstr);
	   }
	}
}
/*
bool VControlGadget::PrintProperties(FXString& text)
{
	return false;
}
bool VControlGadget::ScanSettings(FXString& text)
{
	return false;
}
bool VControlGadget::ScanProperties( LPCTSTR text , bool& Invalidate )
{
	return false;
}
*/
CDataFrame* VControlGadget::DoProcessing(const CDataFrame* pDataFrame) 
{
  const CTextFrame * ParamText = pDataFrame->GetTextFrame(DEFAULT_LABEL);
  if(m_bErrorCoccured == TRUE)
  {
	  SwitchToPlayback(true);
	  SwitchToContinuesPlayMode(false);
    RemoveFileFormArray(m_sLastFName);
    m_bErrorCoccured = FALSE;
  }

  BOOL bAuto = FALSE;
  if (ParamText->GetString().Find("Auto") > -1)
    bAuto = TRUE;

  if (ParamText->GetString().Find("Closed") > -1)
  {
    FXSIZE iTok = 0;

    //FXString fxLength = ParamText->GetString().Tokenize(";", iTok); 
    FXString fxName = ParamText->GetString().Tokenize(";", iTok); 
    //if (iTok>-1)
    //  fxName = ParamText->GetString().Tokenize(";",iTok);

    iTok = 0;
    fxName.Tokenize("=",iTok);
    m_sLastFName = fxName.Tokenize("=",iTok);

    //ParamText->GetString().Tokenize("=", iTok); 
    //m_sLastFName = ParamText->GetString().Tokenize("=", iTok); 
    m_sLastFName.Remove(';');
    FXString sRequest;
    sRequest.Format("set FileName(%s)",m_sLastFName);
    CTextFrame * ViewText = CTextFrame::Create( sRequest ); 
    m_iCurrentState = SET_NAME_EVENT;
    time(&LastTime); 
    if ( m_pAsyncOutThread )
      m_pAsyncOutThread->Put( ViewText ) ;
//     if ( !m_pDuplexConnector->Put(ViewText))
//       ViewText->Release( ViewText ) ;

    Sleep(200);
    FXString fxOut = "FILE_NAME : "; 
    fxOut += m_sLastFName;
    ViewText = CTextFrame::Create( fxOut ); 
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;

    //switch live video to playback

    //SwitchToPlayback(true);
  }
  else if (ParamText->GetString().Find("Finished=")>-1)
  {
    time(&LastTime); 
    FXSIZE iTok = 0;
    FXString fxLength = ParamText->GetString().Tokenize(";", iTok); 
    FXString fxName;
    if (iTok>-1)
      fxName = ParamText->GetString().Tokenize(";",iTok);
    iTok = 0;
    fxLength.Tokenize("=",iTok);
    fxLength = fxLength.Tokenize("=",iTok);
    int iLength = atoi(fxLength);

    iTok = 0;
    fxName.Tokenize("=",iTok);
    fxName = fxName.Tokenize("=",iTok);

    VFile vfile;
    //vfile.bAlert = m_bAlert;
    vfile.iFileLength = iLength;
    vfile.sFileName = fxName;
    time(&LastTime); 
    vfile.TimeStamp = LastTime;

    GetFileDateandTime(vfile);
    m_ArrLock.Lock();
    m_VFilesArr.Add(vfile); 
    m_ArrLock.Unlock();

    m_sLastFName = fxName;

    AnalyzeVFiles();
    //m_bAlert = FALSE;

    if (!m_bPlaybackRunning)
    {
      m_iNCurrentFile = (int) m_VFilesArr.GetSize()-1;
     
      FXString sRequest;
      sRequest.Format("set FileName(%s)",m_sLastFName);
      CTextFrame * ViewText = CTextFrame::Create( sRequest ); 
  
      if ( m_pAsyncOutThread )
        m_pAsyncOutThread->Put( ViewText ) ;
//       if ( !m_pDuplexConnector->Put(ViewText))
//         ViewText->Release( ViewText ) ;

      Sleep(200);
      FXString fxOut = "FILE_NAME : "; 
      fxOut += m_sLastFName;
      ViewText = CTextFrame::Create( fxOut ); 
      if ( !m_pOutput->Put(ViewText))
        ViewText->Release( ViewText ) ;
    }
  }

  FXString fxPattern;
  fxPattern.Format("<Render=%s>",m_sLabel);
  if(ParamText->GetString().Find("All>") == -1 )
  {
    if(ParamText->GetString().Find(fxPattern) == -1 && !bAuto)
    {
      CVideoFrame* retV = NULL;
      return retV;
    }
  }

  if (ParamText->GetString().Find("Close File") > -1)
  {
    CloseFileStream();
  }
  else if (ParamText->GetString().Find("Closed") > -1)
  {
    FXSIZE iTok = 0;

    FXString fxLength = ParamText->GetString().Tokenize(";", iTok); 
    FXString fxName;
    if (iTok>-1)
      fxName = ParamText->GetString().Tokenize(";",iTok);

    iTok = 0;
    fxName.Tokenize("=",iTok);
    m_sLastFName = fxName.Tokenize("=",iTok);

    //ParamText->GetString().Tokenize("=", iTok); 
    //m_sLastFName = ParamText->GetString().Tokenize("=", iTok); 
    m_sLastFName.Remove(';');
    FXString sRequest;
    sRequest.Format("set FileName(%s)",m_sLastFName);
    CTextFrame * ViewText = CTextFrame::Create( sRequest ); 
    m_iCurrentState = SET_NAME_EVENT;
    time(&LastTime); 
    if ( m_pAsyncOutThread )
      m_pAsyncOutThread->Put( ViewText ) ;
//     if ( !m_pDuplexConnector->Put(ViewText))
//       ViewText->Release( ViewText ) ;

    Sleep(200);
    FXString fxOut = "FILE_NAME : "; 
    fxOut += m_sLastFName;
    ViewText = CTextFrame::Create( fxOut ); 
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;

  }
  else if (ParamText->GetString().Find("Alert") > -1)
  {
    if(m_bPlaybackRunning)
    {
        CVideoFrame* retV = NULL;
        return retV;
    }

    m_bAlert = TRUE;
    time(&lastAlertTime); 

    FXSIZE iTok = 0;
    FXString fxPar = ParamText->GetString().Tokenize("<",iTok);
    if (iTok!=-1)
      fxPar = ParamText->GetString().Tokenize("<",iTok);
    //if (iTok!=-1)
    //	fxPar = ParamText->GetString().Tokenize("<",iTok);

    fxPar.Trim("<>");
    iTok = 0;
    m_fxAlertID = fxPar.Tokenize("=",iTok);
    if (iTok!=-1)
      m_fxAlertID = fxPar.Tokenize("=",iTok);
    CloseFileStream();

  }
  else if(ParamText->GetString().Find("SetView") > -1)
  {
    FXSIZE iTok = 0;
    FXString fxPar = ParamText->GetString().Tokenize("<",iTok);
    if (iTok!=-1)
      fxPar = ParamText->GetString().Tokenize("<",iTok);
    if (iTok!=-1)
      fxPar = ParamText->GetString().Tokenize("<",iTok);

    fxPar.Trim("<>");
    iTok = 0;
    FXString fxHandle = fxPar.Tokenize("Window=",iTok);
    ChangeViewWindow(fxHandle);
  }
  else if(ParamText->GetString().Find("Playback") > -1)
  {
    SwitchToPlayback(true);
  }
  else if(ParamText->GetString().Find("Stop Record") > -1)
  {
    StartStopRecord(false);
  }
  else if(ParamText->GetString().Find("Start Record") > -1)
  {
    StartStopRecord(true);
  }
  else if(ParamText->GetString().Find("Back Start") > -1)
  {
    SwitchToContinuesPlayMode(false);
    if (m_VFilesArr.GetSize() > 0)
    {
      FXString sRequest;
      m_sLastFName = m_VFilesArr.GetAt(0).sFileName;
      sRequest.Format("set FileName(%s)",m_sLastFName);
      CTextFrame *ViewText = CTextFrame::Create( sRequest ); 
      if ( m_pAsyncOutThread )
        m_pAsyncOutThread->Put( ViewText ) ;
//       if ( !m_pDuplexConnector->Put(ViewText))
//         ViewText->Release( ViewText ) ;

      Sleep(150);  
      m_VFileCurrentPos = m_iDesirePosition = 0;
      sRequest.Format("set Position(%d)",m_iDesirePosition);
      ViewText = CTextFrame::Create( sRequest ); 
      if ( !m_pDuplexConnector->Put(ViewText))
        ViewText->Release( ViewText ) ;

      m_iNCurrentFile = 0;
    }
  }
  else if(ParamText->GetString().Find("Forw End") > -1)
  {
    SwitchToContinuesPlayMode(false);
    if (m_VFilesArr.GetSize() > 0)
    {
      int isz = (int) m_VFilesArr.GetSize();
      FXString sRequest;
      m_sLastFName = m_VFilesArr.GetAt(isz-1).sFileName;
      sRequest.Format("set FileName(%s)",m_sLastFName);
      CTextFrame *ViewText = CTextFrame::Create( sRequest ); 
      if ( m_pAsyncOutThread )
        m_pAsyncOutThread->Put( ViewText ) ;
//       if ( !m_pDuplexConnector->Put(ViewText))
//         ViewText->Release( ViewText ) ;

      Sleep(150);  
      m_VFileCurrentPos = m_iDesirePosition =  m_VFilesArr.GetAt(isz-1).iFileLength-1;
      sRequest.Format("set Position(%d)",m_iDesirePosition);
      ViewText = CTextFrame::Create( sRequest ); 
      if ( !m_pDuplexConnector->Put(ViewText))
        ViewText->Release( ViewText ) ;

      m_iNCurrentFile = isz - 1;
    }
  }
  else if(ParamText->GetString().Find("SetTime") > -1)
  {
    SwitchToContinuesPlayMode(false);
    FXString fxTime ;
    FXSIZE iTok = 0;
    FXString fxParam = ParamText->GetString();
    FXString fxstep = fxParam.Tokenize("=",iTok);
    fxstep.Trim("><Render");
    iTok = 0;
    fxTime = (fxstep.Tokenize("<", iTok));
    if (iTok!=-1)
      fxTime = (fxstep.Tokenize("<", iTok));


    time_t sTargetT = ConvertStringToSytemTimeSt(fxTime);   
    GetRelevantFileIndex(sTargetT);

    if(m_bContinuesPlayMode)
      SwitchToContinuesPlayMode(FALSE);

    FXString sRequest;
    sRequest.Format("set FileName(%s)",m_sLastFName);
    CTextFrame * ViewText = CTextFrame::Create( sRequest ); 
    m_iCurrentState = SET_NAME_FROM_ARR_EVENT;
    if ( m_pAsyncOutThread )
      m_pAsyncOutThread->Put( ViewText ) ;
//     if ( !m_pDuplexConnector->Put(ViewText))
//       ViewText->Release( ViewText ) ;


    if(m_bContinuesPlayMode)
      SwitchToContinuesPlayMode(FALSE);

    FXString fxOut = "GET_POSITION : "; 
    fxOut +=ParamText->GetString(); 
    ViewText = CTextFrame::Create( fxOut ); 
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;

    fxOut = "NEW_FILE_NAME : "; 
    fxOut += m_sLastFName; 
    ViewText = CTextFrame::Create( fxOut ); 
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;

    SwitchToPlayback(true);
  }
  else if(ParamText->GetString().Find("Play") > -1 && m_iCurrentState == WAITING_EVENT)
  {
    //FXSIZE iTok = 0;
    //atoi(ParamText->GetString().Tokenize(" ", iTok)); 
    //m_iPlaySpeed = atoi(ParamText->GetString().Tokenize(" ", iTok));
    m_bAskPosition = TRUE;   
    SwitchToContinuesPlayMode(TRUE);
  }
  else if (ParamText->GetString().Find("SetSpeed") > -1)
  {
    FXSIZE iTok = 0;
    FXString fxParam = ParamText->GetString();
    fxParam = fxParam.Tokenize("=",iTok);
    fxParam = fxParam.Trim("><Render");
    iTok = 0;
    double dSpeed = atof(fxParam.Tokenize("<", iTok));
    dSpeed = atof(fxParam.Tokenize("<", iTok));

    int iSpeed = ROUND(DEFAULT_FRAME_RATE * dSpeed);
    FXString fxFPS;
    fxFPS.Format("FrameRate=%d",iSpeed);
    CTextFrame * ViewText = CTextFrame::Create( fxFPS ); 
    ViewText->SetLabel("Control");
    if(!GetOutputConnector(7)->Put(ViewText))
      ViewText->Release(ViewText);

  }
  else if(ParamText->GetString().Find("Pause") > -1 && m_iCurrentState == WAITING_EVENT)
  {
    SwitchToContinuesPlayMode(FALSE);
  }
  else if ((ParamText->GetString().Find("Move") > -1 || ParamText->GetString().Find("ShowFrame") > -1) && m_iCurrentState == WAITING_EVENT )
  {
    if (bAuto)
    {
      FXSIZE iTok = 0;
      FXString fxParam = ParamText->GetString();
      FXString fxstep = fxParam.Tokenize("=",iTok);
      m_VFileToGo = atoi(fxParam.Tokenize("=",iTok)); 
    }
    else
    {
      FXSIZE iTok = 0;
      FXString fxParam = ParamText->GetString();
      FXString fxstep = fxParam.Tokenize("=",iTok);
      fxstep.Trim("><Render");
      iTok = 0;
      m_VFileToGo = atoi(fxstep.Tokenize("<", iTok));
      m_VFileToGo = atoi(fxstep.Tokenize("<", iTok));
    }

    //BOOL bContinuesPlay = m_bContinuesPlayMode;
    //if(bContinuesPlay && !bAuto)
    //  SwitchToContinuesPlayMode(FALSE);

    if ( m_bAskPosition || m_VFileCurrentPos == 0 )
    {
      m_iCurrentState = GET_VFILE_POSITION_EVENT;
      CTextFrame * ViewText = CTextFrame::Create( "get position" ); 
      if ( m_pAsyncOutThread )
        m_pAsyncOutThread->Put( ViewText ) ;
//       if ( !m_pDuplexConnector->Put(ViewText))
//         ViewText->Release( ViewText ) ;
      m_bAskPosition = FALSE;
    }
    else 
    {
      m_VFileCurrentPos = m_iDesirePosition ; //restore last position
      FXString fxCurrentFName = m_sLastFName ;
      GetVFileNumAndPosition();

      if (m_sLastFName.CompareNoCase(fxCurrentFName)!=NULL)
      {   
        //if(bContinuesPlay && bAuto)
        //  SwitchToContinuesPlayMode(FALSE);

        FXString sRequest;
        sRequest.Format("set FileName(%s)",m_sLastFName);
        CTextFrame * ViewText = CTextFrame::Create( sRequest ); 
        m_iCurrentState = SET_NAME_FROM_ARR_EVENT;
        if ( m_pAsyncOutThread )
          m_pAsyncOutThread->Put( ViewText ) ;
//         if ( !m_pDuplexConnector->Put(ViewText))
//           ViewText->Release( ViewText ) ;

        FXString fxOut = "NEW_FILE_NAME : "; 
        fxOut += m_sLastFName; 
        ViewText = CTextFrame::Create( fxOut ); 
        if ( !m_pOutput->Put(ViewText))
          ViewText->Release( ViewText ) ;


        Sleep(100);
        //if(bContinuesPlay && bAuto)
        //  SwitchToContinuesPlayMode(TRUE);

      }
      else if(m_VFileToGo !=1 ) //the same file
      {
        FXString sRequest;
        sRequest.Format("set Position(%d)",m_iDesirePosition);
        CTextFrame * ViewText = CTextFrame::Create( sRequest ); 
        if ( m_pAsyncOutThread )
          m_pAsyncOutThread->Put( ViewText ) ;
//         if ( !m_pDuplexConnector->Put(ViewText))
//           ViewText->Release( ViewText ) ;

        FXString fxOut = "NEW_FILE_NAME : "; 
        fxOut += m_sLastFName; 
        ViewText = CTextFrame::Create( fxOut ); 
        if ( !m_pOutput->Put(ViewText))
          ViewText->Release( ViewText ) ;

        sRequest.Format("new Position(%d)",m_iDesirePosition);
        ViewText = CTextFrame::Create( sRequest ); 
        if ( !m_pOutput->Put(ViewText))
          ViewText->Release( ViewText ) ;

        Sleep(150);
      }
      else if(bAuto)
      {
        CTextFrame * ViewText = CTextFrame::Create( "Clicked" ); 
        if(!GetOutputConnector(6)->Put(ViewText))
          ViewText->Release(ViewText);
      }
    }

    if (!bAuto)
    {
      Sleep(150);
      //m_iDesirePosition--;
      FXString fxOut;
      fxOut.Format("GoTo : %d", m_VFileToGo);
      CTextFrame * ViewText = CTextFrame::Create( fxOut ); 
      if ( !m_pOutput->Put(ViewText))
        ViewText->Release( ViewText ) ;

      ViewText = CTextFrame::Create( "Clicked" ); 
      if(!GetOutputConnector(6)->Put(ViewText))
        ViewText->Release(ViewText);
    }

    //if (ParamText->GetString().Find("ShowFrame") > -1)
    //{
    //m_iDesirePosition++; //trigger moves 1 frame forward        
    //FXString sRequest;
    //sRequest.Format("new Position(%d)",m_iDesirePosition);
    // CTextFrame *ViewText = CTextFrame::Create( sRequest ); 
    //if ( !m_pOutput->Put(ViewText))
    //  ViewText->Release( ViewText ); 
    //}
    //if(bContinuesPlay && !bAuto)
    //  SwitchToContinuesPlayMode(TRUE);

    //m_bContinuesPlayMode = bContinuesPlay;
  }
  else if(ParamText->GetString().Find("Live") > -1 && m_iCurrentState == WAITING_EVENT)
  {
    if (m_bContinuesPlayMode)
      SwitchToContinuesPlayMode(FALSE);
    SwitchToPlayback(false);
  }
  else if (ParamText->GetString().Find("Get Channels") > -1)
  {
    FXString fxOut;
    fxOut.Format("%d Channels", VChannelCounter);
    CTextFrame * ViewText = CTextFrame::Create( fxOut ); 
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;
  }

  CVideoFrame* retV = NULL;
  return retV;
}

void VControlGadget::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame) 
{
  CTextFrame * ParamText = pParamFrame->GetTextFrame(DEFAULT_LABEL);
  if (ParamText == NULL)
  {
    pParamFrame->Release( pParamFrame ) ;
    //m_LastID = m_CurrentID;
    return;
  }


  FXString str = ParamText->GetString();

  if (str.MakeLower().Find("error") > -1)
  {
      //m_bErrorCoccured = TRUE;
	  //m_iCurrentState = WAITING_EVENT;
	  //SwitchToPlayback(true);
	  //SwitchToContinuesPlayMode(false);
  }

  FXParser pk=(LPCTSTR)ParamText->GetString();
  FXString cmd,param;
  FXSIZE pos=0;
  pk.GetWord(pos,cmd);
  if ((cmd.CompareNoCase("list")==0) || (cmd.CompareNoCase("help")==0) || (cmd.CompareNoCase("?")==0))
  {
    pk="ClearFile\r\n\t"
      "SaveToFile\r\n\t"
      "ClearBuffer\r\n\t"
      "LoadFiles\r\n\t"
      "GetFileNames\r\n\t"
      "DeleteFile";
  }


  if (ParamText->GetString().Find("ClearFile") > -1)
  {
    remove  (m_sFileSavePath);
    FXString fxOut = "ClearFile"; 
    CTextFrame * ViewText = CTextFrame::Create( fxOut ); 
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;
    //return;

  }
  else if (ParamText->GetString().Find("SaveToFile") > -1)
  {
    SaveVFileNamesToFile();
    FXString fxOut = "SaveToFile"; 
    CTextFrame * ViewText = CTextFrame::Create( fxOut ); 
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;
    //return;
  }
  else if (ParamText->GetString().Find("ClearBuffer") > -1)
  {
    m_ArrLock.Lock();
    m_VFilesArr.RemoveAll();
    m_ArrLock.Unlock();

    FXString fxOut = "ClearBuffer"; 
    CTextFrame * ViewText = CTextFrame::Create( fxOut ); 
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;

    //return;
  }
  else if (ParamText->GetString().Find("LoadFiles") > -1)
  {
    if(LoadVFileNames() > 0 )
    {
      m_sLastFName = m_VFilesArr.GetAt(0).sFileName;
      m_iLastFLength = m_VFilesArr.GetAt(0).iFileLength;	
      m_iNCurrentFile = 0;
      FXString sRequest;
      sRequest.Format("set FileName(%s)",m_sLastFName);
      CTextFrame * ViewText = CTextFrame::Create( sRequest ); 
      time(&LastTime); 
      if ( m_pAsyncOutThread )
        m_pAsyncOutThread->Put( ViewText ) ;
//       if ( !m_pDuplexConnector->Put(ViewText))
//         ViewText->Release( ViewText ) ;

      Sleep(200);
    }

    FXString fxOut = "LoadFiles"; 
    CTextFrame * ViewText = CTextFrame::Create( fxOut ); 
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;
    //return;
  }
  else if (ParamText->GetString().Find("GetFileNames") > -1)
  {
    FXString fxOut("");
    for (FXSIZE i = 0 ; i < m_VFilesArr.GetSize() ; i++)
    {
      fxOut = fxOut + m_VFilesArr.GetAt(i).sFileName;
      fxOut  = fxOut + ";";
    }
    CTextFrame * ViewText = CTextFrame::Create( fxOut ); 
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;
    //return;
  }
  else if (ParamText->GetString().Find("DeleteFile") > -1)
  {
    FXSIZE iTok = 0;
    ParamText->GetString().Tokenize(" ", iTok); 
    FXString fxFiletoDelete = ParamText->GetString().Tokenize(" ", iTok); 
    if (fxFiletoDelete.GetLength() < 4)
    {
      pParamFrame->Release( pParamFrame ) ;
      return;    
    }
    FXString fxStatus;
    if(remove( fxFiletoDelete ) == 0)
    {
      for (FXSIZE i = 0 ; i < m_VFilesArr.GetSize() ; i++)
      {
        FXSIZE iret = m_VFilesArr.GetAt(i).sFileName.CompareNoCase(fxFiletoDelete);
        if(m_VFilesArr.GetAt(i).sFileName.CompareNoCase(fxFiletoDelete) == 0)
        {
          m_ArrLock.Lock();
          m_VFilesArr.RemoveAt(i);
          m_ArrLock.Unlock();
        }
      }

      if (m_sLastFName.CompareNoCase(fxFiletoDelete) == 0)
      {
        m_sLastFName = m_VFilesArr.GetAt(0).sFileName;
        m_iLastFLength = m_VFilesArr.GetAt(0).iFileLength;
        m_iNCurrentFile = 0;
        FXString fxOut  = "New File is " + m_sLastFName;
        CTextFrame * ViewText = CTextFrame::Create( fxOut ); 
        m_pOutput->Put(ViewText);
      }
      fxStatus  = fxFiletoDelete + " deleted";
    }
    else
      fxStatus = "Can't delete" + fxFiletoDelete; 

    CTextFrame * ViewText = CTextFrame::Create( fxStatus ); 
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;
    //return;
  }
  else if (m_iCurrentState == SET_NAME_EVENT)
  {
    if (ParamText->GetString().Find("Error") > -1)
    {
      m_iCurrentState = WAITING_EVENT;
    }
    else if (ParamText->GetString().Find("OK") > -1)
    {
      FXString sRequest = "get Length";
      CTextFrame * ViewText = CTextFrame::Create( sRequest ); 
      m_iCurrentState = GET_LENGHT_EVENT;
      if ( m_pAsyncOutThread )
        m_pAsyncOutThread->Put( ViewText ) ;
//       if ( !m_pDuplexConnector->Put(ViewText))
//         ViewText->Release( ViewText ) ;
    }
    FXString fxOut = "SET_NAME : "; 
    fxOut +=ParamText->GetString(); 
    CTextFrame * ViewText = CTextFrame::Create( fxOut ); 
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;
    //return;
  }
  else if (m_iCurrentState == GET_LENGHT_EVENT)
  {
    m_iLastFLength = atoi (ParamText->GetString());
    VFile  vfile;
    //vfile.bAlert = m_bAlert;
    vfile.iFileLength = m_iLastFLength;
    vfile.sFileName = m_sLastFName;
    vfile.TimeStamp = LastTime; 
    GetFileDateandTime(vfile);
    m_ArrLock.Lock();
    m_VFilesArr.Add(vfile);
    m_ArrLock.Unlock();
    AnalyzeVFiles();
    m_iNCurrentFile = (int) m_VFilesArr.GetSize()-1;
    //m_bAlert = FALSE;

    if (m_bAlert)
    {
      SaveAlertFiles();
      m_bAlert = FALSE;
    }

    FXString sRequest;
    sRequest.Format("set Position(%d)",m_iLastFLength - 1);
    CTextFrame * ViewText = CTextFrame::Create( sRequest ); 
    if ( m_pAsyncOutThread )
      m_pAsyncOutThread->Put( ViewText ) ;
//     if ( !m_pDuplexConnector->Put(ViewText))
//       ViewText->Release( ViewText ) ;
    m_iCurrentState = SET_FRAME_POSITION_EVENT;

    FXString fxOut = "GET_LENGTH : "; 
    fxOut +=ParamText->GetString(); 
    ViewText = CTextFrame::Create( fxOut ); 
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;
    //return;
  }
  else if (m_iCurrentState == GET_VFILE_POSITION_EVENT)
  {
    FXString fxOut = "CURRENT_FILE_NAME : "; 
    fxOut += m_sLastFName; 
    CTextFrame * ViewText = CTextFrame::Create( fxOut ); 
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;

    m_VFileCurrentPos = atoi (ParamText->GetString());
    GetVFileNumAndPosition();

    FXString sRequest;
    sRequest.Format("set FileName(%s)",m_sLastFName);
    ViewText = CTextFrame::Create( sRequest ); 
    m_iCurrentState = SET_NAME_FROM_ARR_EVENT;
    if ( m_pAsyncOutThread )
      m_pAsyncOutThread->Put( ViewText ) ;
//     if ( !m_pDuplexConnector->Put(ViewText))
//       ViewText->Release( ViewText ) ;

    Sleep(200);

    fxOut = "GET_POSITION : "; 
    fxOut +=ParamText->GetString(); 
    ViewText = CTextFrame::Create( fxOut ); 
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;

    fxOut = "NEW_FILE_NAME : "; 
    fxOut += m_sLastFName; 
    ViewText = CTextFrame::Create( fxOut ); 
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;

    //CBooleanFrame* pControl = CBooleanFrame::Create(true);
    //GetOutputConnector(2)->Put(pControl);
    //return;
  }	
  else if (m_iCurrentState == SET_NAME_FROM_ARR_EVENT)
  {
    FXString sRequest;
    sRequest.Format("set Position(%d)",m_iDesirePosition);
    CTextFrame * ViewText = CTextFrame::Create( sRequest ); 
    if ( m_pAsyncOutThread )
      m_pAsyncOutThread->Put( ViewText ) ;
//     if ( !m_pDuplexConnector->Put(ViewText))
//       ViewText->Release( ViewText ) ;
    m_iCurrentState = SET_FRAME_POSITION_EVENT;

    FXString fxOut;// = "SET_POSITION : %d"; 
    fxOut.Format("SET_POSITION : %d", m_iDesirePosition);
    ViewText = CTextFrame::Create( fxOut ); 
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;
    //return;
  }
  else if (m_iCurrentState == SET_FRAME_POSITION_EVENT )
  {
    FXString fxOut = "SET_POSITION : "; 
    fxOut +=ParamText->GetString(); 
    CTextFrame * ViewText = CTextFrame::Create( fxOut ); 
    m_iCurrentState = WAITING_EVENT;
    if ( !m_pOutput->Put(ViewText))
      ViewText->Release( ViewText ) ;	
    //return;
  }

  //return;

  FXString fxOut;
  fxOut.Format("LastCommandAns:%s", ParamText->GetString());
  CTextFrame * ViewText = CTextFrame::Create( fxOut ); 
  //ViewText->SetLabel(m_sLabel);
  if ( !m_pOutput->Put(ViewText))
    ViewText->Release( ViewText ) ;

  pParamFrame->Release( pParamFrame ) ;
  return ;
};

/*
int VControlGadget::AnalyzeVFiles()
{
  int iNRemoved = 0;
  time_t last = m_VFilesArr.GetAt(m_VFilesArr.GetSize()-1).TimeStamp;
  //for (int i = 0 ; i < m_VFilesArr.GetSize() ; i++)
  //{
    double idifftime = difftime (last,m_VFilesArr.GetAt(0).TimeStamp);
    if (((difftime (last,m_VFilesArr.GetAt(0).TimeStamp))  > m_iMinutesToSave * 60) && m_sLastFName.CompareNoCase(m_VFilesArr.GetAt(0).sFileName)!=NULL)
    {
      if(remove( m_VFilesArr.GetAt(0).sFileName ) == 0)
      {
        m_VFilesArr.RemoveAt(0);
        iNRemoved++;
        m_iNCurrentFile--;
      }
    }
  //}
  return iNRemoved;
}
 */
bool VControlGadget::ScanProperties( LPCTSTR text , bool& Invalidate )
{
	bool ret = UserGadgetBase::ScanProperties(text,Invalidate);
	if (!m_bFirstCleanDone)
	{
	ClearOldRecords();
		m_bFirstCleanDone = true;
	}	
	return ret;
}
int VControlGadget::AnalyzeVFiles()
{
  int iNRemoved = 0;
  time_t last = m_VFilesArr.GetAt(m_VFilesArr.GetSize()-1).TimeStamp;
  for (int i = 0 ; i < m_VFilesArr.GetSize() ; i++)
  {
    double idifftime = difftime (last,m_VFilesArr.GetAt(i).TimeStamp);
    int uuu = m_sLastFName.CompareNoCase(m_VFilesArr.GetAt(i).sFileName);
	FXString fxname = m_VFilesArr.GetAt(i).sFileName;
    if (((difftime (last,m_VFilesArr.GetAt(i).TimeStamp)) >= m_iMinutesToSave * 60) && (m_sLastFName.CompareNoCase(m_VFilesArr.GetAt(i).sFileName) != 0))
    {
      if(remove( m_VFilesArr.GetAt(i).sFileName ) == 0)
      {
        m_ArrLock.Lock();
        m_VFilesArr.RemoveAt(i);
        m_ArrLock.Unlock();
        iNRemoved++;
        //m_iNCurrentFile--;
        //ASSERT(m_iNCurrentFile>=0);
      }   
    }
    //return iNRemoved;
  }
  return iNRemoved;
}


void VControlGadget::GetVFileNumAndPosition()
{
  if (m_VFilesArr.GetSize() < 1)
    return;
  int iRes = 0;
  if (m_VFileToGo > 0)
  {
    iRes = m_VFilesArr.GetAt(m_iNCurrentFile).iFileLength - m_VFileCurrentPos;
    if (iRes >= m_VFileToGo)
    {
      m_iDesirePosition = m_VFileCurrentPos + m_VFileToGo;
      return;
    }
    for ( int i = m_iNCurrentFile + 1 ; i < m_VFilesArr.GetSize(); i++)
    {
      iRes+= m_VFilesArr.GetAt(i).iFileLength;
      if (iRes  >= m_VFileToGo)
      {
        m_iNCurrentFile = i;
        m_sLastFName = m_VFilesArr.GetAt(i).sFileName;
        m_iDesirePosition = m_VFileToGo - (iRes -  m_VFilesArr.GetAt(m_iNCurrentFile).iFileLength);
        break;
      }		
    }
  }
  else
  {
    iRes = m_VFileCurrentPos;
    if (iRes >= abs(m_VFileToGo))
    {
      m_iDesirePosition = m_VFileCurrentPos + m_VFileToGo;
      return;
    }
    for ( int i = m_iNCurrentFile - 1 ; i >= 0; i--)
    {
      iRes+= m_VFilesArr.GetAt(i).iFileLength;
      if (iRes  >= abs(m_VFileToGo))
      {
        m_iNCurrentFile = i;
        m_sLastFName = m_VFilesArr.GetAt(i).sFileName;
        m_iDesirePosition = iRes + m_VFileToGo;
        break;
      }
    }
  }
  ASSERT(m_iDesirePosition > 0);
}

void VControlGadget::PropertiesRegistration() 
{
  addProperty(SProperty::SPIN			,	_T("MinutesToSave")		,	&m_iMinutesToSave	,	SProperty::Long	,	0		,	240	);
  addProperty(SProperty::EDITBOX  ,	_T("SavedFilePath")		,	&m_sFileSavePath			,	SProperty::String	);
  addProperty(SProperty::EDITBOX  ,	_T("Label")		        ,	&m_sLabel					,	SProperty::String	);
  addProperty(SProperty::EDITBOX  ,	_T("AlertFilePath")		,	&m_sAlertFileSavePath		,	SProperty::String	);
  //addProperty(SProperty::SPIN			,	_T("MinutesToSaveonAlert")		,	&m_iAlertMinutesToSave		,	SProperty::Long	,	0		,	30	);
};

void VControlGadget::ConnectorsRegistration() 
{
  addInputConnector( text, "VFileName");
  addOutputConnector( text , "Status");
  addOutputConnector( logical , "Source");
  addOutputConnector( text , "TriggerControl");
  addOutputConnector( text , "RenderControl");
  addOutputConnector( text , "RecordControl");
  addOutputConnector( text , "FileCloser");
  addOutputConnector( text , "SingleStepTrigger");
  addOutputConnector( text , "PlaySpeedControl");
  addDuplexConnector( transparent, transparent, "Control");
};


//void VControlGadget::ShutDown()
//{
//  CGadget::ShutDown();
//SaveVFileNamesToFile();
//   CDataFrame * pFr ;
//   while ( m_pInputs[0]->Get( pFr ) )
// 	  pFr->Release( pFr )  ;
// 
//   delete m_pInputs[0];
//   m_pInput = m_pInputs[0] = NULL;
//   delete m_pOutput;
//   m_pOutput = NULL;
//   delete m_pDuplexConnector;
//   m_pDuplexConnector = NULL;
//}


FXString VControlGadget::ConvertTimeToString(time_t _tTime)
{
  FXString fxTime;
  struct tm tTime;
  tTime = *localtime(&_tTime);
  fxTime.Format("%d:%d:%d:%d:%d:%d",tTime.tm_hour, 
    tTime.tm_min, 
    tTime.tm_sec, 
    tTime.tm_mday,
    tTime.tm_mon, 
    tTime.tm_year);
  return fxTime;
}
void VControlGadget::SaveVFileNamesToFile()
{
  FILE * FilePtr;
  if((FilePtr=fopen(m_sFileSavePath,"a"))!=NULL)
  {
    for (int i = 0; i < m_VFilesArr.GetSize(); i++)
    {
      FXString fxOut;
      FXString fxStart = ConvertTimeToString(m_VFilesArr.GetAt(i).StartWrite);
      FXString fxStop = ConvertTimeToString(m_VFilesArr.GetAt(i).TimeStamp);
      fxOut.Format("%s,%d,%s,%s\n",m_VFilesArr.GetAt(i).sFileName,m_VFilesArr.GetAt(i).iFileLength,fxStart,fxStop);
      fputs(fxOut,FilePtr);
    }
    fclose(FilePtr);
  }
}

int  VControlGadget::LoadVFileNames()
{
  int icounter = 0;
  FILE  *FilePtr;

  if((FilePtr=fopen(m_sFileSavePath,"r"))!=NULL)
  {
    char buf[400] ;
    while ( fgets( buf, 400, FilePtr) )
    {
      FXString fx(buf);
      fx.Trim("\n");
      VFile  vfile;
      FXSIZE itok = 0;
      vfile.sFileName = fx.Tokenize(",",itok);
      if (itok!=-1)
        vfile.iFileLength = atoi(fx.Tokenize(",",itok));
      if (itok)
      {
      }
      time(&LastTime); 
      vfile.TimeStamp = LastTime;	
      icounter++;
      vfile.bAlert = FALSE;
      m_ArrLock.Lock();
      m_VFilesArr.Add(vfile);	
      m_ArrLock.Unlock();
      FXString fxOut = "FILE_NAME : "; 
      fxOut += vfile.sFileName; 
      CTextFrame * ViewText = CTextFrame::Create( fxOut ); 
      m_pOutput->Put(ViewText);
    }
    fclose(FilePtr);
  }
  return icounter;
}

void VControlGadget::CloseFileStream()
{
  FXString fxCloseStream;
  fxCloseStream.Format("set close(All)");
  CTextFrame * ViewText = CTextFrame::Create( fxCloseStream ); 
  if(!GetOutputConnector(5)->Put(ViewText))
    ViewText->Release(ViewText);

  Sleep(50);
}

void VControlGadget::SwitchToPlayback(bool bPlayback)
{
  m_bPlaybackRunning = bPlayback;
  CBooleanFrame* pBool = CBooleanFrame::Create(bPlayback);
  if(!GetOutputConnector(1)->Put(pBool))
    pBool->Release(pBool);
  Sleep(100);
}

void VControlGadget::SwitchToContinuesPlayMode(bool bMode)
{
  Sleep(50);
  m_bContinuesPlayMode = bMode;
  CBooleanFrame* pBool = CBooleanFrame::Create(bMode);
  if(!GetOutputConnector(2)->Put(pBool))
    pBool->Release(pBool);
  Sleep(100);
}

void VControlGadget::StartStopRecord(bool bStartRecord)
{
  CBooleanFrame* pBool = CBooleanFrame::Create(bStartRecord);
  if(!GetOutputConnector(4)->Put(pBool))
    pBool->Release(pBool);

  Sleep(100);
}

void VControlGadget::ChangeViewWindow(FXString fxWindowHandle)
{
  FXString fxHandle;
  fxHandle.Format("set hTargetWindow(%s)",fxWindowHandle);
  CTextFrame * ViewText = CTextFrame::Create( fxHandle ); 
  ViewText->SetRegistered();//241115
  if(!GetOutputConnector(3)->Put(ViewText))
    ViewText->Release(ViewText);

  Sleep(100);
}

void VControlGadget::ShutDown() 
{ 
  UserGadgetBase::ShutDown();
  m_pAsyncOutThread->ShutDown() ;
  delete m_pAsyncOutThread ;
  VChannelCounter--;
}


void VControlGadget::GetFileDateandTime(VFile &vfile)
{  
  struct stat fileInfo;
  stat(vfile.sFileName, &fileInfo);

  //   struct tm tCreation,tLastWrite;
  //   tCreation = *localtime(&fileInfo.st_atime);
  //   tLastWrite = *localtime(&fileInfo.st_mtime);

  vfile.StartWrite = fileInfo.st_atime;

  //   HANDLE  hFile = CreateFile(vfile.sFileName, GENERIC_READ, //open for reading
  //     FILE_SHARE_READ, //share for reading
  //     NULL, //default security
  //     OPEN_EXISTING, //existing file only
  //     FILE_ATTRIBUTE_NORMAL, //normal file
  //     NULL); //no attribute template
  // 
  //   if (hFile != INVALID_HANDLE_VALUE)
  //   {
  //     FILETIME ftCreation, ftLastaccess, ftLastwrite;
  //     SYSTEMTIME stCreation, stLastaccess, stLastwrite;
  // 
  //     if (GetFileTime(hFile, &ftCreation, &ftLastaccess, &ftLastwrite)
  //       && FileTimeToSystemTime(&ftCreation, &stCreation)
  //       && FileTimeToSystemTime(&ftLastaccess, &stLastaccess)
  //       && FileTimeToSystemTime(&ftLastwrite, &stLastwrite)) 
  //     {
  //         vfile.stCreation = stCreation;
  //         vfile.stLastwrite = stLastwrite;
  //     }
  //     else
  //     {
  //       //Error code
  //     }
  //     CloseHandle(hFile);
  //   }
}

time_t VControlGadget::ConvertStringToSytemTimeSt(FXString fxTime)
{
  FXSIZE iTok = 0;
  FXString fxHour,fxMinute,fxSecond,fxYear,fxMonth,fxDay;  
  fxHour = fxTime.Tokenize(":",iTok);
  if (iTok!=-1)
    fxMinute = fxTime.Tokenize(":",iTok);
  if (iTok!=-1)
    fxSecond = fxTime.Tokenize(":",iTok);
  if (iTok!=-1)
    fxDay = fxTime.Tokenize(":",iTok);
  if (iTok!=-1)
    fxMonth = fxTime.Tokenize(":",iTok);
  if (iTok!=-1)
    fxYear = fxTime.Tokenize(":",iTok);

  struct tm sTime;

  sTime.tm_hour = atoi(fxHour);
  sTime.tm_min = atoi(fxMinute);
  sTime.tm_sec = atoi(fxSecond);
  sTime.tm_mday = atoi(fxDay);
  sTime.tm_mon = atoi(fxMonth) - 1;
  sTime.tm_year = atoi(fxYear) + 100 ;

  time_t ret = mktime(&sTime);
  return ret;

}

int VControlGadget::SaveAlertFiles()
{
  m_CopyArr.RemoveAll();
  int icounter = 0;
  int isz = (int) m_VFilesArr.GetSize();
  if (isz == 0)
    return 0;
  int icurretnIndex = isz - 1;
  while (true)
  {
    FXString fxId;
    fxId.Format("_%d",FILES_TO_SAVE_ON_ALERT - icounter);
    FXString fxDestFilePath = m_fxAlertID + fxId +".avi";
    fxDestFilePath = m_sAlertFileSavePath + "\\" + fxDestFilePath;
    //if (CopyFile(m_VFilesArr.GetAt(icurretnIndex).sFileName,fxDestFilePath,TRUE) != 0)
    CopyParam CopyParam;
    CopyParam.sDestanation = fxDestFilePath;
    CopyParam.sSource = m_VFilesArr.GetAt(icurretnIndex).sFileName;
    m_CopyArr.Add(CopyParam);	
    icounter++;		
    icurretnIndex--;
    if (icurretnIndex < 0 || icounter == FILES_TO_SAVE_ON_ALERT)
      break;		
  }
  FXString fxOut = "Files were saved"; 
  CTextFrame * ViewText = CTextFrame::Create( fxOut ); 
  if ( !m_pOutput->Put(ViewText))
    ViewText->Release( ViewText ) ;

  HANDLE threadHandle = CreateThread(NULL, 0, CopyThread, this, 0, NULL);
  return icounter;
}

void VControlGadget::GetRelevantFileIndex( time_t st )
{
  for (int i = 0; i < m_VFilesArr.GetSize(); i++)
  {
    double d1 = difftime((st),(m_VFilesArr.GetAt(i).StartWrite));
    double d2 = difftime((st),(m_VFilesArr.GetAt(i).TimeStamp));

    struct tm st;
    st = *localtime(&m_VFilesArr.GetAt(i).StartWrite);

    struct tm fin;
    fin = *localtime(&m_VFilesArr.GetAt(i).TimeStamp);


    if (d1 >= 0 && d2 <= 0)
    {
      m_sLastFName = m_VFilesArr.GetAt(i).sFileName;
      m_iDesirePosition = int (d1 * FRAME_RATE - FRAME_RATE);
      m_iNCurrentFile = i;
    }
  }
}

