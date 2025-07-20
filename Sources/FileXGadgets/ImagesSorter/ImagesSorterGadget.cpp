// ImagesSorterGadget.h : Implementation of the ImagesSorterGadget class


#include "StdAfx.h"
#include "ImagesSorterGadget.h"

#define THIS_MODULENAME "ImagesSorter"

USER_FILTER_RUNTIME_GADGET(ImagesSorterGadget, LINEAGE_FILEX );	//	Mandatory






ImagesSorterGadget::ImagesSorterGadget()
  : m_sorter()
{
	init();

  m_pInput->SetQueueSize( 100 ) ;
}
ImagesSorterGadget::~ImagesSorterGadget()
{

}


// Mandatory methods
void ImagesSorterGadget::PropertiesRegistration()
{
	//addProperty(SProperty::EDITBOX, _T("Width"), &m_lWidth, SProperty::Long);
	//addProperty(SProperty::EDITBOX, _T("Height"), &m_lHeight, SProperty::Long);

  //addProperty(SProperty::EDITBOX, _T("Timing"), &m_TimingAsString, SProperty::String);
};
void ImagesSorterGadget::ConnectorsRegistration()
{
	addOutputConnector(transparent, "RGBContainer");
  
	addInputConnector(transparent, "Input");
	addDuplexConnector(transparent, transparent, "SetupSorter");
};

CDataFrame* ImagesSorterGadget::DoProcessing(const CDataFrame* pDataFrame)
{
  CDataFrame* pRes = NULL;
  if ((pDataFrame) && (!Tvdb400_IsEOS(pDataFrame)))
  {
    //YS_20190502 !IMPORTANT! 
    //To receive the reqired type of the Frame use the l-side variable with 'const'
    //and DO NOT cast the result of Get____Frame() function!
    const CVideoFrame* pInputVideoData = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
    if (pInputVideoData != NULL)
    {
      m_Lock.Lock( 0xffffffff , "ImagesSorterGadget::DoProcessing" );
      //YS_20190502 !IMPORTANT!
      //If the recived frame should be changed during the gadget operation -- use COPY 
      //of the received frame and DON'T do anything to the received Frame,
      //otherwise if the received Frame will be used in the gadget WITHOUT
      //changes and WILL be SEND OUT -- just ADD REFERENCE,
      //otherwise if the received Frame will be used in the gadget WITHOUT
      //changes and will NOT be send out -- DON'T copy and DON'T add reference.
      bool bRes = m_sorter.GetRGBFrame(*(CVideoFrame*)pInputVideoData->Copy(), &ImagesSorterGadget::SendContainerHandler, (UserBaseGadget&)*this);
      m_Lock.Unlock();

      if (!bRes && m_sorter.GetErrorMsg().GetLength() > 0)
        SENDERR("Error! %s", m_sorter.GetErrorMsg());
    }
  }
  return NULL;
}
void ImagesSorterGadget::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
  if (!pParamFrame)
    return;
  CTextFrame * pParamsTF = NULL;
  CTextFrame * pTFSelectSubProg = pParamFrame->GetTextFrame(SETTING_NAME_VIDEO);
  if (!pTFSelectSubProg)
  {
    pTFSelectSubProg = pParamFrame->GetTextFrame(SETTING_NAME_SNAPSHOT);
    if (!pTFSelectSubProg)
    {
      pParamsTF = pParamFrame->GetTextFrame(DEFAULT_LABEL);
      if (!pParamsTF)
      {
        pParamFrame->Release( pParamFrame );
        return;
      }
    }
  }

  if (pTFSelectSubProg)
  {
    m_Lock.Lock(0xffffffff, "ImagesSorterGadget::AsyncTransaction");
    
    m_sorter.SetSubProgram(pTFSelectSubProg->GetString()[0]);
    m_Lock.Unlock();
  }
  else
  {
    FXString prgrStatus;
    char errCause[400] = { 0 };
    bool res = m_sorter.SetSuperProgram(*pParamsTF, errCause);

    prgrStatus.Format("IsPrgrmLoaded=%s;%s%s%s"
      , res ? "true" : "false"
      , !errCause[0] ? "" : " Cause="
      , !errCause[0] ? "" : errCause
      , !errCause[0] ? "" : ";"
    );

    if (!res)
    {
    }

    PutFrame(pConnector, CreateTextFrame(prgrStatus, "Status_PrgrmLoad"));
    if (errCause[0])
      SENDERR(errCause);
  }
  
  pParamFrame->Release();
}