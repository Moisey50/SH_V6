#pragma once

#include <gadgets/gadbase.h>
#include <gadgets\containerframe.h>
#include <map>
#include <queue>
#include "RGBContainerFrame/RGBHelper.h"

using namespace std;

#define PK_KEY_IS_SNAPSHOT   ("IsSnapshotMode")
#define PK_KEY_TIME_START_ns ("TimeStart_ns")
#define PK_KEY_TIME_END_ns   ("TimeEnd_ns")
#define PK_KEY_RGB_MASK      ("RGBMask")
#define PK_KEY_RGB_ORDER     ("RGBOrder")
#define PK_KEY_VFRAME_PTR_R  ("VidFrmPtr_R")
#define PK_KEY_VFRAME_PTR_G  ("VidFrmPtr_G")
#define PK_KEY_VFRAME_PTR_B  ("VidFrmPtr_B")

#define FRAME_LBL_VFRAME_R   ("Red")
#define FRAME_LBL_VFRAME_G   ("Green")
#define FRAME_LBL_VFRAME_B   ("Blue")

#define CONTAINER_LBL        ("RGB Frame")

#define FRMT_TMPLT_HEX       ("%x")
#define FRMT_TMPLT_UNSIGN_L  ("%llu")


typedef map<RGB_STATE, const CVideoFrame*> VideoFramesOrder;

class RGB2ContainerFrame2RGBDeSerializer
{
private:
  map<RGB_STATE, FXString>           PK_KEYS_VFRAMES_PTRS = { {RGB_STATE::STATE_R, PK_KEY_VFRAME_PTR_R},  { RGB_STATE::STATE_G, PK_KEY_VFRAME_PTR_G },  {RGB_STATE::STATE_B, PK_KEY_VFRAME_PTR_B} };
  map<RGB_STATE, FXString>           FRAMES_LBLS_VFRAMES = { {RGB_STATE::STATE_R, FRAME_LBL_VFRAME_R},  { RGB_STATE::STATE_G, FRAME_LBL_VFRAME_G },  {RGB_STATE::STATE_B, FRAME_LBL_VFRAME_B} };
  map<RGB_STATE, FXString>*          PK_KEYS_AND_FRMS_LBLS[2] = { &PK_KEYS_VFRAMES_PTRS, &FRAMES_LBLS_VFRAMES };

  bool                               m_bToSend;
  bool                               m_bIsSnapshot;

  UINT64                             m_ui64Start_ns;
  UINT64                             m_ui64End_ns;
  RGB_STATE                          m_eColors;

  FXString                           m_order_RGBStatesVals;

  map<RGB_STATE, const CVideoFrame*> m_order;

  RGB2ContainerFrame2RGBDeSerializer(const RGB2ContainerFrame2RGBDeSerializer&);
  RGB2ContainerFrame2RGBDeSerializer& operator=(const RGB2ContainerFrame2RGBDeSerializer&);

  __forceinline const CVideoFrame* GetVideoFrame(RGB_STATE eState) const
  {
    VideoFramesOrder::const_iterator ci = m_order.find(eState);
    return ci == m_order.end() ? NULL : ci->second;
  }

  __forceinline CContainerFrame * Serialize(__out CContainerFrame** ppContainer, const FXString& lblSufix) const
  {
    CContainerFrame * pRes = *ppContainer;

    if (pRes)
    {
      FXString frmtTmplt;
      frmtTmplt.Format("0x%s", FRMT_TMPLT_HEX);
      if (!Serialize(m_order, pRes, lblSufix, frmtTmplt))
      {
        (*ppContainer)->Release();
        *ppContainer = NULL;
        pRes = NULL;
      }
      else
      {
        Serialize(m_ui64Start_ns, PK_KEY_TIME_START_ns, pRes->Attributes(), FRMT_TMPLT_UNSIGN_L);
        Serialize(m_ui64End_ns, PK_KEY_TIME_END_ns, pRes->Attributes(), FRMT_TMPLT_UNSIGN_L);

        pRes->Attributes()->WriteInt(PK_KEY_RGB_MASK, m_eColors);
        pRes->Attributes()->WriteBool(PK_KEY_IS_SNAPSHOT, m_bIsSnapshot);
        pRes->Attributes()->WriteString(PK_KEY_RGB_ORDER, m_order_RGBStatesVals);
      }
    }

    return pRes;
  }

  __forceinline bool Serialize(const VideoFramesOrder& order, CContainerFrame* pContainer, const FXString& lblSufix, const FXString& frmtTmplt) const
  {
    bool bRes = false;
    if (pContainer)
    {
      map<RGB_STATE, FXString>& keys = *PK_KEYS_AND_FRMS_LBLS[0];
      map<RGB_STATE, FXString>& labels = *PK_KEYS_AND_FRMS_LBLS[1];

      VideoFramesOrder::const_iterator ci = order.begin();
      if (ci != order.end())
        bRes = true;

      for (; ci != order.end(); ++ci)
      {
        RGB_STATE eState = ci->first;
        const CVideoFrame* pFrame = ci->second;

        bRes &= Serialize(pFrame, keys[eState], pContainer->Attributes(), frmtTmplt);

        SetLabel(labels[eState], lblSufix, (CVideoFrame*)pFrame);

        //YS_20190502 !VERY IMPORTANT NOTE!
        //Cast the pFrame to the NON const pointer
        //to eliminate frame REFERENCE adding in
        //the AddFrame(const CDataFrame*) function to the local pFrame,
        //that was created localy or copied from the incoming Frame
        pContainer->AddFrame((CDataFrame*)pFrame);
      }
    }

    return bRes;
  }

  __forceinline bool Serialize(const CVideoFrame* pSrcVF, const char* key, FXPropertyKit* pDstPK, const char* pFrmtTmplt) const
  {
    bool res = false;

    if (pSrcVF && pFrmtTmplt)
    {
      FXString  tempFormater;
      tempFormater.Format(pFrmtTmplt, pSrcVF);
      res = pDstPK->WriteString(key, tempFormater);
      tempFormater.Empty();
    }
    return res;
  }

  __forceinline bool Serialize(ULONG64 ulSrc, const char* key, FXPropertyKit* pDstPK, const char* pFrmtTmplt) const
  {
    bool res = false;

    if (ulSrc && pFrmtTmplt)
    {
      FXString  tempFormater;
      tempFormater.Format(pFrmtTmplt, ulSrc);
      res = pDstPK->WriteString(key, tempFormater);
      tempFormater.Empty();
    }
    return res;
  }

  template<typename  T>
  __forceinline bool Deserialize(const FXPropertyKit* pSrcPK, const char* key, __out T& dst)
  {
    bool res = false;

    if (pSrcPK && key)
    {
      FXString tmpltSerialized;

      if (pSrcPK->GetString(key, tmpltSerialized))
      {
        UINT tTypeSize = sizeof(T);

        if (tTypeSize == sizeof(UINT64))
        {
          UINT64 ui64Tmp = strtoull(tmpltSerialized, NULL, 0);
          if (ui64Tmp > 0)
          {
            dst = (T)ui64Tmp;
            res = true;
          }
        }
        else if (tTypeSize == sizeof(CVideoFrame*))
        {
          CVideoFrame* pRes = ( CVideoFrame* )
#ifdef _M_X64
            strtoull( tmpltSerialized , NULL , 0 ) ;
#else
            strtol(tmpltSerialized, NULL, 0);
#endif
          if (pRes)
          {
            dst = (T)pRes;
            res = true;
          }
        }

      }
    }

    return res;
  }

  __forceinline bool Deserialize(const CContainerFrame * pContainer, const FXString & order_RGBStatesVals, RGB_STATE eColorsExpected)
  {
    bool bRes = false;
    if (pContainer)
    {
      map<RGB_STATE, FXString>& keys = *PK_KEYS_AND_FRMS_LBLS[0];
      map<RGB_STATE, FXString>& labels = *PK_KEYS_AND_FRMS_LBLS[1];

      m_order.clear();

      int iColorsExpected = eColorsExpected;
      int c = 0;
      for (; c < order_RGBStatesVals.GetLength() && eColorsExpected > RGB_STATE::STATE_UNKNOWN; ++c)
      {
        RGB_STATE eColor = (RGB_STATE)order_RGBStatesVals[c];

        CVideoFrame* pVF_expected = NULL;
        const CVideoFrame* pVF_actual = pContainer->GetVideoFrame(labels[eColor]);

        if (pVF_actual
          && ((eColorsExpected & eColor) == eColor)
          && Deserialize<CVideoFrame*>(pContainer->Attributes(), keys[eColor], pVF_expected)
          /*&& pVF_actual == pVF_expected*/)
        {
          m_order[eColor] = pVF_actual;
          iColorsExpected ^= (int)eColor;
        }
        bRes = true;
      }

      eColorsExpected = (RGB_STATE)iColorsExpected;

      if (eColorsExpected > RGB_STATE::STATE_UNKNOWN
        || c < order_RGBStatesVals.GetLength()
        || c != m_order.size())
      {
        bRes = false;
        m_order.clear();
      }
    }

    return bRes;
  }

  __forceinline const RGB2ContainerFrame2RGBDeSerializer& SetLabel(LPCTSTR pLabelPefix, LPCTSTR pLabelSuffix, CDataFrame * pFrame) const
  {
    if (pFrame)
    {
      FXString fullLbl;
      GetFullLabel(pLabelPefix, pLabelSuffix, __out fullLbl);

      pFrame->SetLabel(fullLbl);
    }
    return *this;
  }

  __forceinline const RGB2ContainerFrame2RGBDeSerializer& GetFullLabel(LPCTSTR pLabelPrefix, LPCTSTR pLabelSuffix, __out FXString &fullLbl) const
  {
    fullLbl.Format("%s%s%s"
      , pLabelPrefix
      , !pLabelSuffix || !pLabelSuffix[0] ? "" : "_"
      , !pLabelSuffix || !pLabelSuffix[0] ? "" : pLabelSuffix
    );
    return *this;
  }

public:
  __forceinline RGB2ContainerFrame2RGBDeSerializer()
    : m_order()
    , m_bToSend(false)
    , m_bIsSnapshot(false)
    , m_ui64Start_ns(0)
    , m_ui64End_ns(0)
    , m_eColors(RGB_STATE::STATE_UNKNOWN)
    , m_order_RGBStatesVals()
  {
  }
  __forceinline ~RGB2ContainerFrame2RGBDeSerializer()
  {
    Empty();
  }

  __forceinline bool IsEmpty() const
  {
    return m_order.empty();
  }
  __forceinline bool IsSnapshot() const
  {
    return m_bIsSnapshot;
  }
  __forceinline RGB_STATE GetRGBState() const
  {
    return m_eColors;
  }
  __forceinline UINT64 GetStart_ns() const
  {
    return m_ui64Start_ns;
  }
  __forceinline UINT64 GetEnd_ns() const
  {
    return m_ui64End_ns;
  }
  __forceinline const FXString& GetOrderAsTxt() const
  {
    return m_order_RGBStatesVals;
  }
  __forceinline const VideoFramesOrder& GetOrder() const
  {
    return m_order;
  }
  __forceinline const CVideoFrame* GetVideoFrame_R() const
  {
    return GetVideoFrame(RGB_STATE::STATE_R);// (CVideoFrame*)m_pR;
  }
  __forceinline const CVideoFrame* GetVideoFrame_G() const
  {
    return GetVideoFrame(RGB_STATE::STATE_G);//(CVideoFrame*)m_pG;
  }
  __forceinline const CVideoFrame* GetVideoFrame_B() const
  {
    return GetVideoFrame(RGB_STATE::STATE_B);//(CVideoFrame*)m_pB;
  }

  __forceinline const RGB2ContainerFrame2RGBDeSerializer& Empty()
  {
    m_order.clear();
    m_bToSend = false;

    m_bToSend = (false);
    m_bIsSnapshot = (false);
    m_ui64Start_ns = (0);
    m_ui64End_ns = (0);
    m_eColors = (RGB_STATE::STATE_UNKNOWN);
    m_order_RGBStatesVals.Empty();
    return *this;
  }

  __forceinline CDataFrame* GetCContainerFrame(
    bool bIsSnapshot
    , UINT64 ui64Start_ns
    , UINT64 ui64End_ns
    , RGB_STATE eColors
    , const FXString& orderAsTxt
    , const VideoFramesOrder& order
    , DWORD dwId = 0
    , LPCTSTR pLabel = NULL
  )
  {
    m_bIsSnapshot = bIsSnapshot;
    m_ui64Start_ns = ui64Start_ns;
    m_ui64End_ns = ui64End_ns;
    m_eColors = eColors;

    m_bToSend = true;

    m_order_RGBStatesVals.Empty();
    m_order_RGBStatesVals = orderAsTxt;

    m_order.clear();
    VideoFramesOrder::const_iterator ci = order.begin();
    for (; ci != order.end(); ++ci)
    {
      RGB_STATE eColor = ci->first;
      const CVideoFrame* pVFrame = ci->second;

      if ((m_eColors & eColor) == eColor)
      {
        m_order[eColor] = pVFrame;
      }
    }

    CContainerFrame* pRes = CContainerFrame::Create();

    CContainerFrame* pContainer = Serialize(__out &pRes, pLabel);

    if (pContainer && pRes)
    {
      pContainer->ChangeId(dwId);
      pContainer->SetTime(GetHRTickCount());

      SetLabel(CONTAINER_LBL, pLabel, pContainer);

      if (pContainer->GetFramesCount() == 0)
        pRes = NULL;
    }

    return pRes;
  }


  __forceinline bool Deserialize(const CContainerFrame* pSerialized)
  {
    bool res = false;

    if (pSerialized)
    {
      FXString order_RGBStatesVals;
      res = pSerialized->Attributes()->GetString(PK_KEY_RGB_ORDER, order_RGBStatesVals);
      if (res)
        m_order_RGBStatesVals = order_RGBStatesVals;

      int clrsMask = 0;
      res = pSerialized->Attributes()->GetInt(PK_KEY_RGB_MASK, clrsMask);
      if (res)
        m_eColors = (RGB_STATE)clrsMask;

      if (!Deserialize(pSerialized, m_order_RGBStatesVals, m_eColors))
      {
        m_order_RGBStatesVals.Empty();
        m_eColors = RGB_STATE::STATE_UNKNOWN;
      }
      else
      {
        UINT64 ui64Tmp = 0;
        if (Deserialize(pSerialized->Attributes(), PK_KEY_TIME_START_ns, __out ui64Tmp))
          m_ui64Start_ns = ui64Tmp;
        ui64Tmp = 0;

        if (Deserialize(pSerialized->Attributes(), PK_KEY_TIME_END_ns, __out ui64Tmp))
          m_ui64End_ns = ui64Tmp;
        ui64Tmp = 0;

        bool bIsSnapshot = false;
        if (pSerialized->Attributes()->GetBool(PK_KEY_IS_SNAPSHOT, bIsSnapshot))
          m_bIsSnapshot = bIsSnapshot;

        res = true;
      }
    }

    return res;
  }

  __forceinline static RGB2ContainerFrame2RGBDeSerializer* Create(const CContainerFrame* pContainer)
  {
    RGB2ContainerFrame2RGBDeSerializer* pRes = NULL;

    const int nMax_COLORS = 3;

    if (pContainer)
    {
      //const char* lblsPrefix[] = { FRAME_LBL_VFRAME_R, FRAME_LBL_VFRAME_G, FRAME_LBL_VFRAME_B };
      //const CVideoFrame* pVFs[nMax_COLORS] = { 0 };
      //int j = 0;
      //for (int i = 0; i < nMax_COLORS; i++)
      //{
      //  const CVideoFrame* pVF = pContainer->GetVideoFrame(lblsPrefix[i]);
      //  if (pVF)
      //    pVFs[j++] = pVF;
      //}

      pRes = new RGB2ContainerFrame2RGBDeSerializer();
      bool bRes = pRes->Deserialize(pContainer);//, pVFs, j);
      if (/*j == 0 ||*/ !bRes)
      {
        delete pRes;
        pRes = NULL;
      }
    }
    return pRes;
  }
};