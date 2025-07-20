#pragma once
#include <stdint.h>
#include <imageproc\VFrameEmbedInfo.h>
#include <helpers\FramesHelper.h>
#include "ColorChannelNode.h"

enum RGB_MODE
{
  MODE_UNKNOWN = 0,
  MODE_R = 1,
  MODE_G = MODE_R << 1,
  MODE_B = MODE_G << 1,
  MODE_RGB = MODE_R | MODE_G | MODE_B
};

/*typedef*/
struct ColorFrame
{
private:
  UINT64                  m_ui64Id = 0;
  UINT64                  m_ui64TimeStamp_ns = 0;
  double                  m_dTimeStamp_us = 0;
  double                  m_dFrameLength_us = 0;
  int                     m_nColorId = -1;
  const ColorChannelNode *m_pColorChannelNode = NULL;

  RGB_MODE                m_eColors = RGB_MODE::MODE_UNKNOWN;
  const CVideoFrame*      m_pFrame = NULL;

  bool GetFrameIdTimestampAndColorID(const CVideoFrame & vf, __out uint64_t& ui64FrameID, __out uint64_t& ui64Timestamp, __out int &iColorID)
  {
    bool res = false;
    PVFEI pEmbedInfo = (PVFEI)GetData(&vf);
    if (pEmbedInfo && pEmbedInfo->m_dwIdentificationPattern == EMBED_INFO_PATTERN)
    {
      ui64FrameID = pEmbedInfo->m_CameraFrameID;
      ui64Timestamp = pEmbedInfo->m_CameraFrameTime;
      iColorID = pEmbedInfo->m_ColorSign;
      res = true;
    }
    return res;
  }

  ColorFrame& SetFrame(const CVideoFrame& frame)
  {
    m_pFrame = NULL;
    uint64_t ui64FrameID = 0;
    uint64_t ui64Timestamp_ns = 0;
    int iColorID = -1;
    if (GetFrameIdTimestampAndColorID(frame, ui64FrameID, ui64Timestamp_ns, iColorID))
    {
      m_pFrame = &frame;
      m_nColorId = iColorID;
      m_ui64Id = ui64FrameID;
      m_ui64TimeStamp_ns = ui64Timestamp_ns;
      m_dTimeStamp_us = (double)(m_ui64TimeStamp_ns / 1000.);
    }

    return *this;
  }


public:
  ColorFrame(const CVideoFrame* pVideoFrame)
  {
    if (pVideoFrame)
      SetFrame(*pVideoFrame); 
  }
  ~ColorFrame()
  {
    m_pColorChannelNode = NULL;

    m_pFrame = NULL;
  }

  ColorFrame& ReleaseVideoFrame()
  {
    if (m_pFrame &&((CDataFrame*) m_pFrame)->Release())
      m_pFrame = NULL;
    
    return *this;
  }

  const CVideoFrame* GetVideoFrame()const
  {
    return m_pFrame;
  }

  bool IsVideoFrameEmpty()const
  {
    return m_pFrame == NULL;
  }

  const ColorChannelNode* GetColorChannelNode() const
  {
    return m_pColorChannelNode;
  }
  ColorFrame& SetColorChannelNode(const ColorChannelNode* pColorChannelNode)
  {
    m_pColorChannelNode = pColorChannelNode;
    if (!m_pColorChannelNode)
    {
      m_eColors = RGB_MODE::MODE_UNKNOWN;
      m_nColorId = UNDEFINED;
    }
    else
    {
      const ColorChannel& cc = m_pColorChannelNode->GetData();
      const string& colorName = cc.GetName();

      switch (colorName[0])
      {
      case 'R':
      case 'r':
        m_eColors = RGB_MODE::MODE_R;
        break;
      case 'G':
      case 'g':
        m_eColors = RGB_MODE::MODE_G;
        break;
      case 'B':
      case 'b':
      case 'Y':
      case 'y':
        m_eColors = RGB_MODE::MODE_B;
        break;
      default:
        if (colorName.compare("IR") == 0
          || colorName.compare("ir") == 0)
          m_eColors = RGB_MODE::MODE_R;
        break;
      }
    }

    return *this;
  }

  uint64_t GetTimestamp_ns() const
  {
    return m_ui64TimeStamp_ns;
  }
  double GetTimestamp_us() const
  {
    return m_dTimeStamp_us;
  }

  double GetFrameLengthActual_us() const
  {
    return m_dFrameLength_us;
  }
  ColorFrame& SetFrameLengthActual_us(const ColorFrame& next)
  {
    m_dFrameLength_us = next.m_dTimeStamp_us - m_dTimeStamp_us;
    return *this;
  }

  RGB_MODE GetColor() const
  {
    return m_eColors;
  }
  bool IsColor(RGB_MODE colorsExpected) const
  {
    return m_eColors & colorsExpected;
  }

};