// AviCaptureGadget.cpp: implementation of the AviCapture class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <sys\stat.h>
#include <fstream>
#include "AviCaptureGadget.h"
#include <Gadgets\VideoFrame.h>
#include <Gadgets\TextFrame.h>
#include "TVAvi.h"
#include <files\AviStream.h>
#include "AviCaptureDialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "TVAvi.AviCapture"

bool IsFileExistsAndNotOpenedForWrite( LPCTSTR FileName )
{

  FILE * pf =  _fsopen( FileName , _T("r") , _SH_DENYWR ) ;
  if ( pf )
  {
    fclose( pf ) ;
    return true ;
  }
  return false ;
}


// CAVIChunk class
//////////////////////////////////////////////////////////////////////

__forceinline int _compare(FILETIME& f1, FILETIME& f2)
{
  if (f1.dwHighDateTime==f2.dwHighDateTime)
  {
    if (f1.dwLowDateTime<f2.dwLowDateTime)
      return -1;
    else if (f1.dwLowDateTime>f2.dwLowDateTime)
      return 1;
    return 0;
  }
  else if (f1.dwHighDateTime<f2.dwHighDateTime)
    return -1;
  else 
    return 1;
}

CAVIChunk::CAVIChunk():
m_CurPos(0)
{
}

CAVIChunk::~CAVIChunk()
{
}

bool CAVIChunk::List(LPCTSTR dir)
{
  m_List.RemoveAll();
  FXString path=dir; path+="*.avi"; 
  FXFileFind ff;
  BOOL bWorking = ff.FindFile(path);
  while (bWorking)
  {
    chunk chnk;
    bWorking = ff.FindNextFile();
    chnk.fName=ff.GetFileName();
    ff.GetLastWriteTime(&chnk.fTime);
    m_List.Add(chnk);
  }
  // Sort by time
  bool sorted=true;
  do
  {
    sorted=true;
    for (int i=0; i<m_List.GetSize()-1; i++)
    {
      if (_compare(m_List[i].fTime,m_List[i+1].fTime)>0)
      {
        chunk tmp=m_List[i];
        m_List[i]=m_List[i+1];
        m_List[i+1]=tmp;
        sorted=false;
      }
    }
  }
  while(!sorted);

  m_Length=0;
  m_TimeLength=0;
  for (int i=0; i<m_List.GetSize(); i++)
  {
    AVIFILEINFO afi;
    if (GetInfo(FXString(dir)+m_List[i].fName,afi))
    {
      m_List[i].dwLength=afi.dwLength;
      m_List[i].dRate=((double)afi.dwRate)/afi.dwScale;
      m_List[i].dTimeLength=((double)afi.dwLength)/m_List[i].dRate;
      m_List[i].dwOffset=m_Length;
      m_List[i].dTimeOffset=m_TimeLength;
      m_Length+=m_List[i].dwLength;
      m_TimeLength+=m_List[i].dTimeLength;
      CTime tm(m_List[i].fTime);
      TRACE(_T("%s %s: %d frames, %.2f s\n"), m_List[i].fName, tm.Format("%x %X"), m_List[i].dwLength, m_List[i].dTimeLength);
    }
  }
  TRACE(_T("%d frames, %.2f s\n"),m_Length,m_TimeLength);
  m_CurPos=0;
  if (m_List.GetSize())
  {
    m_Path=dir;
  }
  else
    m_Path="";
  return true;
}

FXString CAVIChunk::GetNext()
{
  if (m_CurPos<m_List.GetSize())
  {
    FXString retV=m_Path+m_List[m_CurPos].fName;
    m_CurPos++;
    return retV;
  }
  return "";
}

FXString    CAVIChunk::GetForPosition(DWORD pos)
{
  for (int i=0; i<m_List.GetSize(); i++)
  {
    if ((pos>=m_List[i].dwOffset) && (pos<m_List[i].dwOffset+m_List[i].dwLength))
    {
      m_CurPos=i;
      FXString retV=m_Path+m_List[m_CurPos].fName;
      m_CurPos++;
      return retV;
    }
  }
  return "";
}

bool CAVIChunk::IsEOD()
{
  return (m_CurPos>=m_List.GetSize());
}

DWORD CAVIChunk::GetChunkOffset()
{
  int prevChunck=m_CurPos-1;
  if (prevChunck<0)
    return 0; 
  return m_List[prevChunck].dwOffset;
}

bool CAVIChunk::GetInfo(LPCTSTR fName, AVIFILEINFO& afi)
{
  PAVIFILE		hFile;
  HRESULT result = ::AVIFileOpen(&hFile, fName, OF_READ, NULL);
  if (!SUCCEEDED(result))
    return false;
  result = ::AVIFileInfo(hFile, &afi, sizeof(afi));
  ::AVIFileRelease(hFile);
  return SUCCEEDED(result);
}
// AviCapture gadget class
//////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX(AviCapture, CCaptureGadget, "Files.Capture", TVDB400_PLUGIN_NAME);

AviCapture::AviCapture():
m_AVIChunks(NULL),
  m_bSoftwareTrigger(FALSE),
  m_pInputTrigger(NULL),
  m_pControlPin(NULL),
  m_dwFrameRate(25),
  m_AddEOS(0),
  m_LoopFilm(FALSE),
  m_ReserveOutputs(false),
  m_OutputsNumber(1),
  m_ChunkMode(false),
  m_SWTriggerEvent( NULL )
{
  m_pControlPin = new CDuplexConnector(this, text, text);
  m_SetupObject = new AviCaptureDialog(this, NULL);
  m_dwFrameRate = 25;
  SetTicksIdle(1000 / m_dwFrameRate);
}

void AviCapture::ShutDown()
{
  OnStop();
  m_AviFile.Close();
  if (m_AVIChunks)
  {
    delete m_AVIChunks;
    m_AVIChunks=NULL;
  }
  if (m_pInputTrigger)
    delete m_pInputTrigger;
  m_pInputTrigger = NULL;
  if ( m_SWTriggerEvent )
  {
    CloseHandle( m_SWTriggerEvent ) ;
    m_SWTriggerEvent = NULL ;
  }
  RemoveOutputs();
  delete m_pControlPin; m_pControlPin = NULL;
  CCaptureGadget::ShutDown();
}

int AviCapture::GetInputsCount()
{
  return ((m_pInputTrigger) ? 1 : 0);
}

CInputConnector* AviCapture::GetInputConnector(int n)
{
  if (m_pInputTrigger && !n)
    return m_pInputTrigger;
  return NULL;
}

int AviCapture::GetOutputsCount()
{
  return (int)m_Outputs.GetSize();
}

COutputConnector* AviCapture::GetOutputConnector(int n)
{
  if ((n < 0) || (n >= GetOutputsCount()))
    return NULL;
  return (COutputConnector*)m_Outputs.GetAt(n);
}

int AviCapture::GetDuplexCount()
{
  return 1;
}

CDuplexConnector* AviCapture::GetDuplexConnector(int n)
{
  if (!n)
    return m_pControlPin;
  return NULL;
}

bool AviCapture::NextChunk()
{
  if ((m_AVIChunks->IsEOD()) && (m_LoopFilm))
    m_AVIChunks->Rewind();
  if (m_AVIChunks->IsEOD()) return false;
  FXString fn=m_AVIChunks->GetNext();
  if (fn.IsEmpty() || !OpenAvi(fn))
  {
    //SENDERR_1("Error: can't open source file '%s'",path);
    return false;
  }
  return true;
}

CDataFrame* AviCapture::GetNextFrame(void* pStream)
{
  if (!pStream)
    return CDataFrame::Create();
  switch (m_AviFile.GetStreamType(pStream))
  {
  case streamtypeVIDEO:
    {
      DWORD ID;
      pTVFrame Frame = (pTVFrame)m_AviFile.Read(pStream,ID);
      if (!Frame)
      {
        if ((m_AviFile.IsEOF(pStream)) && (m_ChunkMode))
        {
          if (NextChunk())
            return NULL;
          else
          {
            CDataFrame* pFrame=CDataFrame::Create();
            Tvdb400_SetEOS(pFrame);
            return pFrame;
          }
        }
        else if ((m_AviFile.IsEOF(pStream)) && (m_LoopFilm))
        {
          m_AviFile.SeekToBegin(pStream);
          //CDataFrame* pFrame=CDataFrame::Create();
          //Tvdb400_SetEOS(pFrame);
          return NULL;
        }
        else
        {
          //return NULL;
          CDataFrame* pFrame=CDataFrame::Create();
          Tvdb400_SetEOS(pFrame);
          return pFrame;
        }
        Frame = (pTVFrame)m_AviFile.Read(pStream,ID);
      }
      CVideoFrame* pFrame = CVideoFrame::Create(Frame);
      if ((m_ChunkMode) && (m_AVIChunks))
        pFrame->ChangeId(m_AVIChunks->GetChunkOffset()+ID+1);
      else
        pFrame->ChangeId(ID+1);
      pFrame->SetLabel(((CAviStream*)pStream)->GetLabel());
      return pFrame;
    }
  case streamtypeTEXT:
    {
      DWORD ID;
      char* Text = (char*)m_AviFile.Read(pStream,ID);
      if (!Text)
      {
        if ((m_AviFile.IsEOF(pStream)) && (m_LoopFilm))
        {
          m_AviFile.SeekToBegin(pStream);
          CDataFrame* pFrame=CDataFrame::Create();
          //Tvdb400_SetEOS(pFrame);
          return pFrame;
        }
        else
        {
          //return NULL;
          CDataFrame* pFrame=CDataFrame::Create();
          Tvdb400_SetEOS(pFrame);
          return pFrame;
        }
        Text = (char*)m_AviFile.Read(pStream,ID);
      }
      CTextFrame* pFrame = CTextFrame::Create();
      pFrame->ChangeId(ID);
      pFrame->GetString() = Text;
      free(Text);
      return pFrame;
    }
  default:
    return NULL;
  }
}

void AviCapture::OnStop()
{
  FXAutolock al(m_OutputLock);
  for (int i = 0; i < m_Outputs.GetSize(); i++)
  {
    void* pStream = NULL;
    if (m_OutputStreams.Lookup(m_Outputs.GetAt(i), pStream))
      m_AviFile.SeekToBegin(pStream);
  }
  m_bRun = FALSE;
}

void AviCapture::ToggleSoftwareTrigger()
{
  FXAutolock al(m_OutputLock) ;
  m_bSoftwareTrigger = !m_bSoftwareTrigger;
  if (m_bSoftwareTrigger)
  {
//     Suspend();
    ASSERT(!m_pInputTrigger);
    if ( !m_SWTriggerEvent )
      m_SWTriggerEvent = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
    else
      ASSERT( 0 ) ;
    if ( !m_pInputTrigger )
      m_pInputTrigger = new CInputConnector(nulltype, _fn_capture_trigger, this);
    else
      ASSERT( 0 ) ;
  }
  else
  {
    ASSERT(m_pInputTrigger);
    delete m_pInputTrigger;
    m_pInputTrigger = NULL;
    if ( m_SWTriggerEvent )
    {
      CloseHandle( m_SWTriggerEvent ) ;
      m_SWTriggerEvent = NULL ;
    }
    else
      ASSERT( 0 ) ;
//     Resume();
  }
}

void AviCapture::TriggerPulse(CDataFrame* pDataFrame)
{
  int tick = pDataFrame->GetId();
  pDataFrame->Release();
  if ( m_SWTriggerEvent )
    SetEvent( m_SWTriggerEvent ) ;
//   SendData();
}

LPCTSTR AviCapture::StepViaFile( int iDelta )
{
  int iPos = GetPosition() ;
  int iLength = GetLength() ;
  iPos += iDelta - 1 ;

  if ( m_LoopFilm ) // we are working inside one file
  {
    if ( iPos < 0 )  // looping from 0 to the end of file
      iPos = iLength - 1 ;
    else             // may be looping from end of file to beginning
      iPos %= iLength ;
    SetPosition( iPos ) ;
    SendData() ;
  }
  else if ( !m_bMultiFile )
  {
    if ( iPos < 0 )
      iPos = 0 ;
    else if ( iPos >= iLength )
      iPos = iLength - 1 ;
    SetPosition( iPos ) ;
    SendData() ;
  }    
  else if ( 0 <= iPos   &&  iPos < iLength )// we still in the same file
  {
    SetPosition( iPos ) ;
    SendData() ;
  }
  else // we should go to other file, if it exists
  {
    CString CurrentFileName = m_AviFile.GetRealFileName() ;
    struct stat CurrentStat , OtherStat ;
    time_t CurrFileTime = NULL , OtherFileTime = NULL , NearestFileTime ;
    if ( stat( (LPCTSTR)CurrentFileName , &CurrentStat ) < 0 )
    {
      ASSERT(0) ;
      return _T("Error: no file time") ;
    }
    else
      CurrFileTime = CurrentStat.st_mtime ; 

    int iSlashPos = CurrentFileName.ReverseFind( _T('\\') ) ;
    CString DirName ;
    if (iSlashPos >= 0 )  
      DirName = CurrentFileName.Left( iSlashPos + 1 ) ;
    CString NearestFile ;

    CString FileProto = DirName + _T("*.avi") ;
    CFileFind ff ;
    BOOL bWorking = ff.FindFile( FileProto ) ;
    while ( bWorking )
    {
      bWorking = ff.FindNextFile() ;
      CString NextFile = ff.GetFilePath() ;
      if ( stat( (LPCTSTR)NextFile , &OtherStat ) < 0 )
      {
        ASSERT(0) ;
      }
      else
      {
        OtherFileTime = OtherStat.st_mtime ; 
        if ( iPos >= iLength ) // go forward
        {
         if ( OtherFileTime > CurrFileTime )  // other is after current
          {
            if ( NearestFile.IsEmpty() )
            {
              NearestFile = NextFile ;
              NearestFileTime = OtherFileTime ;
            }
            else if ( NearestFileTime < OtherFileTime )
            {
              NearestFile = NextFile ;
              NearestFileTime = OtherFileTime ;
            }
          }
        }
        else
        {
          if ( OtherFileTime < CurrFileTime )  // other is before current
          {
            if ( NearestFile.IsEmpty() )
            {
              NearestFile = NextFile ;
              NearestFileTime = OtherFileTime ;
            }
            else if ( NearestFileTime > OtherFileTime )
            {
              NearestFile = NextFile ;
              NearestFileTime = OtherFileTime ;
            }
          }
        }
      }
    }
    if ( !NearestFile.IsEmpty()  
      &&  IsFileExistsAndNotOpenedForWrite( NearestFile) )
    {  // file exists and not opened for writing
      OpenAvi( NearestFile ) ;
      // correct position in new file
      // Keep in the mind - we don't jump over files.
      int iNewLength = GetLength() ;
      if ( iPos < 0 )
        iPos = iNewLength + iPos ;
      else
        iPos -= iLength ;
      if ( iPos >= iNewLength ) 
        iPos = iNewLength - 1 ;
      if ( 0 < iPos )
        SetPosition( iPos ) ;
      SendData() ;
    }
  }
  return _T("ok") ;
}

void AviCapture::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pDataFrame)
{
  CTextFrame* TextFrame = pDataFrame->GetTextFrame(DEFAULT_LABEL);
  if (!TextFrame)
  {
    pDataFrame->Release();
    return;
  }
  FXParser pk=(LPCTSTR)TextFrame->GetString();
  pDataFrame->Release();
  FXString cmd,param;
  FXSIZE pos=0;
  pk.GetWord(pos,cmd);
  if ((cmd.CompareNoCase("list")==0) || (cmd.CompareNoCase("help")==0) || (cmd.CompareNoCase("?")==0))
  {
    pk="get|set\r\n\tFileName\r\n\tFrameRate\tSWTrigger\r\n\tLoopFilm\r\n\tPosition\r\n\tLength (only get)\r\n\r\nFor example:\r\n\tset loopfilm(false)";
  }
  else if ((cmd.CompareNoCase("get")==0) && (pk.GetWord(pos,cmd)))
  {
    if (cmd.CompareNoCase("FileName")==0)
      pk=m_AviFile.GetFileNameFormat();
    else if (cmd.CompareNoCase("FrameRate")==0)
      pk.Format("%d",m_dwFrameRate);
    else if (cmd.CompareNoCase("SWTrigger")==0)
      pk=(m_bSoftwareTrigger)?"true":"false";
    else if (cmd.CompareNoCase("LoopFilm")==0)
      pk=(m_LoopFilm)?"true":"false";
    else if (cmd.CompareNoCase("Length")==0)
      pk.Format("%d",GetLength());
    else if (cmd.CompareNoCase("Position")==0)
      pk.Format("%d",GetPosition());
  }
  else if ( (cmd.CompareNoCase("set")==0) && (pk.GetWord(pos,cmd)) )
  {
    pk.GetParamString(pos, param) ;
    if (cmd.CompareNoCase("FileName")==0)
    {
      if (param.IsEmpty() || !OpenAvi(param))
        pk="Error";
      else 
        pk="OK";
    }
    else if (cmd.CompareNoCase("FrameRate")==0)
    {
      if ( param.IsEmpty() )
        pk = "Error" ;
      else
      {
        m_dwFrameRate=atoi(param);
        SetTicksIdle(1000 / m_dwFrameRate);
        pk="OK";
      }
    }
    else if (cmd.CompareNoCase("SWTrigger")==0)
    {
      BOOL bSoftwareTrigger= !(param.CompareNoCase("false")==0);
      if (bSoftwareTrigger!=m_bSoftwareTrigger)
        ToggleSoftwareTrigger();
      pk="OK";
    }
    else if (cmd.CompareNoCase("LoopFilm")==0)
    {
      m_LoopFilm = !(param.CompareNoCase("false")==0);
      pk="OK";
    }
    else if ( cmd.CompareNoCase("Position")==0 )
    {
      if ( param.IsEmpty() )
        pk = "Error" ;
      else
      {
        SetPosition(atoi(param));
        pk="OK";
      }
    }
    else if (cmd.CompareNoCase("Forw")==0 || cmd.CompareNoCase("Forward")==0)
    {
      int iDelta = param.IsEmpty() ? 0 : atoi( param ) ;
      LPCTSTR Answer = StepViaFile( iDelta ) ;
      pk = Answer ;
    }
    else if (cmd.CompareNoCase("Back")==0 || cmd.CompareNoCase("Backward")==0)
    {
      int iDelta = param.IsEmpty() ? 0 : atoi( param ) ;
      pk = StepViaFile( -iDelta ) ;
    }
  }
  else
  {
    pk="Error";
  }
  CTextFrame* retV=CTextFrame::Create(pk);
  retV->ChangeId(NOSYNC_FRAME);
  if (!m_pControlPin->Put(retV))
    retV->RELEASE(retV);
}

bool AviCapture::PrintProperties(FXString& text)
{
  FXPropertyKit pc;
  pc.WriteString("FileName",m_AviFile.GetFileNameFormat());
  pc.WriteInt("FrameRate",m_dwFrameRate); 
  pc.WriteBool("SWTrigger",m_bSoftwareTrigger);
  pc.WriteBool("LoopFilm", m_LoopFilm);
  pc.WriteBool("ReserveOutputs", m_ReserveOutputs);
  pc.WriteBool("ChunkMode",m_ChunkMode);
  pc.WriteInt("OutputsNumber",m_OutputsNumber);
  text=pc;
  return true;
}

bool AviCapture::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXPropertyKit pc(text);
  FXString path;
  pc.GetString("FileName",path);
  if (pc.GetInt("FrameRate", (int&)m_dwFrameRate))
    SetTicksIdle(1000 / m_dwFrameRate);
  BOOL bSoftwareTrigger = m_bSoftwareTrigger;
  if (pc.GetBool("SWTrigger", bSoftwareTrigger) &&
    (bSoftwareTrigger != m_bSoftwareTrigger))
    ToggleSoftwareTrigger();
  pc.GetBool("ChunkMode",m_ChunkMode);
  pc.GetBool("LoopFilm", m_LoopFilm);
  pc.GetBool("ReserveOutputs", m_ReserveOutputs);
  pc.GetInt("OutputsNumber",m_OutputsNumber);
  if (m_ReserveOutputs)
  {
    while((int)m_Outputs.GetCount()>m_OutputsNumber)
      RemoveOutput((int)m_Outputs.GetCount()-1);
    while((int)m_Outputs.GetCount()<m_OutputsNumber)
      AddOutput(NULL);
  }
  if (m_ChunkMode)
  {
    path=FxExtractPath(path);
    if (!m_AVIChunks)
      m_AVIChunks = new CAVIChunk;
    m_AVIChunks->List(path);
    FXString fn=m_AVIChunks->GetNext();
    if (fn.IsEmpty() || !OpenAvi(fn))
    {
      //SENDERR_1("Error: can't open source file '%s'",path);
    }
  }
  else
  {
    if (m_AVIChunks)
    {
      delete m_AVIChunks;
      m_AVIChunks=NULL;
    }
    if (path.IsEmpty() || !OpenAvi(path))
    {
      //SENDERR_1("Error: can't open source file '%s'",path);
    }
  }
  Status().WriteBool(STATUS_REDRAW, true);
  return true;
}

bool AviCapture::ScanSettings(FXString& text)
{
  text="calldialog(true)";
  return true;
}

int AviCapture::DoJob()
{
  ASSERT(m_pStatus);
  switch (m_pStatus->GetStatus())
  {
  case CExecutionStatus::STOP:
    if (m_bRun)
    {
      m_AddEOS = 1 ;
      SendData( m_bSoftwareTrigger == TRUE ) ; // if SW trigger mode - send EOS only
      OnStop();
      return WR_CONTINUE;
    }
    else
    {
      HANDLE pEvents[] = { m_evExit, m_pStatus->GetStartHandle() };
      DWORD cEvents = sizeof(pEvents) / sizeof(HANDLE);
      DWORD retVal=::WaitForMultipleObjects(cEvents, pEvents, FALSE, INFINITE);
      return WR_CONTINUE;
    }
  case CExecutionStatus::PAUSE:
    {
      if ( !m_bSoftwareTrigger )
      {
        HANDLE pEvents[] = { m_evExit, m_pStatus->GetStartHandle() , m_pStatus->GetStpFwdHandle() };
        DWORD cEvents = sizeof(pEvents) / sizeof(HANDLE);
        DWORD retVal=::WaitForMultipleObjects(cEvents, pEvents, FALSE, INFINITE);
        if ((retVal==2) && (!m_bSoftwareTrigger))
          SendData();
      }
      else
      {
        HANDLE pEvents[] = { m_evExit, m_pStatus->GetStartHandle() , m_pStatus->GetStpFwdHandle() , m_SWTriggerEvent };
        DWORD cEvents = sizeof(pEvents) / sizeof(HANDLE);
        DWORD retVal=::WaitForMultipleObjects(cEvents, pEvents, FALSE, INFINITE);
        if ((retVal==3) && (m_bSoftwareTrigger))
          SendData();
      }
       return WR_CONTINUE;
    }
  case CExecutionStatus::RUN:
    if (!m_bRun)
    {
      TRACE("AviCapture::DoJob - Start is requested\n");
      OnStart();
    }
    if (!m_bSoftwareTrigger)
      SendData();
    else
    {
      if ( WaitForSingleObject( m_SWTriggerEvent , 100 ) == WAIT_OBJECT_0 )
        SendData() ;
    }
    return WR_CONTINUE;
  case CExecutionStatus::EXIT:
    return WR_EXIT;
  default:
    ASSERT(FALSE);
    return WR_CONTINUE;
  }
}

void AviCapture::SendData( bool bEOSOnly )
{
  double ts=GetHRTickCount();
  if (m_bRun)
  {
    FXAutolock al(m_OutputLock);
    POSITION pos = m_OutputStreams.GetStartPosition();
    while (pos)
    {
      COutputConnector* pOutput;
      void* pStream;
      m_OutputStreams.GetNextAssoc(pos, (void*&)pOutput, pStream);
      ASSERT(pOutput);
      if (!pStream)
        continue;
      CDataFrame* pDataFrame = (bEOSOnly) ? 
        CDataFrame::Create( nulltype ) : GetNextFrame(pStream);
      if (pDataFrame)
      {
        if (m_AddEOS)
          Tvdb400_SetEOS(pDataFrame);
        StampDataFrame(pDataFrame);
        if (!pOutput->Put(pDataFrame))
          pDataFrame->Release();
      }
    }
    m_AddEOS=0;
  }
  AddCPUUsage(GetHRTickCount()-ts);
}

BOOL AviCapture::OpenAvi(LPCTSTR fileName)
{
  // FXAutolock al(m_OutputLock);
  //	RemoveOutputs();
  m_AviFile.Close();
  POSITION pos = m_OutputStreams.GetStartPosition();
  while (pos)
  {
    LPVOID output, stream;
    m_OutputStreams.GetNextAssoc(pos, output, stream);
    m_OutputStreams.SetAt(output, NULL);
  } // very well! but you forgot to delete obsolete assocs later... see bug 57
  if (!m_AviFile.Open(fileName, CAviFile::modeRead))
    return FALSE;
  int cOutputs = m_AviFile.GetStreamCount();
  int cVideo = 0, cAudio = 0, cText = 0;
  for (int i = 0; i < cOutputs; i++)
  {
    void* pStream = m_AviFile.GetStream((long)i);
    if (!pStream)
      break;
    DWORD streamType = m_AviFile.GetStreamType(pStream);
    int cOutput;
    switch (streamType)
    {
      // insert outputs of known types:
    case streamtypeVIDEO:
      cOutput = cVideo++;
      break;
    case streamtypeTEXT:
      cOutput = cText++;
      break;
    case streamtypeAUDIO:
      cOutput = cAudio++;
      break;
      // free unknown streams
    default:
      m_AviFile.ReleaseStream(pStream);
      continue;
    }
    CConnector* Connector = GetOutput(streamType, cOutput);
    if (Connector)
      m_OutputStreams.SetAt(Connector, pStream);
    else
      AddOutput(pStream);
  }
  // now, let's do the clean-up
  if (!m_ReserveOutputs)
  {
    for (int i = 0; i < GetOutputsCount(); i++)
    {
      void* stream;
      COutputConnector* Connector = GetOutputConnector(i);
      if (!m_OutputStreams.Lookup(Connector, stream) || !stream)
      {
        m_Outputs.RemoveAt(i--);
        Connector->Disconnect();
        delete Connector;
        m_OutputStreams.RemoveKey(Connector);
      }
    }
  }
  SENDTRACE("File \"%s\" is opened",fileName);
  return TRUE;
}

void AviCapture::AddOutput(void* pStream)
{
  FXAutolock al(m_OutputLock);
  COutputConnector* pOutput = NULL;
  if (pStream)
  {
    switch (m_AviFile.GetStreamType(pStream))
    {
    case streamtypeVIDEO:
      pOutput = new COutputConnector(vframe);
      break;
    case streamtypeTEXT:
      pOutput = new COutputConnector(text);
      break;
    case streamtypeAUDIO:
      pOutput = new COutputConnector(wave);
      break;
    }
    if (!pOutput)
      return;
    m_OutputStreams.SetAt(pOutput, pStream);
  }
  else // 2013.03.05 reserved pins
  {
    pOutput = new COutputConnector(transparent);
    if (!pOutput)
      return;
    m_OutputStreams.SetAt(pOutput, NULL);
  }
  m_Outputs.Add(pOutput);
}

CConnector* AviCapture::GetOutput(DWORD aviType, int nOutput)
{
  datatype dType;
  if (aviType == streamtypeVIDEO)
    dType = vframe;
  else if (aviType == streamtypeTEXT)
    dType = text;
  else if (aviType == streamtypeAUDIO)
    dType = wave;
  else
    return NULL;
  int nConnector = 0;
  while (nConnector < GetOutputsCount())
  {
    CConnector* Connector = GetOutputConnector(nConnector++);
    if (Connector && Tvdb400_TypesCompatible(Connector->GetDataType(),dType) && (!nOutput--))
      return Connector;
  }
  return NULL;
}

void AviCapture::RemoveOutput(int pos)
{
  FXAutolock al(m_OutputLock);
  COutputConnector* pOutput = GetOutputConnector(pos);
  m_Outputs.RemoveAt(pos);
  pOutput->Disconnect();
  m_OutputStreams.RemoveKey(pOutput);
  delete pOutput;
}

void AviCapture::RemoveOutputs()
{
  while (GetOutputsCount())
    RemoveOutput(0);
}

void AviCapture::StampDataFrame(CDataFrame* pDataFrame)
{
  pDataFrame->SetTime(GetGraphTime() * 1.e-3); // convert to milli seconds
  //if (!m_bRun)
  //{
  //Tvdb400_SetEOS(pDataFrame);
  //pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
  //}
  //else
  //{
  //pDataFrame->ChangeId(m_FrameCounter);
  //pDataFrame->SetTime(GetGraphTime() * 1.e-3 );
  //m_FrameCounter++;
  //}
}

DWORD AviCapture::GetLength()
{
  if (m_ChunkMode)
    return m_AVIChunks->GetLength();
  if (m_AviFile.IsOpen())
    return m_AviFile.GetLength();
  return 0;
}

DWORD AviCapture::GetPosition()
{
  DWORD retV=0;
  void* pStream = NULL;
  if (m_OutputStreams.Lookup(m_Outputs.GetAt(0), pStream))
    retV=m_AviFile.GetPos(pStream);
  if ((m_ChunkMode) && (m_AVIChunks))
    retV+=m_AVIChunks->GetChunkOffset();
  return retV;
}

void  AviCapture::SetPosition(DWORD pos)
{
  FXAutolock al(m_OutputLock);
  if ((m_ChunkMode) && (m_AVIChunks))
  {
    FXString fxs=m_AVIChunks->GetForPosition(pos);
    if (fxs!=m_AviFile.GetFileNameFormat())
      OpenAvi(fxs);
    pos-=m_AVIChunks->GetChunkOffset();
  }
  for (int i = 0; i < m_Outputs.GetSize(); i++)
  {
    void* pStream = NULL;
    if (m_OutputStreams.Lookup(m_Outputs.GetAt(i), pStream))
      m_AviFile.SeekToFrame(pStream,pos);
  }
}

