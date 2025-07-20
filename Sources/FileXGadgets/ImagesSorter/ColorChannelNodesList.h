#pragma once
#include "ColorChannelNode.h"
#include <string>

using namespace std;

#define MAX_CHANNELS_QTY_RGB       (3)
#define DELIMITER_BETWEEN_CHANNELS (';')
#define DELIMITER_PERIOD_NOMINAL   ('@')

struct ColorChannelNodesList
{


private:
  const string      m_type;
  ColorChannelNode* m_pHead = NULL;
  ColorChannelNode* m_pTail = NULL;
  ColorChannelNode* m_pIterator = NULL;
  int               m_count = 0;
  UINT32            m_ui32PeriodNominal_us = 0;

  ColorChannelNodesList(const ColorChannelNodesList&);
  ColorChannelNodesList& operator= (const ColorChannelNodesList&);

public:
  ColorChannelNodesList(const string& type)
    : m_type(type)
  {
    //RESET_INSTANCE;
  }

  ~ColorChannelNodesList()
  {
    Empty();
    m_pHead = NULL;
    m_pTail = NULL;
    m_pIterator = NULL;
  }

  const string& GetType() const
  {
    return m_type;
  }

  const ColorChannelNode* Start() const
  {
    return m_pHead;
  }
  const ColorChannelNode* End() const
  {
    return m_pTail;
  }

  int GetCount() const
  {
    return m_count;
  }

  bool IsValid(char* pErr) const
  {
    bool isEmpty = m_count == 0;
    bool isOverflow = m_count > MAX_CHANNELS_QTY_RGB;
    bool isValid = !isEmpty && !isOverflow;

    if (!isValid && pErr)
    {
      sprintf(pErr, isEmpty ? "is empty" : "has too much colors");
    }

    return isValid;
  }

  uint32_t GetPeriodNominal_us() const
  {
    return m_ui32PeriodNominal_us;
  }

  const ColorChannelNode* GetChannel(double dPeriodActual) const
  {
    const ColorChannelNode* pRes = NULL;

    const ColorChannelNode* pIterator = m_pHead;
    while (pIterator)
    {
      const ColorChannel& cc = pIterator->GetData();
      if (!cc.IsColorChannel(dPeriodActual))
        pIterator = pIterator->GetNext();
      else
      {
        pRes = pIterator;
        pIterator = NULL;
      }
    }

    return pRes;
  }

  ColorChannelNodesList& Empty()
  {
    m_pIterator = NULL;
    if (m_pTail)
    {
      delete m_pTail->GetNext();
      m_pTail->SetNext(NULL);

      while (m_pTail)
      {
        ColorChannelNode * pLast = m_pTail;
        m_pTail = m_pTail->GetPrev();
        if (m_pTail)
          m_pTail->SetNext(NULL);
        pLast->SetPrev(NULL);

        delete pLast;
        --m_count;
      }

      m_pHead = NULL;
      m_count = 0;
      m_ui32PeriodNominal_us = 0;
    }

    return *this;
  }

  ColorChannelNodesList& Deserialize(const string& src)
  {
    int nOffset = 0;
    int dlmtrIndx = (int)src.find_first_of(DELIMITER_BETWEEN_CHANNELS, nOffset);
    string pToken;
    ColorChannelNode *pNode = NULL;
    Empty();

    while (dlmtrIndx >= 0)
    {
      pNode = NULL;
      int cnt = dlmtrIndx - nOffset;
      pToken = src.substr(nOffset, cnt);
      ColorChannel *pCC = ColorChannel::Deserialize(pToken);

      if (pCC)
        pNode = new ColorChannelNode(*pCC);

      if (!Add(pNode) && pToken[0] == DELIMITER_PERIOD_NOMINAL)
      {
        m_ui32PeriodNominal_us = atol(pToken.c_str() + 1);
      }
      nOffset += (cnt + 1);
      dlmtrIndx = (int)src.find_first_of(DELIMITER_BETWEEN_CHANNELS, nOffset);
    }
    pNode = m_pHead;
    while (pNode)
    {
      ColorChannel &cc = (ColorChannel &)pNode->GetData();
      cc.SetPeriodExpected_us(m_ui32PeriodNominal_us);
      pNode = pNode->GetNext();
    }

    return *this;
  }

  bool Add(ColorChannelNode * pNode)
  {
    bool res = false;
    if (pNode)
    {
      if (!m_pHead)
      {
        m_pHead = pNode;
        m_count = 0;
      }
      else
      {
        pNode->SetPrev(m_pTail);
        m_pTail->SetNext(pNode);
      }
      m_pTail = pNode;
      ++m_count;
      res = true;
    }

    return res;
  }
};

