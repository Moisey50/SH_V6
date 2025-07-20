#pragma once
#include "RGBFrame.h"
#include "Program.h"
#include "helpers\UserBaseGadget.h"


using namespace std;

#define SETTING_NAME_VIDEO    ("StatusVideo")
#define SETTING_NAME_SNAPSHOT ("StatusSnapshot")

typedef bool(*LP_DF_SENDER_DELEGATE)(UserBaseGadget& source, CDataFrame* pDF);

class RGBFramesSorter
{
private:
  FXLockObject           m_Lock; // synchronization object
  ColorChannelNodesList* m_pSubPrgrmCurrent;
  RGBFrame*              m_pRgbCurrent;

  Program                m_superPrgrm;
  bool                   m_bIsProgramLoaded;
  bool                   m_bIsSnapshot;
  FXString               m_err;

  RGBFramesSorter(const RGBFramesSorter&);
  RGBFramesSorter& operator=(const RGBFramesSorter&);

  __forceinline bool SendContainer(RGBFrame** ppFrameRGB, const LP_DF_SENDER_DELEGATE & pSenderDelegate, UserBaseGadget & source)
  {
    bool bRes = false;

    if (ppFrameRGB && *ppFrameRGB)
    {
      if ((*ppFrameRGB)->IsFull())
      {
        CDataFrame * pContainer = (*ppFrameRGB)->GetCContainerFrame();

        if (pSenderDelegate && pContainer)
          bRes = pSenderDelegate(source, pContainer);

        DestroyRGBFrame(ppFrameRGB);
      }
    }
    return bRes;
  }

  __forceinline RGBFramesSorter& DestroyRGBFrame(RGBFrame** ppFrameRGB)
  {
    if (ppFrameRGB && *ppFrameRGB)
    {
      delete (*ppFrameRGB);

      (*ppFrameRGB) = NULL;
    }
    return *this;
  }

public:
  RGBFramesSorter()
    : m_Lock()
    , m_pSubPrgrmCurrent(NULL)
    , m_pRgbCurrent(NULL)
    , m_superPrgrm()
    , m_bIsProgramLoaded(false)
    , m_bIsSnapshot(false)
  {
    //RESET_INSTANCE
  }
  ~RGBFramesSorter()
  {
    m_superPrgrm.Empty();

    m_pSubPrgrmCurrent = (NULL);
    m_pRgbCurrent = (NULL);
    //RESET_INSTANCE
  }

  __forceinline const FXString& GetErrorMsg()
  {
    return m_err;
  }

  __forceinline RGBFramesSorter& SetSubProgram(char subPrgrmId)
  {
    if (m_pRgbCurrent)
    {
      DestroyRGBFrame(&m_pRgbCurrent);
    }

    m_bIsSnapshot = false;
    switch (subPrgrmId)
    {
    case 'V':
    case 'v':
      m_pSubPrgrmCurrent = &m_superPrgrm.m_mdVideo;
      break;
    case 'S':
    case 's':
      m_pSubPrgrmCurrent = &m_superPrgrm.m_mdSnapshot;
      m_bIsSnapshot = true;
      break;
    default:
      m_pSubPrgrmCurrent = NULL;
      break;
    }

    TRACE("Subprogram is %s!!!!\n"
      , !m_pSubPrgrmCurrent ? "EMPTY" : m_pSubPrgrmCurrent->GetType().c_str());
    return *this;
  }
   
  __forceinline bool GetRGBFrame(const CVideoFrame& vFrame, LP_DF_SENDER_DELEGATE pSenderDelegate, UserBaseGadget& source)
  {
    bool bRes = false;

    m_err.Empty();
    char buf[100] = { 0 };
    if (!m_pSubPrgrmCurrent || !m_pSubPrgrmCurrent->IsValid(buf))
    {
      m_err.Format("%s subprogram is missing!%s%s"
        , m_bIsSnapshot ? "Snapshot" : "Video"
        , buf[0] ? "Cause: " : ""
        , buf[0] ? buf : ""
      );
    }
    else
    {
      queue <const ColorFrame*> res;
      if (!m_pRgbCurrent)
        m_pRgbCurrent = new RGBFrame(*m_pSubPrgrmCurrent, m_bIsSnapshot);

      try
      {
        m_pRgbCurrent->Add(vFrame, res);

        if (res.empty())
        {
          bRes = SendContainer(&m_pRgbCurrent, pSenderDelegate, source);
        }
        else
        {
          ColorFrame* pColorFrameCandidate = NULL;
          ColorFrame* pColorFrameNew = NULL;
          while (!res.empty())
          {
            bRes = SendContainer(&m_pRgbCurrent, pSenderDelegate, source);

            // Destroying anyway (or because it was sent 
            // or because it should be Destroyed (ToDestroy()==true)
            if (!bRes || (m_pRgbCurrent && m_pRgbCurrent->ToDestroy()))
            {
              delete m_pRgbCurrent;
              m_pRgbCurrent = NULL;
            }

            pColorFrameNew = (ColorFrame*)res.front();
            res.pop();
            if (res.size() > 1)
            {
              pColorFrameCandidate = (ColorFrame*)res.front();
              res.pop();
            }

            m_pRgbCurrent = new RGBFrame(*m_pSubPrgrmCurrent, m_bIsSnapshot, pColorFrameCandidate);

            m_pRgbCurrent->Add(*pColorFrameNew, __out res);
            pColorFrameCandidate = NULL;
            pColorFrameNew = NULL;
          }

          bRes = SendContainer(&m_pRgbCurrent, pSenderDelegate, source);
        }
      }
      catch (const std::exception& ex)
      {
        TRACE("%s", ex.what());
      }
    }

    return bRes;
  }

  inline bool SetSuperProgram(const CTextFrame& srcData, char* pErr)
  {
    bool res = false;
    // yu-20190424
    // SET COLORS BY TIME DELTAS FROM THIS TextFrame:
    // (FullPrgr=Y:-1000;IR:0;@200000;&R:0;Y:-1000;G:1000;IR:0;@19000;)

    FXString command = srcData.GetString();
    FXSIZE lIndx = command.Find(MSG_KEY);
    FXSIZE rIndx = command.ReverseFind(DELIMITER_BETWEEN_CHANNELS);
    if (lIndx >= 0 && rIndx >= 0 && rIndx > lIndx)
    {
      m_bIsProgramLoaded = false;
      string settings = command.Mid(lIndx, rIndx - lIndx + 1);

      m_superPrgrm.Deserialize(settings);

      char errCause[96] = { 0 };
      res = m_superPrgrm.IsValid(errCause);
      m_bIsProgramLoaded = res;
      if (!res && errCause[0])
      {
        m_superPrgrm.Empty();

        if (pErr)
        {
          sprintf(pErr, "%s%s%s in the '%s' super program"
            , !pErr[0] ? "" : pErr
            , !pErr[0] ? "" : " "
            , errCause
            , settings.c_str());
        }
      }
    }

    res = m_bIsProgramLoaded;

    return res;
  }
};

