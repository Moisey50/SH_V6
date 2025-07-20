#pragma once
#include "RGBContainerFrame\RGB2ContainerFrame2RGBDeSerializer.h"
#include "RGBChannel.h"
#include <map>

using namespace std;

struct RGBMixer
{
private:
  bool                                m_bToSendRGBPerSeparation;
  UINT64                              m_ui64PrevEndTimestamp_ns;
  FXString                            m_sOrderAsTxt;
  bool                                m_bIsSnapshot;
  RGB_STATE                           m_eExpectedRGBState;
                                      
  CVideoFrame*                        m_pDstVideoFrame;
  LPBYTE                              m_pDstData;
  bool                                m_bIsDstRenewRequested;
  SIZE                                m_szDstDimen_px;
  RGB_STATE                           m_eDstColorsReady;
  map<RGB_STATE, const CVideoFrame*>  m_order;
                                  
  RGBChannel_R                        m_R;
  RGBChannel_G                        m_G;
  RGBChannel_B                        m_B;
  map<RGB_STATE, RGBChannelBase*>     m_channels;


  RGBMixer(const RGBMixer&);
  RGBMixer& operator= (const RGBMixer&);

  bool IsInRGBState(RGB_STATE eColors) const
  {
    return (m_eExpectedRGBState & eColors) == eColors;
  }
  bool IsSameOrder(const FXString& newOrder) const
  {
    return m_sOrderAsTxt.Compare(newOrder) == 0;
  }
  bool IsTimestampContinues(UINT64 newStartTime_ns)
  {
    bool bRes = newStartTime_ns - m_ui64PrevEndTimestamp_ns == 0;
    return bRes;
  }

  RGBMixer& SetIsSnapshot(bool bNewIsSnapshot)
  {
    if (!m_bToSendRGBPerSeparation || m_bIsSnapshot != bNewIsSnapshot)
       m_bIsDstRenewRequested = true;
    m_bIsSnapshot = bNewIsSnapshot;
    return *this;
  }
  RGBMixer& SetRGBState(RGB_STATE eNewColors)
  {
    if (!m_bToSendRGBPerSeparation || m_eExpectedRGBState != eNewColors)
      m_bIsDstRenewRequested = true;
    m_eExpectedRGBState = eNewColors;
    return *this;
  }
  RGBMixer& SetOrderAsTxt(const FXString & newOrderAsTxt)
  {
    if (!m_bToSendRGBPerSeparation || !IsSameOrder(newOrderAsTxt))
      m_bIsDstRenewRequested = true;
    m_sOrderAsTxt= newOrderAsTxt;
    return *this;
  }
  RGBMixer& SetEndTimestamp(UINT64 ui64TimestampStart_ns, UINT64 ui64TimestampEnd_ns)
  {
    if (!m_bToSendRGBPerSeparation || !IsTimestampContinues(ui64TimestampStart_ns))
      m_bIsDstRenewRequested = true;
    m_ui64PrevEndTimestamp_ns = ui64TimestampEnd_ns;
    return *this;
  }

  bool AddColorAndSend(RGB_STATE eColor, const CVideoFrame* pVFSrc, LP_SENDER_DELEGATE pSenderDelegate, UserBaseGadget& source, bool bToSendRGB)
  {
    bool bRes = false;

    if (m_bIsSnapshot
      || m_eExpectedRGBState == RGB_STATE::STATE_R
      || m_eExpectedRGBState == RGB_STATE::STATE_G
      || m_eExpectedRGBState == RGB_STATE::STATE_B)
    {
      if (pSenderDelegate)
      {
        //CVideoFrame* pVFSrcCopy = (CVideoFrame*)pVFSrc->Copy();

        //YS_2019 !IMPORTANT!
        //The reference should be added HERE in the case
        //of the SNAPSHOT mode only, and if the Monocromatic frame will be 
        //sended BEFORE the mixing of colors, otherwise
        //adding of the reference should be perfomed just BEFORE mixing only!
        if(m_bIsSnapshot)
          ((CVideoFrame*)pVFSrc)->AddRef();
        pSenderDelegate(source, eColor, (CVideoFrame*)pVFSrc, m_bIsSnapshot);
      }
    }

    if (m_eExpectedRGBState > RGB_STATE::STATE_UNKNOWN
      && m_eExpectedRGBState <= RGB_STATE::STATE_RGB
      && m_eExpectedRGBState != RGB_STATE::STATE_R
      && m_eExpectedRGBState != RGB_STATE::STATE_G
      && m_eExpectedRGBState != RGB_STATE::STATE_B)
    {
      RGBChannelBase * pChannel = m_channels[eColor];

      if (pChannel)
        bRes = pChannel->AddColorAndSend(pVFSrc, m_eExpectedRGBState, m_bIsSnapshot, pSenderDelegate, source, bToSendRGB);
    }

    return bRes;
  }


public:
  RGBMixer()
    : m_bToSendRGBPerSeparation()
    , m_ui64PrevEndTimestamp_ns(0)
    , m_sOrderAsTxt()
    , m_bIsSnapshot (false)
    , m_eExpectedRGBState( RGB_STATE::STATE_UNKNOWN)
    , m_pDstVideoFrame(NULL)
    , m_pDstData(NULL)
    , m_bIsDstRenewRequested(false)
    , m_szDstDimen_px()
    , m_eDstColorsReady(RGB_STATE::STATE_UNKNOWN)
    , m_R(&m_pDstVideoFrame, &m_pDstData, m_bIsDstRenewRequested, m_szDstDimen_px, m_eDstColorsReady)
    , m_G(&m_pDstVideoFrame, &m_pDstData, m_bIsDstRenewRequested, m_szDstDimen_px, m_eDstColorsReady)
    , m_B(&m_pDstVideoFrame, &m_pDstData, m_bIsDstRenewRequested, m_szDstDimen_px, m_eDstColorsReady)
  {
    m_channels =
    {
      {m_R.GetColor(), &m_R},
      {m_G.GetColor(), &m_G},
      {m_B.GetColor(), &m_B}
    };
  }  
  __forceinline ~RGBMixer()
  {
    m_pDstData = NULL;
    if(m_pDstVideoFrame)
      m_pDstVideoFrame->Release();
    m_pDstVideoFrame = NULL;
    m_szDstDimen_px.cy = 0;
    m_szDstDimen_px.cx = 0;
    m_eDstColorsReady = RGB_STATE::STATE_UNKNOWN;

    m_sOrderAsTxt.Empty();
    m_ui64PrevEndTimestamp_ns = 0;
    m_bIsSnapshot = (false);
    m_eExpectedRGBState = (RGB_STATE::STATE_UNKNOWN);
  }

  __forceinline RGBMixer& SetToSendRGBPerSeparation(bool bToSendRGBPerSeparation)
  {
    if (m_bToSendRGBPerSeparation != bToSendRGBPerSeparation)
      m_bIsDstRenewRequested = true;
    m_bToSendRGBPerSeparation = bToSendRGBPerSeparation;

    return *this;
  }

  __forceinline void AddContainerAndMixRGB(const CContainerFrame* pCF, LP_SENDER_DELEGATE pSenderHandler, UserBaseGadget& source)
  {
    RGB2ContainerFrame2RGBDeSerializer cntnrFrm2RGBDeserializer;
    if (pCF)
    {      
      if (cntnrFrm2RGBDeserializer.Deserialize(pCF) && cntnrFrm2RGBDeserializer.GetRGBState() > RGB_STATE::STATE_UNKNOWN)
      {
        SetIsSnapshot(cntnrFrm2RGBDeserializer.IsSnapshot())
          .SetOrderAsTxt(cntnrFrm2RGBDeserializer.GetOrderAsTxt())
          .SetRGBState(cntnrFrm2RGBDeserializer.GetRGBState())
          .SetEndTimestamp(cntnrFrm2RGBDeserializer.GetStart_ns(), cntnrFrm2RGBDeserializer.GetEnd_ns());

        m_order = cntnrFrm2RGBDeserializer.GetOrder();

        cntnrFrm2RGBDeserializer.Empty();
        FXSIZE clrsQty = m_sOrderAsTxt.GetLength();
        for (int cr = 0; cr < clrsQty;  ++cr)
        {
          RGB_STATE eColor = (RGB_STATE)m_sOrderAsTxt[cr];
          const CVideoFrame* pVFrame = m_order[eColor];

          if (pVFrame)
            AddColorAndSend(eColor, pVFrame, pSenderHandler, source, m_bToSendRGBPerSeparation || cr+1 == clrsQty);

          if (pVFrame->GetUserCnt() <= 0)
            m_order[eColor] = NULL;
        }

      }
      
      //CVideoFrame *pVf = NULL;
      //CFramesIterator* pIterator = pCF->CreateFramesIterator(vframe);
      //if (pIterator != NULL)
      //{
      //  pVf = (CVideoFrame*)pIterator->Next(DEFAULT_LABEL);
      //  while (pVf)
      //  {
      //    pVf->Release();
      //    
      //    pVf = (CVideoFrame*)pIterator->Next(DEFAULT_LABEL);
      //  } 
      //}


      //((CContainerFrame*)pCF)->Release();
    }
  }
 };