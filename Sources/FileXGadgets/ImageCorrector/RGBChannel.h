#pragma once
#include <stdint.h>
#include "helpers\UserBaseGadget.h"
#include <imageproc\VFrameEmbedInfo.h>
#include <helpers\FramesHelper.h>
#include "RGBContainerFrame/RGBHelper.h"

typedef bool(*LP_SENDER_DELEGATE)(UserBaseGadget& source, RGB_STATE eColor, CVideoFrame* pVF_MonochromOrRGB, bool isSnapshot);

struct RGBChannelBase
{
private:
  CVideoFrame**       m_ppDstVideoFrame;
  LPBYTE*             m_ppDstData;
  bool&               m_bIsDstRenewRequested;
  SIZE&               m_szDstDimen_px;

  int                 m_nColorShift;
  RGB_STATE           m_eColor;
  const CVideoFrame*  m_pVideoFrameSrc = NULL;


protected:
  RGB_STATE&           m_eDstColorsReady;
  

  __forceinline CVideoFrame * CreateRGBVideoFrame(long lHieght, long lWidth)
  {
    SIZE sz;
    sz.cx = lWidth;
    sz.cy = lHieght;
    return CreateRGBVideoFrame(sz);
  }
  __forceinline CVideoFrame * CreateRGBVideoFrame(const SIZE& szDimens)
  {
    CVideoFrame * pvf = CVideoFrame::Create();
    DWORD dwImageSize = szDimens.cy * szDimens.cx * 3;

    pvf->lpData = NULL;
    pvf->lpBMIH = (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER) + dwImageSize);
    pvf->lpBMIH->biSize = sizeof(BITMAPINFOHEADER);
    pvf->lpBMIH->biWidth = szDimens.cx;
    pvf->lpBMIH->biHeight = szDimens.cy;
    pvf->lpBMIH->biSizeImage = dwImageSize;
    pvf->lpBMIH->biPlanes = 1;
    pvf->lpBMIH->biBitCount = 24;
    pvf->lpBMIH->biCompression = BI_RGB;
    pvf->lpBMIH->biXPelsPerMeter = 0;
    pvf->lpBMIH->biYPelsPerMeter = 0;
    pvf->lpBMIH->biClrUsed = 0;
    pvf->lpBMIH->biClrImportant = 0;
    return pvf;
  }
  __forceinline virtual RGBChannelBase& SetSrcVideoFrame(const CVideoFrame* pVFSrc)
  {
    if (pVFSrc)
    {
      ResetSrcVideoFrame();

      long lSrcHeght = pVFSrc->lpBMIH->biHeight;
      long lSrcWidth = pVFSrc->lpBMIH->biWidth;

      if (m_ppDstVideoFrame && m_ppDstData)
      {
        if (*m_ppDstVideoFrame)
        {
          if (fabs(lSrcHeght - m_szDstDimen_px.cy) != 0
            || fabs(lSrcWidth - m_szDstDimen_px.cx) != 0
            || m_bIsDstRenewRequested)
          {
            *m_ppDstData = NULL;
            (*m_ppDstVideoFrame)->Release();
            (*m_ppDstVideoFrame) = NULL;
            m_szDstDimen_px.cy = 0;
            m_szDstDimen_px.cx = 0;
            m_eDstColorsReady = RGB_STATE::STATE_UNKNOWN;
          }
        }

        if (!*m_ppDstVideoFrame)
        {
          m_bIsDstRenewRequested = false;
          m_szDstDimen_px.cx = lSrcWidth;
          m_szDstDimen_px.cy = lSrcHeght;
          (*m_ppDstVideoFrame) = CreateRGBVideoFrame(m_szDstDimen_px);
          (*m_ppDstVideoFrame)->SetLabel("RGB");
        }

        ASSERT(m_ppDstVideoFrame && *m_ppDstVideoFrame);

        if (!*m_ppDstData)
          (*m_ppDstData) = GetData(*m_ppDstVideoFrame);

        m_pVideoFrameSrc = pVFSrc;
      }
    }

    return *this;
  }

  double coeff = 0;
  __forceinline bool FillColor()
  {
    bool res = false;

    if (m_pVideoFrameSrc && m_ppDstData && *m_ppDstData)
    {
      int iSourceRows = m_szDstDimen_px.cy;
      int iSourceColumns = m_szDstDimen_px.cx;
      const LPBYTE pSrcData = GetData(m_pVideoFrameSrc);

      int iCamImgRowCurrent = iSourceRows - 2;

      for (; iCamImgRowCurrent >= 0; iCamImgRowCurrent--)//start from the pre-last row
      {
        LPBYTE pSrcStart = pSrcData + iCamImgRowCurrent * iSourceColumns;
        LPBYTE pSrcEnd = pSrcStart + iSourceColumns;

        LPBYTE pDst = (*m_ppDstData) + (iSourceRows - 2 - iCamImgRowCurrent) * 3 * iSourceColumns + m_nColorShift - 3;

        while (pSrcStart < pSrcEnd)
        {
          *(pDst += 3) = *(pSrcStart++);
        }
      }

      res = iCamImgRowCurrent == -1;
    }
    return res;
  }

  __forceinline const RGB_STATE& AddDstReadyColor(RGB_STATE eDstReadyColor)
  {
    int nReadyColors = m_eDstColorsReady;
    nReadyColors |= eDstReadyColor;

    m_eDstColorsReady = (RGB_STATE)nReadyColors;

    return m_eDstColorsReady;
  }
  __forceinline const RGB_STATE& AddDstReadyColor()
  {
    return AddDstReadyColor(m_eColor);
  }

public:
  RGBChannelBase(RGB_STATE eColor, int nColorShift
    , __out CVideoFrame** ppDstVideoFrame
    , __out LPBYTE* ppDstData
    , __out bool& bIsDstRenewRequested
    , __out SIZE& szDstDimen
    , __out RGB_STATE& eDstColorsReady
  )
    : m_eColor(eColor)
    , m_nColorShift(nColorShift)
    , m_pVideoFrameSrc(NULL)
    , m_ppDstVideoFrame(ppDstVideoFrame)
    , m_ppDstData(ppDstData)
    , m_bIsDstRenewRequested(bIsDstRenewRequested)
    , m_szDstDimen_px(szDstDimen)
    , m_eDstColorsReady(eDstColorsReady)
  {

  }

  ~RGBChannelBase()
  {
    m_pVideoFrameSrc = NULL;
  }

  __forceinline RGB_STATE GetColor() const
  {
    return m_eColor;
  }

  __forceinline RGBChannelBase& ResetSrcVideoFrame(bool bToRelease = true)
  {
    if (m_pVideoFrameSrc && m_pVideoFrameSrc->GetUserCnt() > 0)
    {
      if (bToRelease)
        ((CDataFrame*) m_pVideoFrameSrc)->Release();

      m_pVideoFrameSrc = NULL;
    }
    return *this;
  }

  __forceinline bool AddColorAndSend(const CVideoFrame* pVFSrc, RGB_STATE eExpectedColors, bool bIsSnapshot, LP_SENDER_DELEGATE pSenderDelegate, UserBaseGadget& source, bool bToSendRGB)
  {
    bool bRes = false;
   

    if (SetSrcVideoFrame(pVFSrc).FillColor() && AddDstReadyColor() == eExpectedColors)
    {
      bRes = !bToSendRGB ? true : (pSenderDelegate && pSenderDelegate(source, eExpectedColors,(CVideoFrame*)(*m_ppDstVideoFrame)->Copy(), bIsSnapshot));
    }
   
    //ResetSrcVideoFrame();

    return bRes;
  }
};


struct RGBChannel_R
  : public RGBChannelBase
{
public:
  RGBChannel_R(
    __out CVideoFrame** ppDstVideoFrame
    , __out LPBYTE* ppDstData
    , __out bool& bIsDstRenewRequested
    , __out SIZE& szDstDimen
    , __out RGB_STATE& eDstColorsReady
  )
    : RGBChannelBase(RGB_STATE::STATE_R, 2
      , ppDstVideoFrame
      , ppDstData
      , bIsDstRenewRequested
      , szDstDimen
      , eDstColorsReady
    )
  {}
};
struct RGBChannel_G
  : public RGBChannelBase
{
public:
  RGBChannel_G(
    __out CVideoFrame** ppDstVideoFrame
    , __out LPBYTE* ppDstData
    , __out bool& bIsDstRenewRequested
    , __out SIZE& szDstDimen
    , __out RGB_STATE& eDstColorsReady
  )
    : RGBChannelBase(RGB_STATE::STATE_G, 1
      , ppDstVideoFrame
      , ppDstData
      , bIsDstRenewRequested
      , szDstDimen
      , eDstColorsReady
    )
  {}
};
struct RGBChannel_B
  : public RGBChannelBase
{
public:
  RGBChannel_B(
    __out CVideoFrame** ppDstVideoFrame
    , __out LPBYTE* ppDstData
    , __out bool& bIsDstRenewRequested
    , __out SIZE& szDstDimen
    , __out RGB_STATE& eDstColorsReady
  )
    : RGBChannelBase(RGB_STATE::STATE_B, 0
      , ppDstVideoFrame
      , ppDstData
      , bIsDstRenewRequested
      , szDstDimen
      , eDstColorsReady
    )
  {}
};