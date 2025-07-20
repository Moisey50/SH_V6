#pragma once
#include "ColorChannel.h"


struct ColorChannelNode
{
private:
  const ColorChannel m_data;
  ColorChannelNode* m_pPrev = NULL;
  ColorChannelNode* m_pNext = NULL;

  ColorChannelNode(const ColorChannelNode&);
  ColorChannelNode& operator= (const ColorChannelNode&);

public:


  ColorChannelNode(const ColorChannel & chnl)
    : m_data(chnl)
  {}
  ~ColorChannelNode()
  {
    m_pPrev = NULL;
    m_pNext = NULL;
  }

  ColorChannelNode(const string& name, int deltaTime_us)
    : ColorChannelNode(ColorChannel(name, deltaTime_us))
  {}

  ColorChannelNode* GetPrev() const
  {
    return m_pPrev;
  }
  ColorChannelNode& SetPrev(ColorChannelNode* pPrev)
  {
    m_pPrev = pPrev;
    return *this;
  }

  ColorChannelNode* GetNext() const
  {
    return m_pNext;
  }
  ColorChannelNode& SetNext(ColorChannelNode* pNext)
  {
    m_pNext = pNext;
    return *this;
  }

  const ColorChannel& GetData() const
  {
    return m_data;
  }
};


