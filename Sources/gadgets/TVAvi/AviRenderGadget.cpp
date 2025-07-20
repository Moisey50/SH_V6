// AviRenderGadget.cpp: implementation of the AviRender class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AviRenderGadget.h"
#include <Gadgets\VideoFrame.h>
#include <Gadgets\TextFrame.h>
#include "TVAvi.h"
#include "AviRenderDialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "AviRenderGadget"

__forceinline DWORD Frame2StreamType(CDataFrame* Frame)
{
  switch (Frame->GetDataType())
  {
  case vframe:
    return streamtypeVIDEO;
  case text:
    return streamtypeTEXT;
  case wave:
    return streamtypeAUDIO;
  default:
    if (Frame->GetDataType() & vframe)
      return streamtypeVIDEO;
    return 0;
  }
}

__forceinline void* MakeDataCopy(CDataFrame* Frame)
{
  switch (Frame->GetDataType())
  {
  case vframe:
    return makecopyTVFrame(Frame->GetVideoFrame());
  case text:
    {
      FXString StrData = Frame->GetTextFrame()->GetString();
      char* Data = (char*)calloc(StrData.GetLength() + 1, sizeof(char));
      strcpy(Data, LPCTSTR(StrData));
      return Data;
    }
  case wave:
    return NULL;
  default:
    if (Frame->GetDataType() & vframe)
      return makecopyTVFrame(Frame->GetVideoFrame());
    return NULL;
  }
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX(AviRender, CRenderGadget, "Files.Render", TVDB400_PLUGIN_NAME);

AviRender::AviRender():
  m_bOverwrite(TRUE),
  m_FileName("%X %x.avi"),
  m_VIDC_FOURCC(0),
  m_Buffer(50),
  m_CycleWritting(FALSE),
  m_MaxFileNumber(-1),
  m_MaxFileLength(3600),
  m_CurrentFileNmb(0),
  m_iRenderId( -1 ) ,
  m_GadgetInfo( _T("AviRender") )
{
  m_pControlPin = new CDuplexConnector(this, text, text);
  m_pOutput = new COutputConnector( text ) ;
  m_SetupObject = new AviRenderDialog(this, NULL);
  m_pInput = NULL;	// Default input is not used
  SetMonitor(SET_INPLACERENDERERMONITOR);	// Always built-in (not a floating window)
  AddInput();
  m_ErrorShown=false;
  m_BufferReady=::CreateEvent(NULL, FALSE, FALSE, NULL);
}

void AviRender::ShutDown()
{
  RemoveInputs();
  delete m_pControlPin; m_pControlPin = NULL;
  CRenderGadget::ShutDown();
  if (IsAviOpen()) 
    CloseAvi();
  CloseHandle(m_BufferReady);
}

int AviRender::GetInputsCount()
{
  return (int)m_Inputs.GetCount();
}

CInputConnector* AviRender::GetInputConnector(int n)
{
  CInputConnector* pInput = NULL;
  m_InputLock.Lock();
  if (n >= 0 && n < m_InputsIndex.GetSize())
    pInput = (CInputConnector*)m_InputsIndex.GetAt(n);
  m_InputLock.Unlock();
  return pInput;
}

int AviRender::GetDuplexCount()
{
  return 1;
}

CDuplexConnector* AviRender::GetDuplexConnector(int n)
{
  if (!n)
    return m_pControlPin;
  return NULL;
}

int AviRender::DoJob()
{
  HANDLE pEvents[] = { m_evExit, m_BufferReady /*, m_pStatus->GetStopHandle(),m_pStatus->GetPauseHandle() */ };
  DWORD cEvents = sizeof(pEvents) / sizeof(HANDLE);
  DWORD retVal=::WaitForMultipleObjects(cEvents, pEvents, FALSE, INFINITE);
  AviFrameData afd;

  double ts=GetHRTickCount();
  while (m_Buffer.GetQueueObject(afd))
  {
    Write(afd.pDataFrame,afd.pInput);
  }
  AddCPUUsage(GetHRTickCount()-ts);
  return WR_CONTINUE;
}

void AviRender::Input(CDataFrame* pDataFrame, CConnector* pInput)
{
  AviFrameData afd={pDataFrame, pInput};
  if (!m_Buffer.PutQueueObject(afd))
  {
    pDataFrame->Release();
  }
  ::SetEvent(m_BufferReady);
}

void AviRender::Write(CDataFrame* pDataFrame, CConnector* pInput)
{
  m_InputLock.Lock();
  // First of all process EOS frames - they can be not compatible with media type
  if (Tvdb400_IsEOS(pDataFrame)) 
  {
    m_File.ResetErrors();
    TRACE("!!! EOS!!!/n");
    m_ErrorShown=false;
    if (m_File.IsOpen())
    {
      void* pStream = NULL;

      if (!m_Inputs.Lookup(pInput, pStream))
      {
        pDataFrame->Release();
        m_InputLock.Unlock();
        return;
      }
      if (Tvdb400_IsEOS(pDataFrame))
      {
        DWORD dwLength = m_File.GetLength( pStream ) ;
        if (pStream) 
          m_StreamsEnded.SetAt(pStream, NULL);
        if (m_StreamsEnded.GetCount() == GetInputsCount())
        {
          CloseAvi(); 
          CDuplexConnector * pConnector = GetDuplexConnector( 0 ) ;
          if ( pConnector )
          {
            CTextFrame * pOutMsg = CTextFrame::Create() ;
            pOutMsg->GetString().Format( "Length=%d; Finished=%s;" , dwLength , m_PreviousFileName ) ;
            if ( !pConnector->Put( pOutMsg ) )
              pOutMsg->Release() ;
          }
        }
        pDataFrame->Release();
        m_InputLock.Unlock();
        return;
      }
    }
  }
  else do 
  {
    // 1. Check if pDataFrame is of AVI data format
    DWORD streamType = Frame2StreamType(pDataFrame);
    if (!streamType)
      break;
    // 2. Check if pInput is the render registered input
    void* pStream = NULL;
    // 2.5. Ignore any additional data frames from a stream, previously reported to be finished:
    if (!m_Inputs.Lookup(pInput, pStream))
      break;
    void* dummy;
    if (m_StreamsEnded.Lookup(pStream, dummy))
      break;
    // 3. Check if existing stream is compatible to pDataFrame
    if (pStream)
    {
      if (streamType != m_File.GetStreamType(pStream))
      {
        m_Inputs.SetAt(pInput, NULL);
        m_File.ReleaseStream(pStream);
        pStream = NULL;
      }
    }
    // 4. Create and register compatible stream if absent
    if (!pStream)
    {
      pStream = OpenStream(streamType);
      if (!pStream)
        break;
      m_Inputs.SetAt(pInput, pStream);
    }
    // 5. Finally, write input data to stream
    if (pStream)
    {
      void* pFrame = MakeDataCopy(pDataFrame);
      if (pFrame)
        m_File.Write(pStream, pFrame,pDataFrame->GetLabel());
      DWORD dwLength = m_File.GetLength(pStream) ;
//       if ((m_CycleWritting) && (m_File.GetTimeLength(pStream)>m_MaxFileLength))
      if ( (m_CycleWritting)
        && ( (GetHRTickCount() - m_dFileOpenTime) > m_MaxFileLength * 1000. ) ) // HR time is in ms, 
      {                                                            // m_MaxFileLength is in seconds
        ResetChunk();
        CDuplexConnector * pConnector = GetDuplexConnector( 0 ) ;
        if ( pConnector )
        {
          CTextFrame * pOutMsg = CTextFrame::Create() ;
          pOutMsg->GetString().Format( "Length=%d; Finished=%s;" , dwLength , m_PreviousFileName ) ;
          pOutMsg->CopyAttributes( pDataFrame ) ;
          if ( !pConnector->Put( pOutMsg ) )
            pOutMsg->Release() ;
        }
      }
    }
  } while(false);
  pDataFrame->Release();
  m_InputLock.Unlock();
}

void AviRender::AddInput()
{
  m_InputLock.Lock();
  CInputConnector* pInput = new CInputConnector(transparent, _fn_avi_render_input, this);
  m_Inputs.SetAt(pInput, NULL);
  m_InputsIndex.Add(pInput);
  m_InputLock.Unlock();
}

void AviRender::RemoveInputs()
{
  CloseAvi();
  POSITION pos = m_Inputs.GetStartPosition();
  while (pos)
  {
    CInputConnector* pInput = NULL;
    void* pStream = NULL;
    m_Inputs.GetNextAssoc(pos, (void*&)pInput, pStream);
    m_Inputs.RemoveKey(pInput);
    if (pInput)
    {
      pInput->Disconnect();
      delete pInput;
    }
  }
  m_InputsIndex.RemoveAll();
}

bool AviRender::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  pk.WriteBool("Overwrite", m_bOverwrite);
  pk.WriteInt("NumInputs", (int)m_Inputs.GetCount());
  pk.WriteString("File", m_FileName);
  pk.WriteInt("videofourcc", (int&)m_VIDC_FOURCC);
  pk.WriteInt("cyclewritting",m_CycleWritting);
  pk.WriteInt("maxfilenumber",m_MaxFileNumber);
  pk.WriteInt("maxfilelength",m_MaxFileLength);
  pk.WriteBool("overwriteframerate", m_OverwriteFrameRate);
  pk.WriteBool("calcframerate", m_CalcFrameRate);
  pk.WriteInt( "RenderId" , m_iRenderId ) ;
  pk.WriteDouble("framerate", m_FrameRate);

  text = pk;
  return true;
}

bool AviRender::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXPropertyKit pk(text);
  int nInputs = (int)m_Inputs.GetCount();
  BOOL bOverwrite = m_bOverwrite;
  FXString fileName = m_FileName;
  pk.GetBool("Overwrite", bOverwrite);
  pk.GetInt("NumInputs", nInputs);
  pk.GetString("File", fileName);
  pk.GetInt("videofourcc", (int&)m_VIDC_FOURCC);
  pk.GetInt("cyclewritting",m_CycleWritting);
  pk.GetInt("maxfilenumber",m_MaxFileNumber);
  pk.GetInt("maxfilelength",m_MaxFileLength);
  pk.GetInt( "RenderId" , m_iRenderId ) ;
  if (!pk.GetBool("overwriteframerate", m_OverwriteFrameRate))
    m_OverwriteFrameRate = FALSE;
  if (!pk.GetBool("calcframerate", m_CalcFrameRate))
    m_CalcFrameRate = FALSE;
  if (!pk.GetDouble("framerate", m_FrameRate))
    m_FrameRate = 25.0;
  if ((m_bOverwrite != bOverwrite) || (m_FileName != (LPCTSTR)fileName) || (!m_File.IsOpen()))
    CloseAvi();
  m_File.ResetErrors(); 
  m_ErrorShown=false;
  m_bOverwrite = bOverwrite;
  m_FileName = fileName;
  if (nInputs != m_Inputs.GetCount())
  {
    RemoveInputs();
    while (nInputs--)
      AddInput();
  }
  Status().WriteBool(STATUS_REDRAW, true);
  return true;
}

bool AviRender::ScanSettings(FXString& text)
{
  text="calldialog(true)";
  return true;
}

void AviRender::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pDataFrame)
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
    pk="get|set\r\n\t"
      "close - close current file\r\n\t"
      "cyclewritting\r\n\t"
      "File\r\n\t"
      "maxfilelength\r\n"
      "maxfilenumber\r\n\t"
      "NumInputs\r\n\t"
      "Overwrite\r\n\t"
      "videofourcc\r\n\t"
      "\r\n"
      "For example:\r\n\tset File(E:\\Avifile\\)";
  }
  else if ((cmd.CompareNoCase("get")==0) && (pk.GetWord(pos,cmd)))
  {
    if (cmd.CompareNoCase("Overwrite")==0)
      pk=(m_bOverwrite)?"true":"false";
    else if (cmd.CompareNoCase("NumInputs")==0)
      pk.Format("%d",GetInputsCount());
    else if (cmd.CompareNoCase("File")==0)
      pk=m_FileName;
    else if (cmd.CompareNoCase("videofourcc")==0)
      pk.Format("%c%c%c%c", ((char*)(&m_VIDC_FOURCC))[0],
      ((char*)(&m_VIDC_FOURCC))[1],
      ((char*)(&m_VIDC_FOURCC))[2],
      ((char*)(&m_VIDC_FOURCC))[3]);
    else if (cmd.CompareNoCase("cyclewritting")==0)
      pk=(m_CycleWritting)?"true":"false";
    else if (cmd.CompareNoCase("maxfilenumber")==0)
      pk.Format("%d",m_MaxFileNumber);
    else if (cmd.CompareNoCase("maxfilelength")==0)
      pk.Format("%d",m_MaxFileLength);
  }
  else if ((cmd.CompareNoCase("set")==0) && (pk.GetWord(pos,cmd)) && (pk.GetParamString(pos, param)))
  {
    if (cmd.CompareNoCase("Overwrite")==0)
    {
      FXPropertyKit pp;
      bool          Invalidate=false;

      pp.WriteBool("Overwrite",(param.CompareNoCase("true")==0)?true:false);
      ScanProperties(pp,Invalidate);
      pk="OK";
    }
    else if (cmd.CompareNoCase("NumInputs")==0)
    {
      FXPropertyKit pp;
      bool          Invalidate=false;

      pp.WriteInt("NumInputs",atoi(param));
      ScanProperties(pp,Invalidate);
      pk="OK";
    }
    else if (cmd.CompareNoCase("File")==0)
    {
      FXPropertyKit pp;
      bool          Invalidate=false;

      pp.WriteString("File",param);
      ScanProperties(pp,Invalidate);
      pk="OK";
    }
    else if (cmd.CompareNoCase("videofourcc")==0)
    {
      FXPropertyKit pp;
      bool          Invalidate=false;
      DWORD fourcc=0;
      if (param.GetLength()==4)
      {
        ((char*)(&fourcc))[0]=param[0];
        ((char*)(&fourcc))[1]=param[1];
        ((char*)(&fourcc))[2]=param[2];
        ((char*)(&fourcc))[3]=param[3];
      }
      pp.WriteInt("videofourcc",fourcc);
      ScanProperties(pp,Invalidate);
      pk="OK";
    }
    else if (cmd.CompareNoCase("cyclewritting")==0)
    {
      FXPropertyKit pp;
      bool          Invalidate=false;

      pp.WriteBool("cyclewritting",(param.CompareNoCase("true")==0)?true:false);
      ScanProperties(pp,Invalidate);
      pk="OK";
    }
    else if (cmd.CompareNoCase("maxfilenumber")==0)
    {
      FXPropertyKit pp;
      bool          Invalidate=false;

      pp.WriteInt("maxfilenumber",atoi(param));
      ScanProperties(pp,Invalidate);
      pk="OK";
    }
    else if (cmd.CompareNoCase("maxfilelength")==0)
    {
      FXPropertyKit pp;
      bool          Invalidate=false;

      pp.WriteInt("maxfilelength",atoi(param));
      ScanProperties(pp,Invalidate);
      pk="OK";
    }
    else if (cmd.CompareNoCase("close")==0)
    {
      FXPropertyKit pp;
      bool          Invalidate=false;
      int iRequestedRender = -2 ;
      if ( !param.IsEmpty() ) // param holds something
      {
        if ( param.CompareNoCase( _T("All") ) != 0 )  // is command for all renderers
        {
          if ( _istdigit( param[0] ) )    // numeric id
            iRequestedRender = atoi( param ) ;
        }
        else
          iRequestedRender = 0 ; // Command for all renderers
      }
      if ( (m_iRenderId == -1) && GetGadgetName( pp ) ) // is identification by name?
      {
        if ( pp == param ) // matching for gadget name ( command form "set close <GadgetName>" )
        {
          iRequestedRender = 0 ;
        }
      }
      if ( iRequestedRender == 0  // All renderers or gadget name matches with requested name
        || (iRequestedRender == m_iRenderId) )  // Numeric id is matched
      {
        CloseAvi() ;
        pk.Format("Closed=%s;" , m_PreviousFileName ) ;
      }
    }
  }
  else
  {
    if (cmd.CompareNoCase("close")==0)  // no param, command is for all renderers
    {
      CloseAvi() ;
      pk.Format("Closed=%s;" , m_PreviousFileName ) ;
    }
    else
      pk = _T("Error") ;
  }
  CTextFrame* retV=CTextFrame::Create(pk);
  retV->ChangeId(NOSYNC_FRAME);
  if (!m_pControlPin->Put(retV))
    retV->RELEASE(retV);
}

BOOL AviRender::OpenAvi(LPCTSTR fName, BOOL bOverwrite)
{
  GetGadgetName( m_GadgetInfo ) ;
  FXString fileName;
  m_File.SetVideoFourCC(m_VIDC_FOURCC);
  if (m_CycleWritting) // ignore filename
  {
    fileName.Format("%s%06d",fName,m_CurrentFileNmb);
    bOverwrite=true;
  }
  else if (strchr(fName,'%'))
    fileName=GetAviFileName(fName);
  else
    fileName=fName;
  if (!strlen(fileName))
    return FALSE;
  fileName += _T(".avi") ;
  if (!bOverwrite)
  {
    CFileStatus rStatus;
    if (CFile::GetStatus(fileName,rStatus))
    {
      if (!m_ErrorShown)
      {
        SEND_GADGET_TRACE("Can't open file for writing. File '%s' is already exist",fileName);
        m_ErrorShown=true;
      }
      return FALSE;
    }
  }
  if (bOverwrite || strcmp(m_File.GetFileNameFormat(), fileName)|| (!m_File.IsOpen()))
  {
    CloseAvi();
    if (!m_File.Open(fileName, OF_CREATE))
    {
      if (!m_ErrorShown)
      {
        SEND_GADGET_ERR("Fatal error: Can't open file '%s' for writtng",fileName);
        m_ErrorShown=true;
      }
      return FALSE;
    }
    m_File.SetFrameRate(m_CalcFrameRate, m_OverwriteFrameRate,m_FrameRate);
    SEND_GADGET_TRACE("File '%s' has been opened for writting", m_File.GetRealFileName() );
  }
  m_bOverwrite = bOverwrite;
  m_dFileOpenTime = GetHRTickCount() ;
  return TRUE;
}

void AviRender::CloseAvi()
{
  if (m_File.IsOpen())
  {
    m_PreviousFileName = m_File.GetRealFileName() ;
    m_File.Close();
    POSITION pos=m_Inputs.GetStartPosition( );
    while(pos)
    {
      CConnector* pInput;
      void* pStream;
      m_Inputs.GetNextAssoc(pos,(void*&)pInput,pStream);
      m_Inputs.SetAt(pInput, NULL);
    }
    m_StreamsEnded.RemoveAll();
    SEND_GADGET_TRACE("File '%s' is closed",m_File.GetRealFileName());
    m_CurrentFileNmb=0;
  }
}

void AviRender::ResetChunk()
{
  if (m_File.IsOpen())
  {
    if (m_MaxFileNumber!=-1)
      m_CurrentFileNmb=(m_CurrentFileNmb+1)%m_MaxFileNumber;
    else
      m_CurrentFileNmb++;
    m_PreviousFileName = m_File.GetRealFileName() ;
    m_File.Close();
    POSITION pos=m_Inputs.GetStartPosition( );
    while(pos)
    {
      CConnector* pInput;
      void* pStream;
      m_Inputs.GetNextAssoc(pos,(void*&)pInput,pStream);
      m_Inputs.SetAt(pInput, NULL);
    }
    m_StreamsEnded.RemoveAll();
    SEND_GADGET_TRACE("File '%s' is closed",m_PreviousFileName);
  }
}

bool AviRender::ReceiveEOS(const CDataFrame* pDataFrame)
{
  CloseAvi();
  return false; // don't render last frame to file!
}

void* AviRender::OpenStream(DWORD streamType, long id)
{
  if (m_StreamsEnded.GetCount())
    return NULL;	// do not open new streams, if any streams reported to be finished but not closed
  if (!m_File.IsOpen() && !OpenAvi(m_FileName, m_bOverwrite))
    return NULL;
  return m_File.GetStream(streamType, id);
}

bool    AviRender::IsAviOpen()
{
  return (m_File.IsOpen()!=FALSE);
}