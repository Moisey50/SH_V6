#include "stdafx.h"
#include <math\intf_sup.h>
#include "WavSpectrum.h"
#include "WavGadgets.h"
#include <gadgets\WaveFrame.h>
#include <imageproc\conv.h>
#include <gadgets\ArrayFrame.h>
#include <gadgets\QuantityFrame.h>
#include <gadgets\ContainerFrame.h>


void _calc_AmpPh(void* Re, void* Im, int count, int size, DPOINT** ppAmp, DPOINT** ppPh)
{
  if (size == sizeof(DPOINT))
  {
    DPOINT* pRe = (DPOINT*)Re;
    DPOINT* pIm = (DPOINT*)Im;
    *ppAmp = (DPOINT*)calloc(count, sizeof(DPOINT));
    *ppPh = (DPOINT*)calloc(count, sizeof(DPOINT));
    for (int i = 0; i < count; i++)
    {
      (*ppAmp)[i].x = pRe[i].x;
      (*ppAmp)[i].y = sqrt(pRe[i].y * pRe[i].y + pIm[i].y * pIm[i].y);
      (*ppPh)[i].x = pRe[i].x;
      (*ppPh)[i].y = atan2(pIm[i].y, pRe[i].y);
    }
  }
  else if (size == sizeof(double))
  {
    double* pRe = (double*)Re;
    double* pIm = (double*)Im;
    *ppAmp = (DPOINT*)calloc(count, sizeof(DPOINT));
    *ppPh = (DPOINT*)calloc(count, sizeof(DPOINT));
    for (int i = 0; i < count; i++)
    {
      (*ppAmp)[i].x = (double)i;
      (*ppAmp)[i].y = sqrt(pRe[i] * pRe[i] + pIm[i] * pIm[i]);
      (*ppPh)[i].x = (double)i;
      (*ppPh)[i].y = atan2(pIm[i], pRe[i]);
    }
  }
}

void _calc_ReIm(DPOINT* Amp, DPOINT* Ph, int count, double* pRe, double* pIm)
{
  for (int i = 0; i < count; i++)
  {
    pRe[i] = Amp[i].y * cos(Ph[i].y);
    pIm[i] = Amp[i].y * sin(Ph[i].y);
  }
}


IMPLEMENT_RUNTIME_GADGET_EX(WavSpectrum, CFilterGadget, "Wave", TVDB400_PLUGIN_NAME);

WavSpectrum::WavSpectrum():
  m_pBuffer(NULL),
  m_BufferSize(0),
  m_DataWritten(0),
  m_nBitsPerSample(0),
  m_nRange(10)
{
  m_pInput = new CInputConnector(wave);
  m_pOutput = new COutputConnector(quantity * arraytype);
  m_pRe = (double*)calloc(1024, sizeof(double));
  m_pIm = (double*)calloc(1024, sizeof(double));
  m_LUT = _prepare_fft_lut(m_nRange, 1);
  Resume();
}

void WavSpectrum::Reset()
{
  FXAutolock al(m_BufferLock);
  m_DataWritten = 0;
}

void WavSpectrum::AppendData(void* pData, DWORD cData)
{
  FXAutolock al(m_BufferLock);
  if (!cData)
    return;
  if (m_BufferSize - m_DataWritten < cData)
  {
    m_pBuffer = realloc(m_pBuffer, cData + m_DataWritten);
    m_BufferSize = cData + m_DataWritten;
  }
  memcpy((LPBYTE)m_pBuffer + m_DataWritten, pData, cData);
  m_DataWritten += cData;
}

void* WavSpectrum::GetLastBlock()
{
  FXAutolock al(m_BufferLock);
  DWORD szWnd = (1 << m_nRange);
  DWORD cbBlock = szWnd * ((DWORD)m_nBitsPerSample / 8);
  if (!cbBlock || (m_DataWritten < cbBlock))
    return NULL;
  return (void*)m_pBuffer;
}

void WavSpectrum::DropBlock()
{
  FXAutolock al(m_BufferLock);
  DWORD szWnd = (1 << m_nRange);
  DWORD cbBlock = szWnd * ((DWORD)m_nBitsPerSample / 8);
  m_DataWritten -= cbBlock;
  if (m_DataWritten > 0)
    memmove(m_pBuffer, (LPBYTE)m_pBuffer + cbBlock, m_DataWritten);
}

void WavSpectrum::CountSpectrum(void* pData)
{
  DWORD szWnd = (1 << m_nRange);
  memset(m_pIm, 0, szWnd * sizeof(double));
  if (m_nBitsPerSample == 8)
  {
    signed char *src = (signed char*)pData, *end = src + szWnd;
    double* dst = m_pRe;
    while (src < end)
    {
      *dst = (double)*src;
      src++;
      dst++;
    }
  }
  else if (m_nBitsPerSample == 16)
  {
    short *src = (short*)pData, *end = src + szWnd;
    double* dst = m_pRe;
    while (src < end)
    {
      *dst = (double)*src;
      src++;
      dst++;
    }
  }
  else
    return;
  _cfft_1(m_pRe, m_pIm, m_nRange, m_LUT);
}

void WavSpectrum::ShutDown()
{
  FXAutolock lock(m_Lock);
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
  free(m_pBuffer);
  m_pBuffer = NULL;
  m_DataWritten = 0;
  m_BufferSize = 0;
  free(m_pRe);
  m_pRe = NULL;
  free(m_pIm);
  m_pIm = NULL;
  free(m_LUT);
  m_LUT = NULL;
}

CDataFrame* WavSpectrum::DoProcessing(const CDataFrame* pDataFrame)
{
  FXAutolock lock(m_Lock);
  CContainerFrame* pResultFrame = NULL;
  const CWaveFrame* pWaveFrame = pDataFrame->GetWaveFrame();
  if (!pWaveFrame || pWaveFrame->IsNullFrame())
  {
    Reset();
    return NULL;
  }
  if (pWaveFrame->waveformat->wBitsPerSample != (WORD)m_nBitsPerSample)
  {
    Reset();
    m_nBitsPerSample = (BYTE)pWaveFrame->waveformat->wBitsPerSample;
  }
  AppendData(pWaveFrame->data->lpData, pWaveFrame->data->dwBytesRecorded);
  void* pBlock;
  while (pBlock= GetLastBlock())
  {
    CountSpectrum(pBlock);
    DropBlock();
    DWORD szWnd = (1 << m_nRange);
    DWORD szOut = szWnd; //szWnd/2;
    double fMin = (double)pWaveFrame->waveformat->nSamplesPerSec / (double)szWnd;
    CArrayFrame* pArrayFrameRe = CArrayFrame::Create(quantity, &CQuantityFrame::CreateFrom, sizeof(DPOINT));
    CArrayFrame* pArrayFrameIm = CArrayFrame::Create(quantity, &CQuantityFrame::CreateFrom, sizeof(DPOINT));
    pArrayFrameRe->CopyAttributes(pDataFrame);
    pArrayFrameIm->CopyAttributes(pDataFrame);
    DPOINT* pt = (DPOINT*)calloc(szOut , sizeof(DPOINT));
    pt[0].x = 0;
    pt[0].y = m_pRe[0];
    for (int i = 1; i < (int)szOut; i++)
    {
      pt[i].x = (double)i * fMin;
      pt[i].y = m_pRe[i];
    }
    pArrayFrameRe->SetAt(0, pt, szOut);
    pt[0].y = m_pIm[0];
    for (int i = 1; i < (int)szOut; i++)
    {
      pt[i].y = m_pIm[i];
    }
    pArrayFrameIm->SetAt(0, pt, szOut);
    free(pt);
    pArrayFrameRe->SetLabel("Re");
    pArrayFrameIm->SetLabel("Im");
    pResultFrame = CContainerFrame::Create();
    pResultFrame->CopyAttributes(pDataFrame);
    pResultFrame->AddFrame(pArrayFrameRe);
    pResultFrame->AddFrame(pArrayFrameIm); 
    if ((m_pOutput) && (!m_pOutput->Put(pResultFrame)))
      pResultFrame->RELEASE(pResultFrame);
  }
  return NULL;
}

bool WavSpectrum::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  CFilterGadget::ScanProperties(text, Invalidate);
  FXPropertyKit pk(text);
  int range;
  if (pk.GetInt("Range", range) && (range != (int)m_nRange))
  {
    FXAutolock lock(m_Lock);
    if (range > (int)m_nRange)
    {
      DWORD szWnd = (1 << range);
      m_pRe = (double*)realloc(m_pRe, szWnd * sizeof(double));
      m_pIm = (double*)realloc(m_pIm, szWnd * sizeof(double));
    }
    free(m_LUT);
    m_LUT = _prepare_fft_lut(range, 1);
    m_nRange = range;
  }
  return true;
}

bool WavSpectrum::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  CFilterGadget::PrintProperties(text);
  pk.WriteInt("Range", m_nRange);
  text += pk;
  return true;
}

bool WavSpectrum::ScanSettings(FXString& text)
{
  text.Format("template(ComboBox(Range(512(%d),1024(%d))))", 9, 10);
  return true;
}

/////////////////////////////////////////////////////////////////////////
// ReIm2AmpPh

IMPLEMENT_RUNTIME_GADGET_EX(ReIm2AmpPh, CFilterGadget, "Wave", TVDB400_PLUGIN_NAME);

ReIm2AmpPh::ReIm2AmpPh()
{
  m_pInput = new CInputConnector(quantity * arraytype);
  m_pOutput = new COutputConnector(quantity * arraytype);
  Resume();
}

void ReIm2AmpPh::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
}

CDataFrame* ReIm2AmpPh::DoProcessing(const CDataFrame* pDataFrame)
{
  CContainerFrame* pRetFrame=NULL;
  CFramesIterator* pIterator = pDataFrame->CreateFramesIterator(quantity);
  if (!pIterator)
  {
    return NULL;
  }
  const CDataFrame* pNextFrame = pIterator->Next(NULL);

  int nRe = 0, nIm = 0, ceRe = 0, ceIm = 0;
  void* Re = NULL, *Im = NULL;
  const CArrayFrame* ReFrame = NULL;
  const CArrayFrame* ImFrame = NULL;
  while (pNextFrame)
  {
    ReFrame = pNextFrame->GetArrayFrame("Re");
    if (ReFrame)
    {
      nRe = ReFrame->GetCount();
      ceRe = (int)ReFrame->GetElementSize();
      Re = ReFrame->GetData();
      if (ImFrame)
        break;
    }
    else
    {
      ImFrame = pNextFrame->GetArrayFrame("Im");
      if (ImFrame)
      {
        nIm = ImFrame->GetCount();
        ceIm = (int)ImFrame->GetElementSize();
        Im = ImFrame->GetData();
        if (ReFrame)
          break;
      }
    }
    pNextFrame = pIterator->Next(NULL);
  }
  delete pIterator;
  DPOINT *Amp = NULL, *Ph = NULL;
  if (Im && Re && nRe == nIm && ceRe == ceIm)
  {
    _calc_AmpPh(Re, Im, nRe, ceRe, &Amp, &Ph);
  }
  if (Amp && Ph)
  {
    pRetFrame=CContainerFrame::Create();
    pRetFrame->CopyAttributes(pDataFrame);
    CArrayFrame* pAmpFrame = CArrayFrame::Create(quantity, &CQuantityFrame::CreateFrom, sizeof(DPOINT));
    pAmpFrame->CopyAttributes(pDataFrame);
    pAmpFrame->SetLabel("Amp");
    pAmpFrame->SetAt(0, Amp, nRe);
    pRetFrame->AddFrame(pAmpFrame);
    CArrayFrame* pPhFrame = CArrayFrame::Create(quantity, &CQuantityFrame::CreateFrom, sizeof(DPOINT));
    pPhFrame->CopyAttributes(pDataFrame);
    pPhFrame->SetLabel("Ph");
    pPhFrame->SetAt(0, Ph, nRe);
    pRetFrame->AddFrame(pPhFrame);
  }
  free(Amp);
  free(Ph);
  return pRetFrame;
}


/////////////////////////////////////////////////////////////////////////
// Spectrum2Wave

IMPLEMENT_RUNTIME_GADGET_EX(Spectrum2Wave, CFilterGadget, "Wave", TVDB400_PLUGIN_NAME);

Spectrum2Wave::Spectrum2Wave():
  m_nRange(10),
  m_bStartLoop(true)
{
  m_pInput = new CInputConnector(quantity * arraytype);
  m_pOutput = new COutputConnector(wave);
  m_LUT = _prepare_fft_lut(m_nRange, -1);
  int wnd = (1 << m_nRange);
  m_pRe = (double*)calloc(wnd, sizeof(double));
  m_pIm = (double*)calloc(wnd, sizeof(double));
  Resume();
}

void Spectrum2Wave::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
  free(m_LUT);
  m_LUT = NULL;
  free(m_pRe);
  m_pRe = NULL;
  free(m_pIm);
  m_pIm = NULL;
}

CDataFrame* Spectrum2Wave::DoProcessing(const CDataFrame* pDataFrame)
{
  const CArrayFrame* ReFrame = NULL;
  const CArrayFrame* ImFrame = NULL;
  const CArrayFrame* AmpFrame = NULL;
  const CArrayFrame* PhFrame = NULL;
  if (pDataFrame->IsContainer())
  {
    CFramesIterator* pIterator = pDataFrame->CreateFramesIterator(quantity);
    if (!pIterator)
      return NULL;

    CDataFrame* pNextFrame = pIterator->Next(NULL);
    while (pNextFrame)
    {
      if (!ReFrame)
        ReFrame = pNextFrame->GetArrayFrame("Re");
      if (!ImFrame)
        ImFrame = pNextFrame->GetArrayFrame("Im");
      if (!AmpFrame)
        AmpFrame = pNextFrame->GetArrayFrame("Amp");
      if (!PhFrame)
        PhFrame = pNextFrame->GetArrayFrame("Ph");
      pNextFrame = pIterator->Next();
    }
    delete pIterator;
  }
  else
  {
    if (_tcscmp(pDataFrame->GetLabel(),_T("Re"))==0)
      ReFrame = pDataFrame->GetArrayFrame();
    else if (_tcscmp(pDataFrame->GetLabel(),_T("Im"))==0)
      ImFrame = pDataFrame->GetArrayFrame();
    else if (_tcscmp(pDataFrame->GetLabel(),_T("Amp"))==0)
      AmpFrame = pDataFrame->GetArrayFrame();
    else if (_tcscmp(pDataFrame->GetLabel(),_T("Ph"))==0)
      PhFrame = pDataFrame->GetArrayFrame();
  }
  pWaveData wd = NULL;
  if (AmpFrame && PhFrame)
    wd = CalcFromAmpPh(AmpFrame, PhFrame);
  else if (AmpFrame)
    wd = CalcFromAmp(AmpFrame);
  else if (ReFrame && ImFrame)
    wd = CalcFromReIm(ReFrame, ImFrame);

  if (!wd)
    return NULL;

  CWaveFrame* wFrame = CWaveFrame::Create(wd);
  wFrame->CopyAttributes(pDataFrame);

  free(wd->data->lpData);
  free(wd->waveformat);
  free(wd->data);
  free(wd);

  m_bStartLoop = false;
  return wFrame;
}

pWaveData Spectrum2Wave::CalcFromAmpPh(const CArrayFrame* AmpFrame, const CArrayFrame* PhFrame)
{
  ASSERT(AmpFrame->GetCount()==PhFrame->GetCount());
  int count = AmpFrame->GetCount();
  m_nRange = ROUND(log10((double)count)/log10(2.));

  UINT ce = AmpFrame->GetElementSize();
  void* Re = AmpFrame->GetData();
  if (count != PhFrame->GetCount())
    return NULL;
  if (ce != PhFrame->GetElementSize() || ce != sizeof(DPOINT))
    return NULL;
  void* Im = PhFrame->GetData();
  if (!InitBlock((DPOINT*)Re, (DPOINT*)Im, count, false))
    return NULL;
  _calc_ReIm((DPOINT*)Re, (DPOINT*)Im, count, m_pRe, m_pIm);
  for (int i = 0; i < count; i++)
  {
    m_pRe[count + i] = m_pRe[count - 1 - i];
    m_pIm[count + i] = -m_pIm[count - 1 - i];
    //		m_pRe[count - 1 - i] = m_pRe[count + i];
    //		m_pIm[count - 1 - i] = m_pIm[count + i];
  }
  _cfft_1(m_pRe, m_pIm, m_nRange, m_LUT);
  double fMax = ((DPOINT*)Re)[count - 1].x;
  return CreateWave(fMax);
}

pWaveData Spectrum2Wave::CalcFromAmp(const CArrayFrame* AmpFrame)
{
  return NULL;
}

pWaveData Spectrum2Wave::CalcFromReIm(const CArrayFrame* ReFrame, const CArrayFrame* ImFrame)
{
  ASSERT(ReFrame->GetCount()==ImFrame->GetCount());
  int count = ReFrame->GetCount(); 
  m_nRange = ROUND(log10((double)count)/log10(2.));
  UINT ce = ReFrame->GetElementSize();
  void* Re = ReFrame->GetData();
  if (count != ImFrame->GetCount())
    return NULL;
  if (ce != ImFrame->GetElementSize() || ce != sizeof(DPOINT))
    return NULL;
  void* Im = ImFrame->GetData();
  if (!InitBlock((DPOINT*)Re, (DPOINT*)Im, count, true))
    return NULL;
  _cfft_1(m_pRe, m_pIm, m_nRange, m_LUT);
  double fMax = ((DPOINT*)Re)[1].x*count;
  return CreateWave(fMax);
}

bool Spectrum2Wave::InitBlock(DPOINT* Re, DPOINT* Im, int count, bool bCopyData)
{
  {
    int wnd = (1 << m_nRange);
    if (!bCopyData)
      return true;
    DPOINT* srcRe = (DPOINT*)Re;
    DPOINT* srcIm = (DPOINT*)Im;
    double* dstRe = m_pRe;
    double* dstIm = m_pIm;
    double* end = dstRe + wnd;
    while (dstRe < end)
    {
      *dstRe = srcRe->y;
      *dstIm = srcIm->y;
      dstRe++;
      dstIm++;
      srcRe++;
      srcIm++;
    }
  }
  return true;
  // OLD CODE
  int wnd = (1 << m_nRange) / 2, r = m_nRange;
  while (wnd > count)
  {
    wnd /= 2;
    r--;
  }
  while (wnd < count)
  {
    wnd *= 2;
    r++;
  }
  if (wnd != count)
    return false;
  if (r != m_nRange)
  {
    free(m_LUT);
    m_LUT = _prepare_fft_lut(r, -1);
    m_nRange = r;
    wnd = (1 << m_nRange);
    m_pRe = (double*)realloc(m_pRe, wnd * sizeof(double));
    m_pIm = (double*)realloc(m_pIm, wnd * sizeof(double));
  }
  else
    wnd = (1 << m_nRange);
  if (!bCopyData)
    return true;
  DPOINT* srcRe = (DPOINT*)Re;
  DPOINT* srcIm = (DPOINT*)Im;
  double* dstRe = m_pRe;
  double* dstIm = m_pIm;
  double* end = dstRe + wnd / 2;
  while (dstRe < end)
  {
    *dstRe = srcRe->y;
    *dstIm = srcIm->y;
    dstRe++;
    dstIm++;
    srcRe++;
    srcIm++;
  }
  end += wnd / 2;
  while (dstRe < end)
  {
    *dstRe = srcRe->y;
    *dstIm = srcIm->y;
    dstRe++;
    dstIm++;
    srcRe--;
    srcIm--;
  }
  return true;
}

int i=0;

pWaveData Spectrum2Wave::CreateWave(double fMax)
{
  int wnd = (1 << m_nRange);
  double* srcRe = m_pRe;
  double* end = srcRe + wnd;
  short* dst = (short*)malloc(wnd * sizeof(short));
  short* sdst=dst;
  double min, max;
  min=max=*srcRe; srcRe++;
  while (srcRe < end)
  {   
    min=(min<*srcRe)?min:*srcRe;
    max=(max>*srcRe)?max:*srcRe;
    srcRe++;
  }    
  srcRe = m_pRe;
  while (srcRe < end)
  {
    *sdst = (short)(((*srcRe-min)/(max-min)-0x7FFF)*0x7FFF);
    //*sdst = (short)(sin(0.2*i)*0x7FFF); i++;
    sdst++;
    srcRe++;
  }

  pWaveData wd = (pWaveData)malloc(sizeof(WaveData));
  wd->waveformat = (PWAVEFORMATEX)malloc(sizeof(WAVEFORMATEX));
  wd->waveformat->wFormatTag = WAVE_FORMAT_PCM;
  wd->waveformat->nChannels = 1;
  wd->waveformat->nSamplesPerSec = (DWORD)(fMax);
  wd->waveformat->wBitsPerSample = 16;
  wd->waveformat->nAvgBytesPerSec = (wd->waveformat->nChannels * wd->waveformat->nSamplesPerSec * wd->waveformat->wBitsPerSample / 8);
  wd->waveformat->nBlockAlign = (wd->waveformat->nChannels * wd->waveformat->wBitsPerSample / 8);
  wd->waveformat->cbSize = 0;

  wd->data = (LPWAVEHDR)malloc(sizeof(WAVEHDR));
  wd->data->lpData = (LPSTR)dst;
  wd->data->dwBufferLength = wnd * sizeof(short);
  wd->data->dwBytesRecorded = wd->data->dwBufferLength;
  wd->data->dwUser = 0;
  wd->data->dwFlags = WHDR_DONE;//((m_bStartLoop) ? WHDR_BEGINLOOP : 0);
  wd->data->dwLoops = 1; // play once
  wd->data->lpNext = 0;
  wd->data->reserved = 0;

  wd->hWaveIn = NULL;
  return wd;
}
