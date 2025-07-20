// ImageCorrectorGadget.h : Implementation of the ImageCorrectorGadget class


#include "StdAfx.h"
#include "ImageCorrectorGadget.h"

#define THIS_MODULENAME "ImageCorrector"

USER_FILTER_RUNTIME_GADGET(ImageCorrectorGadget, LINEAGE_FILEX );	//	Mandatory



inline bool ImageCorrectorGadget::SendOut(CVideoFrame * pVF_MonochromOrRGB, RGB_STATE eColor, bool isSnapshot)
{
  bool res = false;

  COutputConnector * pConn = GetOutputConnector(0);
  COutputConnector * pConnSnapshot = !isSnapshot ? NULL : GetOutputConnector(1);

  if (pConn && pConn->IsConnected())
  {
    //YS_2019 !IMPORTANT!
    //The reference should be added HERE in the case
    //when the Frame should be sent to TWO outputs (the SNAPSHOT mode),
    //and if this is a FIRST output pin in the SERIA of TWO!
    if (pConnSnapshot)
      pVF_MonochromOrRGB->AddRef();
    res = PutFrame(pConn, pVF_MonochromOrRGB);
  }

  if (pConnSnapshot && pConnSnapshot->IsConnected())
  {
    res = PutFrame(pConnSnapshot, pVF_MonochromOrRGB);

    if (eColor == RGB_STATE::STATE_RGB)
    {
      CTextFrame * pDone = CreateTextFrame("Done", "Done");
      PutFrame(pConnSnapshot, pDone);
    }
  }

  return res;
}

ImageCorrectorGadget::ImageCorrectorGadget()
  : m_mixerRGB() 
  , m_pVFOutput_MixedOrMono(NULL)
  //, m_colorsFilled(RGB_MODE::MODE_UNKNOWN)
{
	//m_state = 0;
 // m_nextColor = RGB_MODE::MODE_UNKNOWN ;
 // m_nextColorByFrameCnt = RGB_MODE::MODE_UNKNOWN ;
 // m_ViewState = MODE_RGB ;
 // m_dNominalPeriod_us = 25000.;
 // m_iOmitedCnt = 0;

 // m_iPreviousColorID = 0;
 // m_dLastMonchromeCopyTime = 0.;
 // m_dLastTickToImageTime = 0.;
 // m_iNR = m_iNB = m_iNG = 0;
 // m_bDoSnap = false;


	//m_pShiftFrameR = new int[IMG_SIZE_DFLT_WIDTH * IMG_SIZE_DFLT_HEIGHT]();
	//m_pShiftFrameG = new int[IMG_SIZE_DFLT_WIDTH * IMG_SIZE_DFLT_HEIGHT]();
	//m_pShiftFrameB = new int[IMG_SIZE_DFLT_WIDTH * IMG_SIZE_DFLT_HEIGHT]();
 // memset(m_pShiftFrameR, 0, sizeof(int) * IMG_SIZE_DFLT_WIDTH * IMG_SIZE_DFLT_HEIGHT);
 // memset(m_pShiftFrameG, 0, sizeof(int) * IMG_SIZE_DFLT_WIDTH * IMG_SIZE_DFLT_HEIGHT);
 // memset(m_pShiftFrameB, 0, sizeof(int) * IMG_SIZE_DFLT_WIDTH * IMG_SIZE_DFLT_HEIGHT);

	init();

  m_pInput->SetQueueSize( 10 ) ;
}
ImageCorrectorGadget::~ImageCorrectorGadget()
{
	if (m_pVFOutput_MixedOrMono)
		m_pVFOutput_MixedOrMono->Release();
}

bool ImageCorrectorGadget::SendVideoFramesHandler(UserBaseGadget& source, RGB_STATE eColor, CVideoFrame * pVF_MonochromOrRGB, bool isSnapshot)
{
  return ((ImageCorrectorGadget&)source).SendOut(pVF_MonochromOrRGB, eColor, isSnapshot);
}

// Mandatory methods
void ImageCorrectorGadget::PropertiesRegistration()
{
	//addProperty(SProperty::EDITBOX, _T("Width"), &m_lWidth, SProperty::Long);
	//addProperty(SProperty::EDITBOX, _T("Height"), &m_lHeight, SProperty::Long);
//
 // addProperty(SProperty::EDITBOX, _T("Timing"), &m_TimingAsString, SProperty::String);
};
void ImageCorrectorGadget::ConnectorsRegistration()
{
	addOutputConnector(transparent, "CorrectedImage");
	addOutputConnector(transparent, "LabeledImage");
  
	addInputConnector(transparent, "Input");
	addDuplexConnector(transparent, transparent, "Init");
};
CDataFrame* ImageCorrectorGadget::DoProcessing(const CDataFrame* pDataFrame)
{
  if ((pDataFrame) && (!Tvdb400_IsEOS(pDataFrame)))
  {
    //YS_20190502 !IMPORTANT!
    //To receive the reqired type of the Frame use the l-side variable with 'const'
    //and DO NOT cast the result of Get____Frame() function!
    const CDataFrame* pRGBFrame = pDataFrame->GetDataFrame(CONTAINER_LBL);
    if (pRGBFrame == NULL || !pRGBFrame->IsContainer())
      return NULL;

    //YS_20190502 !IMPORTANT!
    //If the recived frame should be changed during the gadget operation -- use COPY 
    //of the received frame and DON'T do anything to the received Frame,
    //otherwise if the received Frame will be used in the gadget WITHOUT
    //changes and WILL be SEND OUT -- just ADD REFERENCE,
    //otherwise if the received Frame will be used in the gadget WITHOUT
    //changes and will NOT be send out -- DON'T copy and DON'T add reference.
    //((CContainerFrame*)pRGBFrame)->AddRef();
    m_mixerRGB.AddContainerAndMixRGB((const CContainerFrame*)(pRGBFrame->Copy()), &ImageCorrectorGadget::SendVideoFramesHandler, (UserBaseGadget&)*this);

    //  const CVideoFrame* pInputPictureData = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
    //  if (pInputPictureData == NULL)
    //    return NULL;
  //
    //  FXString Label = pInputPictureData->GetLabel();
    //  uint64_t ui64FrameID , ui64FrameTime_ns ;
    //  bool bTimingIsOK = false ;
    //  int iColorID = -1;
    //  //bool bDecodeOK = GetFrameNumberAndTime( Label , ui64FrameID , ui64FrameTime ) ;
    //  bool bDecodeOK = GetFrameNumberAndTime( pInputPictureData , ui64FrameID , ui64FrameTime_ns , iColorID) ;
    //  double dError = 0. ;
    //  RGB_MODE ColorByTime = RGB_MODE::MODE_UNKNOWN ;
  //
    //  if ( bDecodeOK )
    //  {
    //    if (iColorID != 0)
    //    {
    //      if(m_iPreviousColorID == 0)//first frame in snapshots
    //        m_iNR = m_iNG = m_iNB = 0;
  //
    //      if (iColorID == RGB_MODE::MODE_R)
    //      {
    //        ColorByTime = RGB_MODE::MODE_R;
    //        m_iNR++;
    //      }
    //      else if(iColorID == RGB_MODE::MODE_G)
    //      {
    //        ColorByTime = RGB_MODE::MODE_G;
    //        m_iNG++;
    //      }
    //      else if (iColorID == RGB_MODE::MODE_B)
    //      {
    //        ColorByTime = RGB_MODE::MODE_B;
    //        m_iNB++;
    //      }
    //      bTimingIsOK = true;
    //    }
  //
  //
    //    m_iPreviousColorID = iColorID;
    //  }
    //  double dStart = m_dLastImageReceiveTime = GetHRTickCount();
    //  double dDelay = dStart - m_dLastTickReceiveTime ;
   //
     // int iNewWidth = GetWidth(pInputPictureData);
      //int iNewHeight = GetHeight(pInputPictureData);
      //
    //  m_pVideoFrameRgbOutput = GetVideoFrame_RGB(iNewHeight, iNewWidth);
      //
      //COutputConnector * pOutConnector_Snapshot = GetOutputConnector(1); // output connector for the SNAPSHOT    
      //
      //CVideoFrame * pVideoFrameMonochromOutput = (pOutConnector_Snapshot && pOutConnector_Snapshot->IsConnected() && ((m_ViewState != MODE_RGB) || m_bDoSnap) ) ?
    //    (CVideoFrame*) (pInputPictureData->Copy()) : NULL ;
      //
    //  FXString Addition;
    //  Addition.Format(_T("%s_%06d_%c_%d_%d_%d_"),
    //    (ColorByTime != RGB_MODE::MODE_UNKNOWN) ? _T("Sync") : _T("NoSync"),
    //    (int)dError, ColorByTime , m_iNR , m_iNG , m_iNB );
    //  Label = Addition + Label;
      //
    //  bool isRGBfilled = false;
    //  
    //  if (m_ViewState != MODE_RGB) //Is in MONOCHROMATIC mode only;
    //  {
    //    return pVideoFrameMonochromOutput;
    //  }
    //  else 
    //  {
    //    if (pVideoFrameMonochromOutput) //In SNAPSHOT mode;
    //    {
    //      bool bDefined = true;
    //      switch (ColorByTime)
    //      {
    //      case RGB_MODE::MODE_R: pVideoFrameMonochromOutput->SetLabel("Red"); break;
    //      case RGB_MODE::MODE_G: pVideoFrameMonochromOutput->SetLabel("Green"); break;
    //      case RGB_MODE::MODE_B: pVideoFrameMonochromOutput->SetLabel("Blue"); break;
    //      default: bDefined = false;
    //      }
      //
    //      // In the SNAPSHOT mode each channel
    //      // (MONOCHROMATIC) frame should be stored too;
    //      if (bDefined && m_bDoSnap)
    //        PutFrame(pOutConnector_Snapshot, pVideoFrameMonochromOutput);
    //      else
    //      {
    //        pVideoFrameMonochromOutput->Release();
    //        pVideoFrameMonochromOutput = NULL;
    //      }
    //    }
    //    
    //    // In full (RGB) color mode;
    //    switch (ColorByTime)
    //    {
    //    case RGB_MODE::MODE_R: FillData_Red(pInputPictureData); m_colorsFilled = (RGB_MODE)(m_colorsFilled | RGB_MODE::MODE_R); break;
    //    case RGB_MODE::MODE_G: FillData_Green(pInputPictureData); m_colorsFilled = (RGB_MODE)(m_colorsFilled | RGB_MODE::MODE_G); break;
    //    case RGB_MODE::MODE_B: FillData_Blue(pInputPictureData); m_colorsFilled = (RGB_MODE)(m_colorsFilled | RGB_MODE::MODE_B); break;
    //    }
      //
    //    if ( m_iNR && m_iNG && m_iNB && /*m_pOutput->IsConnected()
    //      &&ys_20190409 (ColorByTime == B)*/ m_colorsFilled == RGB_MODE::MODE_RGB)
    //    {
    //      isRGBfilled = true;
  //
    //      /*ys_20190409
    //      CVideoFrame * pRGBCopy = (CVideoFrame*) m_pvideo->Copy();
    //      pRGBCopy->ChangeId(pInputPictureData->GetId());
    //      pRGBCopy->SetLabel(Label);
    //      PutFrame(m_pOutput, pRGBCopy);
    //      */
  //
    //    }
    //  }
      //double dProcessEnd = GetHRTickCount();
      //m_dLastImageProcTime = dProcessEnd - dStart;
    //  m_dLastTickToImageTime = m_dLastImageReceiveTime - m_dLastTickReceiveTime;
    // 
    //  CVideoFrame* pvf = !isRGBfilled ? NULL : (CVideoFrame*) m_pVideoFrameRgbOutput->Copy();
  //
    //  if (pvf)
    //  {
    //    pvf->SetTime(pInputPictureData->GetTime());
    //    pvf->ChangeId(pInputPictureData->GetId());
    //    pvf->SetLabel(Label);
    //    m_dLastCopyTime = GetHRTickCount() - dProcessEnd;
  //
    //    LockProperties();
    //    m_TimingAsString.Format("Tmc=%.2f Tp=%.2f Tc=%.2f Tti=%.2f",
    //      m_dLastMonchromeCopyTime,
    //      m_dLastImageProcTime,
    //      m_dLastCopyTime,
    //      m_dLastTickToImageTime);
    //    UnlockProperties();
  //
    //    if (m_pOutput && m_pOutput->IsConnected())
    //      PutFrame(m_pOutput, pvf);
    //  }
  //
    //  if (m_iNB == 0 || m_iNG == 0 || m_iNR == 0 || !m_bDoSnap )
    //  {
    //    return NULL;
    //  }
    //  
    //  /*ys_20190409
    //  CVideoFrame* pvf = (CVideoFrame*) m_pVideoFrameRgbOutput->Copy();
    //  */
  //
    //  if (pvf)
    //  {
    //    m_iNR = m_iNG = m_iNB = 0;
    //    /*ys_20190409
    //    pvf->SetTime(pInputPictureData->GetTime());
    //    pvf->ChangeId(pInputPictureData->GetId());
    //    pvf->SetLabel(Label);
    //    m_dLastCopyTime = GetHRTickCount() - dProcessEnd;
  //
    //    LockProperties();
    //    m_TimingAsString.Format("Tmc=%.2f Tp=%.2f Tc=%.2f Tti=%.2f",
    //      m_dLastMonchromeCopyTime,
    //      m_dLastImageProcTime, m_dLastCopyTime, m_dLastTickToImageTime);
    //    UnlockProperties();
    //    */
    //    if (m_iPreviousColorID != 0)//snaphot mode
    //    {
    //      if (pOutConnector_Snapshot && pOutConnector_Snapshot->IsConnected())
    //      {
    //        CVideoFrame * pRGBFrame = (CVideoFrame*)(pvf->Copy()) ;
    //       // pvf->AddRef();
    //        PutFrame(pOutConnector_Snapshot, pRGBFrame);
  //
    //        CTextFrame * pDone = CreateTextFrame("Done","Done");
    //        PutFrame(pOutConnector_Snapshot, pDone);
    //      }
    //      m_bDoSnap = false;
    //    }
   //
    //    /*ys_20190409 is unnecessary
      //  return pvf;
    //    */
      //}
    //  //if (pInputPictureData)
    //  //  ((CVideoFrame*)pInputPictureData)->Release();
  }
  return NULL;
}

void ImageCorrectorGadget::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
  if (!pParamFrame)
    return;

  CTextFrame * pParamsTF = pParamFrame->GetTextFrame(DEFAULT_LABEL);
  if (pParamsTF)
  {

    //if ( !SetViewState(*pParamFrame)
    //  && !SetTimingsForRGBorViewStateForMonochrom(*pParamFrame))
    //{
    //  if (pParamsTF->GetString().Find("Width_Height") > -1)
    //  {
    //    FXSIZE iTok = 0;
    //    FXString dimensions = pParamsTF->GetString().Tokenize(_T(":"), iTok);
    //    if (iTok)
    //    {
    //      FXString width = pParamsTF->GetString().Tokenize(_T(":"), iTok);
    //      if (iTok)
    //      {
    //        FXString height = pParamsTF->GetString().Tokenize(_T(":"), iTok);
    //        m_lWidth = atol(width);
    //        m_lHeight = atol(height);
    //        m_iNewSize = m_lWidth * m_lHeight;
  //
    //        GetVideoFrame_RGB(m_lHeight, m_lWidth);
    //      }
    //    }
    //  }
    //  else if (pParamsTF->GetString().Find("ResetLuts") > -1)
    //  {
    //    memset(m_pShiftFrameR, 0, sizeof(int) * IMG_SIZE_DFLT_WIDTH * IMG_SIZE_DFLT_HEIGHT);
    //    memset(m_pShiftFrameG, 0, sizeof(int) * IMG_SIZE_DFLT_WIDTH * IMG_SIZE_DFLT_HEIGHT);
    //    memset(m_pShiftFrameB, 0, sizeof(int) * IMG_SIZE_DFLT_WIDTH * IMG_SIZE_DFLT_HEIGHT);
    //  }
    //  else if (_tcsstr(pParamsTF->GetLabel(), "StatusSnapshot")
    //    && (pParamsTF->GetString().Find("IsStarted=true") == 0))
    //    m_bDoSnap = true;
    //}
  }
  pParamFrame->Release();
};


//// Private methods
//CVideoFrame* ImageCorrectorGadget::GetVideoFrame_RGB(int iNewHeight, int iNewWidth)
//{
//  if (iNewHeight != m_lHeight
//    || iNewWidth != m_lWidth)
//    SENDERR("Actual frame dimentions (%dx%d) are NOT equal to the predefined in the Gadget properties (%dx%d)."
//      , iNewWidth
//      , iNewHeight
//      , m_lWidth
//      , m_lHeight);
//
//  FXAutolock  al(m_Lock);
//  if (m_pVideoFrameRgbOutput)
//  {
//    if (iNewHeight != GetHeight(m_pVideoFrameRgbOutput)
//      || iNewWidth != GetWidth(m_pVideoFrameRgbOutput))
//    {
//      m_pVideoFrameRgbOutput->Release();
//      m_pVideoFrameRgbOutput = NULL;
//    };
//  }
//
//  if (!m_pVideoFrameRgbOutput)
//  { // New frame in RGB coding
//    m_pVideoFrameRgbOutput = CreateNewVideoFrame(iNewWidth, iNewHeight);
//  }
//
//  ASSERT(m_pVideoFrameRgbOutput);
//
//  return m_pVideoFrameRgbOutput;
//}
//
//bool ImageCorrectorGadget::SetTimingsForRGBorViewStateForMonochrom(const CDataFrame & dataFrame)
//{
//  bool res = false;
//
//  const CTextFrame * pCommand = dataFrame.GetTextFrame(_T("DKandTicks"));
//  if (pCommand)
//  {
//    m_iNR = m_iNG = m_iNB = 0;
//    FXPropertyKit Command(pCommand->GetString());
//    int iDK, iEncoderLength = 3000, iRTick = 0, iGTick = 0, iBTick = 0, iSTick = 0;
//    if (!Command.GetInt(_T("DK"), iDK) || iDK <= 0)
//      SENDERR("Error! Unknown Tick rate");
//    else
//    {
//      Command.GetInt(_T("EL"), iEncoderLength);
//      double dTickPeriod_us = 1.0e6 / (double)iDK;
//      double dEncoderPeriod_us = dTickPeriod_us * iEncoderLength;
//      int iNColors = Command.GetInt(_T("R"), iRTick);
//      iNColors += Command.GetInt(_T("G"), iGTick);
//      iNColors += Command.GetInt(_T("B"), iBTick);
//      if (iNColors == 0) iNColors = Command.GetInt(_T("S"), iSTick);
//      switch (iNColors)
//      {
//      case 3:/*
//        CalcTimingForRGB(iRTick, iGTick, iBTick,
//          dTickPeriod_us, dEncoderPeriod_us);*/
//        res = true;
//        break;
//      case 2:
//      {
//        TCHAR Color = (iRTick) ? _T('R') : _T('G');
//        SENDERR("Can't work with 2 colors; taking %c", Color);
//      }
//      case 1:
//        m_ViewState = (iRTick) | (iSTick) ? MODE_R : (iGTick) ? MODE_G : MODE_B;
//        res = m_ViewState != RGB_MODE::MODE_UNKNOWN;
//        break;
//      case 0:
//      default:
//        SENDERR("Error! Unknown color decoding (%d ticks)", iNColors);
//        break;
//      }
//    }
//  }
//
//  return res;
//}
//
//bool ImageCorrectorGadget::SetViewState(const CDataFrame &dataFrame)
//{
//  bool res = false;
//
//  const CTextFrame * pCommand = dataFrame.GetTextFrame(_T("ColourConfig"));
//
//  if (pCommand)
//  {
//    m_ViewState = RGB_MODE::MODE_UNKNOWN;
//
//    FXString Command = pCommand->GetString();
//    if (Command.Find(_T("ViewRGBS")) == 0 || Command.Find(_T("ViewRGB")) == 0)
//    {
//      m_ViewState = MODE_RGB;
//      m_iNR = m_iNG = m_iNB = 0;
//    }
//    else if (Command.Compare(_T("ViewR")) == 0)
//      m_ViewState = MODE_R;
//    else if (Command.Compare(_T("ViewG")) == 0)
//      m_ViewState = MODE_G;
//    else if (Command.Compare(_T("ViewB")) == 0)
//      m_ViewState = MODE_B;
//
//    res = m_ViewState != RGB_MODE::MODE_UNKNOWN;
//  }
//
//  return res;
//}
//
//CVideoFrame* ImageCorrectorGadget::CreateNewVideoFrame(DWORD dwWidth, DWORD dwHeight)
//{
//	CVideoFrame * pvf = CVideoFrame::Create();
//	DWORD dwImageSize = dwHeight * dwWidth * 3;
//	pvf->lpBMIH = (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER) + dwImageSize);
//	pvf->lpData = NULL;
//	pvf->lpBMIH->biSize = sizeof(BITMAPINFOHEADER);
//	pvf->lpBMIH->biWidth = dwWidth;
//	pvf->lpBMIH->biHeight = dwHeight;
//	pvf->lpBMIH->biSizeImage = dwImageSize;
//	pvf->lpBMIH->biPlanes = 1;
//	pvf->lpBMIH->biBitCount = 24;
//	pvf->lpBMIH->biCompression = BI_RGB;
//	pvf->lpBMIH->biXPelsPerMeter = 0;
//	pvf->lpBMIH->biYPelsPerMeter = 0;
//	pvf->lpBMIH->biClrUsed = 0;
//	pvf->lpBMIH->biClrImportant = 0;
//	return pvf;
//}
//
//bool ImageCorrectorGadget::FillColor(const CVideoFrame* pInputPictureData, int iColorShift, int * pShiftMatrix)
//{
//  bool res = false;
//
//  int iSourceRows = pInputPictureData->lpBMIH->biHeight;
//  int iSourceColumns = pInputPictureData->lpBMIH->biWidth;
//  LPBYTE pData = (LPBYTE)&pInputPictureData->lpBMIH[1];
//  LPBYTE pDstData = GetData(m_pVideoFrameRgbOutput);
//  int j = iSourceRows - 2;
// 
//  for (; j > -1; j--)//take prelast row
//  {
//    LPBYTE pSrc = pData + j * iSourceColumns;
//    LPBYTE pEndSrc = pSrc + iSourceColumns;
//    LPBYTE pDst = pDstData + (iSourceRows - 2 - j) * 3 * iSourceColumns + iColorShift - 3;
//    while (pSrc < pEndSrc)
//    {
//      //*((pDst += 3) + *(pShiftMatrix++)) = *(pSrc++);
//      int shift = *(pShiftMatrix++);
//      *(pDst += 3) = *(pSrc++);
//    }
//  }
//
//  res = j == -1;
//
//  return res;
//}
//bool ImageCorrectorGadget::FillData_Red(const CVideoFrame* pInputPictureData)
//{
//	return FillColor(pInputPictureData, 2, m_pShiftFrameR);
//}
//bool ImageCorrectorGadget::FillData_Green(const CVideoFrame* pInputPictureData)
//{
//	return FillColor(pInputPictureData, 1, m_pShiftFrameG );
//}
//bool ImageCorrectorGadget::FillData_Blue(const CVideoFrame* pInputPictureData)
//{
//	return FillColor(pInputPictureData, 0, m_pShiftFrameB );
//}
