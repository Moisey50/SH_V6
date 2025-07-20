#pragma once

#include <map>
#include "ColorChannelNodesList.h"
#include "ColorFrame.h"
#include "RGBContainerFrame/RGB2ContainerFrame2RGBDeSerializer.h"

using namespace std;

typedef map<RGB_MODE, const ColorFrame*> ColorsOrder;
struct RGBFrame
{
private:
  const ColorChannelNodesList&     m_subProgram;
  bool                             m_bIsSnapShot;
  bool                             m_bToDestroy;
  UINT32                           m_ui32PeriodNominal_us;
  UINT64                           m_ui64Start_ns;
  UINT64                           m_ui64End_ns;
  RGB_MODE                         m_eColors;
  const ColorFrame*                m_pPrev;
  const ColorFrame*                m_pCandidate;
  FXString                         m_orderAsTxt;
  ColorsOrder                      m_order;

  RGBFrame(const RGBFrame&);
  RGBFrame& operator= (const RGBFrame&);

  const ColorChannelNode* GetExpectedColorChannelNode() const
  {
    const ColorChannelNode* pRes = NULL;

    if (m_pPrev
      && m_pCandidate
      && m_pPrev != m_pCandidate
      && !IsFull())
    {
      pRes = m_pPrev->GetColorChannelNode()->GetNext();

      //if the RGB frame was started NOT from the first Color Channel,
      //then Expected ColorChannelNode is a First one in the SubProgram;
      if (!pRes)
        pRes = m_subProgram.Start();
    }

    return pRes;
  }

  void Add(RGB_MODE eColor, const ColorFrame & newFrame)
  {
    if (eColor > RGB_MODE::MODE_UNKNOWN && eColor < RGB_MODE::MODE_RGB)
    {
      m_order[eColor] = m_pCandidate;
      m_orderAsTxt.AppendChar((char)eColor);
      if (!m_pPrev)
        m_ui64Start_ns = m_pCandidate->GetTimestamp_ns();
      m_pPrev = m_pCandidate;
      m_pCandidate = IsFull() ? NULL : &newFrame;
      if (!m_pCandidate)
        m_ui64End_ns = newFrame.GetTimestamp_ns();
      m_eColors = (RGB_MODE)(m_eColors | eColor);
    }
  }

  RGBFrame& DestroyColorFrame(ColorFrame** ppFrameToDestroy)
  {
    if (ppFrameToDestroy && *ppFrameToDestroy)
    {
      delete (*ppFrameToDestroy);
      (*ppFrameToDestroy) = NULL;
    }
    return *this;
  }

  bool GetVFramesOrder(__out map<RGB_STATE, const CVideoFrame*>& outVFrmsOrder)const
  {
    bool bRes = false;
    ColorsOrder::const_iterator ci = m_order.begin();

    for (; ci != m_order.end(); ++ci)
    {
      if (ci->second->GetVideoFrame())
        outVFrmsOrder[(RGB_STATE)ci->first] = (const CVideoFrame*)ci->second->GetVideoFrame()->Copy();
    }

    bRes = (outVFrmsOrder.size() == m_order.size());

    return bRes;
  }

  RGBFrame& ReleaseAndDestroyColorFrame(ColorFrame ** ppFrame)
  {
    if (ppFrame && *ppFrame)
    {
      (*ppFrame)->ReleaseVideoFrame();

      if ((*ppFrame)->IsVideoFrameEmpty())
        DestroyColorFrame(ppFrame);
    }
    return *this;
  }

  RGBFrame& ReleaseAndDestroyCandidate()
  {
    return ReleaseAndDestroyColorFrame((ColorFrame**)&m_pCandidate);
  }


public:
  RGBFrame(const ColorChannelNodesList& subProgram, bool bIsSnapShot, const ColorFrame* pCandidateFrame = NULL)
    : m_subProgram(subProgram)
    , m_bIsSnapShot(bIsSnapShot)
    , m_bToDestroy(false)
    , m_ui32PeriodNominal_us(subProgram.GetPeriodNominal_us())
    , m_ui64Start_ns(0)
    , m_ui64End_ns(0)
    , m_eColors(RGB_MODE::MODE_UNKNOWN)
    , m_pPrev(NULL)
    , m_pCandidate(pCandidateFrame)
    , m_orderAsTxt()
    , m_order()
  {
  }

  ~RGBFrame()
  {
    Empty();
  }

  bool Empty()
  {
    ColorsOrder::iterator i = m_order.begin();

    for (; i!= m_order.end(); ++i)
    {
      if (m_pPrev == i->second)
        m_pPrev = NULL;

      if (m_pCandidate == i->second)
        m_pCandidate = NULL;

      ReleaseAndDestroyColorFrame((ColorFrame**)&i->second);
    }

    m_order.clear();

    if (m_pPrev)
      ReleaseAndDestroyColorFrame((ColorFrame**)&m_pPrev);
    if (m_pCandidate)
      ReleaseAndDestroyCandidate();

    return m_order.size() == 0;
  }

  bool IsFull() const
  {
    return GetCount() > 0 && m_subProgram.GetCount() == GetCount();
  }

  bool ToDestroy() const
  {
    return m_bToDestroy;
  }

  int GetCount() const
  {
    return (int)m_order.size();
  }

  // Returns an empty queue, if the candidate is sucessfuly added;
  // Check the 'IsFull()' flag if the answer queue is empty and move this
  // RGB frame to the queue of the RGB frames;
  // The frames in the answer queue should be used to create a NEW RGB frame,
  // when the first frame is ALLWAYS the wrapped new video frame, amd
  // a second one (when it present) is a unused (but valid) candidate for 
  // reuse. To reuse the received candidate use second (optional) paremeter
  // of the constructor.
  // If answer queue is NOT empty - the ToDEstroy() flag chould be checked
  // and current RGB frame should be destroyed (NOT reused).
  queue<const ColorFrame*>& Add(const CVideoFrame& newFrame, __out queue <const ColorFrame*>& outRes)
  {
    bool res = false;
    const ColorFrame* pNew = new ColorFrame(&newFrame);

    return Add(*pNew, __out outRes);
  }
  queue<const ColorFrame*>& Add(const ColorFrame& newFrame, __out queue <const ColorFrame*>& outRes)
  {
    if (!m_pCandidate)
    {
      if (!m_pPrev) // The new RGB frame is started here!
        m_pCandidate = &newFrame;
      else
        outRes.push(&newFrame);
    }
    else
#pragma region | Explanation |
      // The target is to determine if the m_pCandidate frame belongs to this RGB frame and if this RGB frame is legal or should be DESTROYED;
      // If [m_pCandidate != NULL] -- so this RGB frame expects for the frame, and the candidate (m_pCandidate frame) is an EXPECTED frame,
      // because the RGB frame is:
      // 1. EMPTY yet (Coun=0 and m_pPrev==NULL) and a frame length of the m_pCandidate frame is:
      //   1.1. Legal -- so ADD candidate to the right channel and switch the m_pCandidate to the m_pPrev,
      //        clear the m_pCandidate (=NULL), and then if the RGB actual count (after candidate adding):
      //        1.1.1. Still less than requered quantity (by SubProgram) -- so set the new frame to the m_pCandidate frame
      //               and RETURN the empty result queue;
      //        1.1.2. Equals to reqiered quantity (by SubProgram) -- so just add the new frame to the result queue;
      //   1.2. Illegal -- so just REPLACE the m_pCandidate frame with m_pNew frame ---- RETURN NULL;
      // 2. NOT empty and the frame lengthes of the m_pCandidate frame and of the NEXT color channel in the m_pPrev frame are:
      //   2.1. Equal -- so ADD candidate to the right channel and switch the m_pCandidate to the m_pPrev,
      //        clear the m_pCandidate (=NULL), and then if the RGB actual count (after candidate adding):
      //        2.1.1. Still less than requered quantity (by SubProgram) -- so set the new frame to the m_pCandidate frame
      //               and RETURN the empty result queue;
      //        2.1.2. Equals to reqiered quantity (by SubProgram) -- so just add the new frame to the result queue;
      //   2.2. NOT Equal -- so return m_pNew frame and DESTROY this RGB frame (m_bToDestroy = true;),
      //        and if the frame length of the m_pCandidate frame is:
      //        2.2.1. Illegal (is NOT equals for ANY Color Cannel in the sub-program) -- so clear the m_pCandidate frame;
      //        2.2.2. Legal but different from the expected one -- so DO NOT clear the m_pCandidate frame and return it together with mew frame in the result
      //               queue, when the first is the new frame and the second is the candidate frame;  
#pragma endregion | Explanation |
    {
      double dPeriodActual = ((ColorFrame&)*m_pCandidate).SetFrameLengthActual_us(newFrame).GetFrameLengthActual_us();
     
      TRACE("Actual period (us) between the Candidate (%s) and a New Frame (%s) is %f!!!!\n"
        , m_pCandidate->GetVideoFrame()->GetLabel()
        , newFrame.GetVideoFrame()->GetLabel()
        , dPeriodActual);
      
      const ColorChannelNode* pChannelNode = NULL;
      const ColorChannelNode* pChannelExpected = NULL;

      if (IsFull())
      {
        // UNEXPECTED behavior: In the FULL RGB frame the m_pCandidate SOULD be EMPTY;

        ColorsOrder::iterator i = m_order.begin();

        for (; i != m_order.end(); ++i)
        {
          if(m_pCandidate == i->second )
            m_pCandidate = NULL;
        }

        if (m_pCandidate)
          ReleaseAndDestroyCandidate();
        
        outRes.push(&newFrame);
      }
      else if (!m_pPrev && GetCount() == 0) // This is a New (empty) RGB frame;
      {
        pChannelNode = m_subProgram.GetChannel(dPeriodActual);
      }
      else
      {
        pChannelExpected = GetExpectedColorChannelNode();
        if (!pChannelExpected)
        {
          // UNEXPECTED behavior: Nothing TO Do!?
        }
        else if (pChannelExpected->GetData().IsColorChannel(dPeriodActual))
          // The m_pCandidate is the Expected Frame!!! Very GOOD!!!;
        {
          pChannelNode = pChannelExpected;
        }
      }

      if (pChannelNode)
      {
        RGB_MODE color = ((ColorFrame*)m_pCandidate)->SetColorChannelNode(pChannelNode).GetColor();
        if (!(m_eColors & color)) //
        {
          // (see p. 1.1. or p. 2.1. in the explanation below)
          Add(color, newFrame);

          // (see p. 1.1.2.  or p. 2.1.2.  in the explanation below)
          if (!m_pCandidate)
            outRes.push(&newFrame);
        }
        else
        {
          m_bToDestroy = true;
        }
      }
      else
        // The frame length is too long or too short (Color Channel did NOT found) or the Candidate frame is NOT expected one?!;
      {
        if (!m_pPrev && GetCount() == 0)
        {
          ReleaseAndDestroyCandidate();
          m_pCandidate = &newFrame;  // (see p. 1.2. in the explanation below)
        }
        else if (outRes.empty()) // (see p. 2.2. in the explanation below)
          // Determine how to destroy this (RGB) frame because:
          // 1. pChannelExpected is !?UNexpectadly!? missing!?;
          // 2. m_pCandidate frame is NOT pChannelExpected;
        {
          m_bToDestroy = true;
          pChannelNode = m_subProgram.GetChannel(dPeriodActual);
        }
      }

      if (m_bToDestroy)
      {
        outRes.push(&newFrame);
        if (pChannelNode) // Candidate should start a new RGB frame;
        {
          outRes.push(m_pCandidate);
          // The pair of the newFrame with the Candidate frame will be returned and
          // used to create a new RGB frame;
        }

        ReleaseAndDestroyCandidate();
        // ELSE: The frame length is too long or too short -- Color Channel did NOT found!
        // Just newFrame will be returned;
      }
    }
    return outRes;
  }

  CDataFrame * GetCContainerFrame(DWORD dwId = 0, LPCTSTR pLabel = NULL) const
  {
    CDataFrame * pRes = NULL;

    map<RGB_STATE, const CVideoFrame*> order;
    if (GetVFramesOrder(order))
    {
      RGB2ContainerFrame2RGBDeSerializer rgb2cfSerializer;

      pRes = rgb2cfSerializer.GetCContainerFrame(
        m_bIsSnapShot
        , m_ui64Start_ns
        , m_ui64End_ns
        , (RGB_STATE)m_eColors
        , m_orderAsTxt
        , order
        , dwId, pLabel);

      order.clear();
    }
    return pRes;
  }
};