#include "stdafx.h"
#include "WavGadgetsImpl.h"
#include "WavGadgets.h"
#include "WavOscillograph.h"

#define THIS_MODULENAME "WaveCaptureGadget"

#pragma comment(lib,"winmm.lib")

LPCTSTR GetMMErrorMessage(MMRESULT LastErrNo)
{
  switch (LastErrNo)
  {
  case MMSYSERR_NOERROR: return "Success.";
  case MMSYSERR_ALLOCATED: return "Specified resource is already allocated.";
  case MMSYSERR_BADDEVICEID: return "Specified device identifier is out of range.";
  case MMSYSERR_NODRIVER: return "No device driver is present.";
  case MMSYSERR_NOMEM: return "Unable to allocate or lock memory."; 
  case WAVERR_BADFORMAT: return "Attempted to open with an unsupported waveform-audio format.";
  case SOUNDERROR_DONTOPEN: return "Sound device dosn't open";
  case SOUNDERROR_NOFILE: return "Can't open file specified";
  default:
    return "Unknown error";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//WaveCapture
//

IMPLEMENT_RUNTIME_GADGET_EX(WaveCapture, CCaptureGadget, "Wave", TVDB400_PLUGIN_NAME);

void CALLBACK SoundInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD dwParam2)
{
  WaveCapture* pSoundIn = NULL;
  switch (uMsg)
  {
  case WIM_OPEN:

    TRACE("+++ WaveCapture WIM_OPEN\n");
    break;
  case WIM_DATA:
    if ((((LPWAVEHDR)dwParam1)->dwBytesRecorded)!=0)
    {
      pSoundIn=(WaveCapture*)dwInstance;
      pSoundIn->m_lpWaveHdr = (LPWAVEHDR)dwParam1; 
      SetEvent(pSoundIn->m_hEvent);
    }
    else
    {
      TRACE("+++ WaveCapture WIM_DATA, empty buffer\n");
    }
    break;
  case WIM_CLOSE:
    TRACE("+++ WaveCapture WIM_CLOSE\n");
    break;
  default:
    TRACE("+++ WaveCapture Unknown\n");
    break;
  }
}

WaveCapture::WaveCapture():
  SBSetupHelper(MAXNUMOFBUFFER,BUFFERTIME),
  m_wMCIDeviceID(0)
{
  if (m_wMCIDeviceID>waveOutGetNumDevs()-1) 
    m_wMCIDeviceID=0;
  m_iNChannels = 1 ;
  m_lpWaveHdr=NULL;
  m_IsOpen=false;
  m_IsStarted=false;
  m_pOutput = new COutputConnector(wave);
  m_hEvent=NULL;
}

bool WaveCapture::Init()
{
  if (m_IsOpen) 
    Close();
  memset(&WaveFormat,0,sizeof(WAVEFORMATEX));
  WaveFormat.nSamplesPerSec=44100;
  WaveFormat.wBitsPerSample=16;
  WaveFormat.nChannels = m_iNChannels ;
  WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
  WaveFormat.nBlockAlign = WaveFormat.nChannels * WaveFormat.wBitsPerSample/8;
  WaveFormat.nAvgBytesPerSec = WaveFormat.nBlockAlign * WaveFormat.nSamplesPerSec;
  dwBufferSize = WaveFormat.nAvgBytesPerSec*BUFFERTIME/1000 ; // NChannels and Bits per sample are accounted
  for(int i=0; i<iMaxBufNum; i++)
  {
    hWaveInHdr[i] = GlobalAlloc(GHND | GMEM_SHARE , sizeof(WAVEHDR));
    lpWaveInHdr[i] = (LPWAVEHDR)GlobalLock(hWaveInHdr[i]);
    hInBuffer[i] = GlobalAlloc(GHND | GMEM_SHARE , dwBufferSize);
    lpInBuffer[i] = (LPBYTE)GlobalLock(hInBuffer[i]);
    lpWaveInHdr[i]->lpData = (LPSTR)lpInBuffer[i];
    lpWaveInHdr[i]->dwBufferLength = dwBufferSize;
    lpWaveInHdr[i]->dwBytesRecorded = 0L;
    lpWaveInHdr[i]->dwUser = i;//used for identify wave buffer by it number
    lpWaveInHdr[i]->dwFlags = 0L;
    lpWaveInHdr[i]->dwLoops = 1L;
    lpWaveInHdr[i]->lpNext = NULL;  
    lpWaveInHdr[i]->reserved = 0L;
  }
  if  (
    (LastErrNo=waveInOpen(&hWaveIn, m_wMCIDeviceID, &WaveFormat, 
    (DWORD_PTR)SoundInProc, (DWORD_PTR)this, CALLBACK_FUNCTION)) 
    || (hWaveIn==0) )
  {
    Close();
    return false;
  }
  m_hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
  TRACE("+++ Sound input is open\n");
  m_IsOpen=true; 
  if (IsRun())
    Start();
  return true;
}

void WaveCapture::Close()
{
  Stop();
  for(int i=0; i<iMaxBufNum; i++)
  {
    if (hWaveInHdr[i] != NULL)
    {
      GlobalUnlock(hWaveInHdr[i]);
      GlobalFree(hWaveInHdr[i]);
      GlobalUnlock(hInBuffer[i]);
      GlobalFree(hInBuffer[i]);
      hWaveInHdr[i] = NULL;
      hInBuffer[i]  = NULL;
      lpWaveInHdr[i]= NULL;
      lpInBuffer[i] = NULL;
    }
  }
  if (hWaveIn)
  {
    waveInClose(hWaveIn); 
    hWaveIn=NULL;
  }
  if (m_hEvent) CloseHandle(m_hEvent); m_hEvent=NULL;
  m_IsOpen=false;
}

void WaveCapture::ShutDown()
{
  CCaptureGadget::ShutDown();
  Close();
  delete m_pOutput;
  m_pOutput = NULL;
}

void WaveCapture::Start()
{
  SendEof();
  if (!m_IsOpen) 
  {
    LastErrNo=SOUNDERROR_DONTOPEN;
    return;
  }
  m_IsStarted=true;
  for(int i=0; i<MAXNUMOFBUFFER; i++)
  {
    if (LastErrNo=waveInPrepareHeader(hWaveIn, lpWaveInHdr[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)

    {
      OnStop();
      SENDLOGMSG(MSG_ERROR_LEVEL,THIS_MODULENAME,0,"Can't initialize audio capture process!");
      return;
    }
    if (LastErrNo=waveInAddBuffer(hWaveIn, lpWaveInHdr[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)    
    {
      OnStop();
      SENDLOGMSG(MSG_ERROR_LEVEL,THIS_MODULENAME,0,"Can't initialize audio capture process!");
      return;
    }
  }
  if (waveInStart(hWaveIn)!=MMSYSERR_NOERROR)
  {
    OnStop();
    return;
  }
  SENDLOGMSG(MSG_INFO_LEVEL,THIS_MODULENAME,0,"Audio capture process started.");
  Resume();
}

void WaveCapture::Stop()
{
  FXAutolock al(m_Lock);
  m_IsStarted=false;
  if(hWaveIn) 
  {
    TRACE("waveInStop stoped\n");
    waveInReset(hWaveIn);
    waveInStop(hWaveIn);
  }
  SendEof();
}


void WaveCapture::OnStart()
{
  Start();
  CCaptureGadget::OnStart();
}

void WaveCapture::OnStop()
{
  CCaptureGadget::OnStop();
  Stop();
}

CDataFrame* WaveCapture::GetNextFrame(double* StartTime)
{
  FXAutolock al(m_Lock);
  CWaveFrame* pDataFrame=NULL;
  pWaveData pData=NULL;
  while(m_IsStarted)
  {
    if (WaitForSingleObject(m_hEvent,BUFFERTIME)==WAIT_OBJECT_0)
    {
      *StartTime=GetHRTickCount();
      pData=(pWaveData)malloc(sizeof(WaveData));
      pData->data=m_lpWaveHdr;
      pData->hWaveIn=NULL;
      pData->waveformat=&WaveFormat;
      pDataFrame = CWaveFrame::Create(pData);
      if (pData)
      {
        free(pData);
        pData=NULL;
      }
      if (hWaveIn)
      {
        waveInUnprepareHeader(hWaveIn, lpWaveInHdr[m_lpWaveHdr->dwUser], sizeof(WAVEHDR));
        waveInPrepareHeader  (hWaveIn, lpWaveInHdr[m_lpWaveHdr->dwUser], sizeof(WAVEHDR));
        waveInAddBuffer      (hWaveIn, lpWaveInHdr[m_lpWaveHdr->dwUser], sizeof(WAVEHDR));
      }
      break;
    }
  }
  return pDataFrame;
}

void WaveCapture::SendEof()
{
  CWaveFrame* pDataFrame = CWaveFrame::Create(NULL);
  if (!m_pOutput->Put(pDataFrame))
  {
    pDataFrame->Release();
  }
}

bool WaveCapture::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  pk.WriteInt("MCIDeviceID",m_wMCIDeviceID);
  pk.WriteInt("NChannels" , m_iNChannels ) ;
  text+=pk;
  return CCaptureGadget::PrintProperties(text);
}

bool WaveCapture::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  CCaptureGadget::ScanProperties(text,Invalidate);
  FXPropertyKit pk(text);
  pk.GetInt("MCIDeviceID",(int&)m_wMCIDeviceID);
  pk.GetInt("NChannels" , m_iNChannels ) ;
  Init();

  return true;
}

bool WaveCapture::ScanSettings(FXString& text)
{
  WAVEINCAPS InDevCaps;
  text="template(ComboBox(MCIDeviceID(";
  for (unsigned int j=0; j<waveInGetNumDevs(); j++)
  {
    FXString Item;
    waveInGetDevCaps(j, &InDevCaps, sizeof(WAVEINCAPS));
    Item.Format("%s(%d),",FxRegularize(InDevCaps.szPname),j);
    text+=Item;
  }
  text+="))," ;
  text += "Spin(NChannels,1,7)" ;

  text +=  ")";
  return true; 
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//WavFileCapture
//
IMPLEMENT_RUNTIME_GADGET_EX(WavFileCapture, CCaptureGadget, "Wave", TVDB400_PLUGIN_NAME);

WavFileCapture::WavFileCapture():
  SBSetupHelper(MAXNUMOFBUFFER,BUFFERTIME),
  m_bLoop(FALSE)
{
  m_fName=AfxGetApp()->GetProfileString("root", "FilePath", "C:\\test.wav");
  m_IsOpen=false;
  m_IsStarted=false;
  m_hFile=NULL;
  m_pOutput = new COutputConnector(wave);
  m_SetupObject = new CWaveFileCaptureSetupDlg(this, NULL);
  OpenWavFile();
}

void WavFileCapture::ShutDown()
{ 
  OnStop();
  if (m_IsOpen) 
  {
    for(int i=0; i<iMaxBufNum; i++)
    {
      if (hWaveInHdr[i] != NULL)
      {
        waveInUnprepareHeader(hWaveIn,lpWaveInHdr[i],sizeof(WAVEHDR));
        GlobalUnlock(hWaveInHdr[i]);
        GlobalFree(hWaveInHdr[i]);
        GlobalUnlock(hInBuffer[i]);
        GlobalFree(hInBuffer[i]);
        hWaveInHdr[i] = NULL;
        hInBuffer[i]  = NULL;
        lpWaveInHdr[i]= NULL;
        lpInBuffer[i] = NULL;
      }
    }
    m_IsOpen=false;
  }
  AfxGetApp()->WriteProfileString("root", "FilePath", m_fName);
  CCaptureGadget::ShutDown();
  delete m_pOutput;
  m_pOutput = NULL;
}

bool WavFileCapture::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  pk.WriteString("FileName", m_fName);
  pk.WriteInt("Loop", m_bLoop);
  text = (LPCTSTR)pk;
  return true;
}

bool WavFileCapture::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXPropertyKit pk(text);
  FXString fName;
  if (pk.GetString("FileName", fName))
  {
    m_fName = fName;
    OpenWavFile();
  }
  pk.GetInt("Loop", m_bLoop);
  return true;
}

bool WavFileCapture::ScanSettings(FXString& text) 
{ 
  text="calldialog(true)";
  return true;
}

void WavFileCapture::OpenWavFile()
{
  FXFileStatus fs;
  int i;

  if ((!FXFile::GetStatus(m_fName,fs)) || (fs.m_size==0))
  {
    SENDLOGMSG(MSG_ERROR_LEVEL,THIS_MODULENAME,0,GetMMErrorMessage(SOUNDERROR_NOFILE));
    return;
  }
  m_IsEof=false;    
  MMCKINFO MMCKInfoData;
  MMCKINFO MMCKInfoParent;
  MMCKINFO MMCKInfoChild;
  MMRESULT mmResult;

  ZeroMemory(&MMCKInfoParent,sizeof(MMCKINFO));
  ZeroMemory(&MMCKInfoChild,sizeof(MMCKINFO));
  ZeroMemory(&MMCKInfoData,sizeof(MMCKINFO));

  m_hFile=::mmioOpen((char*)(LPCTSTR)m_fName,NULL,MMIO_READ); 
  if(m_hFile == NULL) 
  {
    m_IsEof=true; 
    SENDLOGMSG(MSG_ERROR_LEVEL,THIS_MODULENAME,0,GetMMErrorMessage(SOUNDERROR_NOFILE));
    return;
  }
  MMCKInfoParent.fccType = mmioFOURCC('W','A','V','E');
  if ((mmResult = ::mmioDescend(m_hFile, &MMCKInfoParent,NULL,MMIO_FINDRIFF))!=MMSYSERR_NOERROR)
  {
    SENDLOGMSG(MSG_ERROR_LEVEL,THIS_MODULENAME,0,GetMMErrorMessage(SOUNDERROR_NOFILE));
    return;
  }

  MMCKInfoChild.ckid = mmioFOURCC('f','m','t',' ');
  if ((mmResult = mmioDescend(m_hFile,&MMCKInfoChild,&MMCKInfoParent,MMIO_FINDCHUNK))!=MMSYSERR_NOERROR)
  {
    SENDLOGMSG(MSG_ERROR_LEVEL,THIS_MODULENAME,0,GetMMErrorMessage(SOUNDERROR_NOFILE));
    return;
  }

  memset(&WaveFormat,0,sizeof(WAVEFORMATEX));
  if ((m_bytesRead = mmioRead(m_hFile,(LPSTR)&WaveFormat,MMCKInfoChild.cksize))<0)
  {
    SENDLOGMSG(MSG_ERROR_LEVEL,THIS_MODULENAME,0,GetMMErrorMessage(SOUNDERROR_NOFILE));
    return;
  }

  dwBufferSize=WaveFormat.nAvgBytesPerSec*BUFFERTIME/1000;

  if ((mmResult = mmioAscend(m_hFile,&MMCKInfoChild,0))!=MMSYSERR_NOERROR)
  {
    SENDLOGMSG(MSG_ERROR_LEVEL,THIS_MODULENAME,0,GetMMErrorMessage(SOUNDERROR_NOFILE));
    return;
  }
  MMCKInfoChild.ckid = mmioFOURCC('d','a','t','a');
  if ((mmResult = mmioDescend(m_hFile,&MMCKInfoChild,
    &MMCKInfoParent,MMIO_FINDCHUNK))!=MMSYSERR_NOERROR)
  {
    SENDLOGMSG(MSG_ERROR_LEVEL,THIS_MODULENAME,0,GetMMErrorMessage(SOUNDERROR_NOFILE));
    return;
  }

  if (m_IsOpen) {
    for(int i=0; i<iMaxBufNum; i++)
    {
      if (hWaveInHdr[i] != NULL)
      {
        waveInUnprepareHeader(hWaveIn,lpWaveInHdr[i],sizeof(WAVEHDR));
        GlobalUnlock(hWaveInHdr[i]);
        GlobalFree(hWaveInHdr[i]);
        GlobalUnlock(hInBuffer[i]);
        GlobalFree(hInBuffer[i]);
        hWaveInHdr[i] = NULL;
        hInBuffer[i]  = NULL;
        lpWaveInHdr[i]= NULL;
        lpInBuffer[i] = NULL;
      }
    }
  }
  for(i=0; i<iMaxBufNum; i++)
  {
    hWaveInHdr[i] = GlobalAlloc(GHND | GMEM_SHARE , sizeof(WAVEHDR));
    lpWaveInHdr[i] = (LPWAVEHDR)GlobalLock(hWaveInHdr[i]);
    hInBuffer[i] = GlobalAlloc(GHND | GMEM_SHARE , dwBufferSize);
    lpInBuffer[i] = (LPBYTE)GlobalLock(hInBuffer[i]);
    lpWaveInHdr[i]->lpData = (LPSTR)lpInBuffer[i];
    lpWaveInHdr[i]->dwBufferLength = dwBufferSize;
    lpWaveInHdr[i]->dwBytesRecorded = 0L;
    lpWaveInHdr[i]->dwUser = (DWORD_PTR)this;
    lpWaveInHdr[i]->dwFlags = WHDR_DONE;
    lpWaveInHdr[i]->dwLoops = 1L;
    lpWaveInHdr[i]->lpNext = NULL;
    lpWaveInHdr[i]->reserved = 0L;
  }
  m_CrntBuffer=0;
  m_BuffersRead=0;
  m_bytesRead=0;
  m_IsOpen=true;
  SENDLOGMSG(MSG_DEBUG_LEVEL,THIS_MODULENAME, 0,"Audio file successfully opened.");
  return;
}

void WavFileCapture::OnStart()
{
  if (!m_IsOpen)
  {
    OpenWavFile();
    if (!m_IsOpen)
    {
      OnStop();
      Sleep(1000);
      return;
    }
  }
  if (m_IsOpen&&!m_IsStarted)
  {
    m_IsEof=false;
    m_IsStarted=true;
    Resume();
    CCaptureGadget::OnStart();
    return;
  }
}

void WavFileCapture::OnStop()
{
  if (m_IsOpen)
  {
    if (m_IsStarted)
    {
      m_IsStarted=false;
      m_IsEof=true;
      ::mmioClose(m_hFile, 0);
      m_hFile = NULL;
    }
    m_IsOpen=false;
  }
  SendEof();
  Sleep(10);
  CCaptureGadget::OnStop();
}

void WavFileCapture::SendEof()
{
  CWaveFrame* pDataFrame = CWaveFrame::Create(NULL);
  if (!m_pOutput->Put(pDataFrame))
  {
    pDataFrame->Release();
  }
}

CDataFrame* WavFileCapture::GetNextFrame(double* StartTime)
{
  CWaveFrame* pDataFrame=NULL;
  //!!!TODO:put timer into class
  HANDLE hCurrThread=::GetCurrentThread();
  while(m_IsStarted&&!m_IsEof)
  {
    if (lpWaveInHdr[m_CrntBuffer]->dwFlags!=WHDR_DONE)
    {
      ::WaitForSingleObject(hCurrThread, 1);
      continue;
    }
    //    TRACE("+++  Is about to read next buffer\n");
    lpWaveInHdr[m_CrntBuffer]->dwBytesRecorded = ::mmioRead(m_hFile, lpWaveInHdr[m_CrntBuffer]->lpData, lpWaveInHdr[m_CrntBuffer]->dwBufferLength);
    m_bytesRead+=lpWaveInHdr[m_CrntBuffer]->dwBytesRecorded;
    //    TRACE("+++   Read %d bytes, whole bytes read %d\n",lpWaveInHdr[m_CrntBuffer]->dwBytesRecorded,m_bytesRead);

    m_BuffersRead++;

    ::WaitForSingleObject(hCurrThread, BUFFERTIME);
    if (lpWaveInHdr[m_CrntBuffer]->dwBytesRecorded==0)
    {
      TRACE("+++    End of file\n");
      if (m_bLoop)
      {
        SendEof();
        ::mmioSeek(m_hFile, 0L, SEEK_SET);
        continue;
      }
      else
      {
        m_IsEof=true;
        continue;
      }
    }

    pWaveData pData=NULL;
    pData=(pWaveData)malloc(sizeof(WaveData));
    pData->data=lpWaveInHdr[m_CrntBuffer];
    pData->hWaveIn=NULL;
    pData->waveformat=&WaveFormat;
    pDataFrame = CWaveFrame::Create(pData);
    if (pData)
    {
      free(pData);
      pData=NULL;
    }

    m_CrntBuffer=(m_CrntBuffer+1)%iMaxBufNum;
    break;
  }
  //    if (lpWaveInHdr[m_CrntBuffer])
  //        TRACE("+++ Buffer #%d, flags: 0x%x\n",m_CrntBuffer,lpWaveInHdr[m_CrntBuffer]->dwFlags);
  //    else
  //        TRACE("+++ Buffer #%d, empty buffer\n",m_CrntBuffer);
  return pDataFrame;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//WaveSpeaker
//

IMPLEMENT_RUNTIME_GADGET_EX(WaveSpeaker, CRenderGadget, "Wave", TVDB400_PLUGIN_NAME);

WaveSpeaker::WaveSpeaker():
  m_pSetupDlg(NULL)
  //    m_Mode(SPEAKER_OUT)

{
  CString str;
  str = AfxGetApp()->GetProfileString("root", "SpeakerID", "0");
  m_wMCIDeviceID = atoi(str);
  if (m_wMCIDeviceID>waveOutGetNumDevs()-1) 
    m_wMCIDeviceID=0;

  m_BuffersNum=MAXNUMOFBUFFER;
  hWaveInHdr    =new GLOBALHANDLE[m_BuffersNum];
  lpWaveHdr     =new LPWAVEHDR[m_BuffersNum];
  hOutBuffer    =new GLOBALHANDLE[m_BuffersNum];
  memset(hWaveInHdr,0,sizeof(GLOBALHANDLE)*m_BuffersNum);
  memset(lpWaveHdr,0,sizeof(LPWAVEHDR)*m_BuffersNum);
  memset(hOutBuffer,0,sizeof(GLOBALHANDLE)*m_BuffersNum);

  m_BufferPntr=0;

  for (int i=0; i<m_BuffersNum; i++)
  {
    hWaveInHdr[i]=GlobalAlloc(GHND | GMEM_SHARE , sizeof(WAVEHDR));
    lpWaveHdr[i]=(LPWAVEHDR)GlobalLock(hWaveInHdr[i]);
    memset(lpWaveHdr[i],0,sizeof(WAVEHDR));
  }
  SetMonitor(SET_INPLACERENDERERMONITOR);
  m_pInput = new CInputConnector(wave);
  Resume();
}

void WaveSpeaker::ShutDown()
{
  if (m_pSetupDlg->GetSafeHwnd( ))
    m_pSetupDlg->DestroyWindow();
  if (m_pSetupDlg)
    m_pSetupDlg->Delete();
  m_pSetupDlg = NULL;

  CString str;
  str.Format("%d", m_wMCIDeviceID);
  AfxGetApp()->WriteProfileString("root", "SpeakerID", str);
  CloseDevice();
  for (int i=0; i<m_BuffersNum; i++)
  {
    if (hWaveInHdr[i]) {GlobalUnlock(hWaveInHdr[i]); GlobalFree(hWaveInHdr[i]); lpWaveHdr[i]=NULL; }
    if (hOutBuffer[i]) {GlobalUnlock(hOutBuffer[i]); GlobalFree(hOutBuffer[i]); hOutBuffer[i]=NULL; }
  }
  CRenderGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete [] hWaveInHdr;
  delete [] lpWaveHdr;
  delete [] hOutBuffer;
}

void WaveSpeaker::copyWAVEHDR(LPWAVEHDR pwh)
{
  LPWAVEHDR dst=lpWaveHdr[m_BufferPntr];
  while (((dst->dwFlags & WHDR_DONE)==0) && (dst->dwBufferLength!=0))
  {
    Sleep(1);
  }
  DWORD_PTR dwUser=lpWaveHdr[m_BufferPntr]->dwUser;
  if (dst->dwBufferLength==pwh->dwBufferLength)
  {
    LPSTR lpDst=dst->lpData;
    memcpy(lpDst,pwh->lpData,pwh->dwBytesRecorded);
    dst->dwBufferLength=pwh->dwBufferLength;
    dst->dwBytesRecorded=pwh->dwBytesRecorded;
    dst->dwFlags=WHDR_BEGINLOOP | WHDR_ENDLOOP | WHDR_DONE;
    lpWaveHdr[m_BufferPntr]->dwUser=dwUser;
  }
  else
  {
    if (dst->lpData) 
    {
      GlobalUnlock(hOutBuffer[m_BufferPntr]); GlobalFree(hOutBuffer[m_BufferPntr]);
    }
    memcpy(dst,pwh,sizeof(WAVEHDR));
    hOutBuffer[m_BufferPntr] = GlobalAlloc(GHND | GMEM_SHARE , pwh->dwBufferLength);
    dst->lpData=(char*)GlobalLock(hOutBuffer[m_BufferPntr]);
    memcpy(dst->lpData,pwh->lpData,pwh->dwBytesRecorded);
    dst->dwBufferLength=pwh->dwBufferLength;
    dst->dwBytesRecorded=pwh->dwBytesRecorded;
    dst->dwLoops=pwh->dwLoops;
    dst->dwUser=(DWORD_PTR)(this);
    dst->dwFlags=WHDR_BEGINLOOP | WHDR_ENDLOOP | WHDR_DONE;
    lpWaveHdr[m_BufferPntr]->dwUser=dwUser;
  }
}

void WaveSpeaker::Render(const CDataFrame* pDataFrame)
{
  //    m_Lock.Lock();
  const CWaveFrame* wd=pDataFrame->GetWaveFrame(DEFAULT_LABEL);
  if (!wd || !((CWaveFrame*)wd)->GetData()->waveformat) // empty waveformat means reset
  {
    TRACE("+++ Reset renderer\n");
    CloseDevice();
    //        m_Lock.Unlock();
    return;
  }
  if (!hWaveOut)
  {
    InitDevice(((CWaveFrame*)wd)->GetData()->waveformat);
  }
  //    switch (m_Mode)
  //    {
  //    case SPEAKER_OUT:
  //        {
  copyWAVEHDR(((CWaveFrame*)wd)->GetData()->data);

  waveOutPrepareHeader(hWaveOut,lpWaveHdr[m_BufferPntr],sizeof(WAVEHDR));
  waveOutWrite(hWaveOut,lpWaveHdr[m_BufferPntr],sizeof(WAVEHDR));
  m_BufferPntr=(m_BufferPntr+1)%m_BuffersNum;
  ((CWaveFrame*)wd)->GetData()->data->dwFlags=0;
  //            break;
  //        }
  //    }
  //    m_Lock.Unlock();
}

bool WaveSpeaker::InitDevice(WAVEFORMATEX*  WaveFormat)
{
  CloseDevice(); 
  waveOutOpen(&hWaveOut,m_wMCIDeviceID,WaveFormat,(DWORD)0, NULL, CALLBACK_FUNCTION);
  return true;
}

void WaveSpeaker::CloseDevice()
{
  if (hWaveOut) 
  {
    waveOutReset(hWaveOut);
    for(int i=0; i<m_BuffersNum; i++)
      waveOutUnprepareHeader(hWaveOut,lpWaveHdr[i],sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
    hWaveOut=NULL;
  }
}
/*
void WaveSpeaker::ShowSetupDialog(CPoint& point)
{
FXString DlgHead;
GetGadgetName(DlgHead);
DlgHead.Append(" Setup Dialog");
if (!m_pSetupDlg)
{
m_pSetupDlg = new WaveSpeakerSetupDlg(this, NULL);
}
if (!m_pSetupDlg->m_hWnd)
{
m_pSetupDlg->m_ComboIndex=m_wMCIDeviceID;
if (!m_pSetupDlg->Create(IDD_SPEAKERSETUPDLG, NULL))
{
SENDERR_0("Failed to create Setup Dialog");
delete m_pSetupDlg;
m_pSetupDlg = NULL;
return;
}
}
m_pSetupDlg->SetWindowText(DlgHead);
m_pSetupDlg->SetWindowPos(NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
m_pSetupDlg->ShowWindow(SW_SHOWNORMAL);
}*/


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//WavOscillograph
//

IMPLEMENT_RUNTIME_GADGET_EX(WavOscillograph, CRenderGadget, "Wave", TVDB400_PLUGIN_NAME);
